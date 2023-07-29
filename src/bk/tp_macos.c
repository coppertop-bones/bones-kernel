// ---------------------------------------------------------------------------------------------------------------------
// Copyright 2025 David Briant, https://github.com/coppertop-bones. Licensed under the Apache License, Version 2.0
//
// TP - TEXT PAD - MACOS SPECIFIC
// ---------------------------------------------------------------------------------------------------------------------

#ifndef SRC_BK_TP_MACOS_C
#define SRC_BK_TP_MACOS_C "bk/tp_macos.c"

#include "tp.c"

typedef struct {
    BK_TP *tp;
    size cursor;
} TP_Cookie;


pvt inline size tp_new_size(size n) {
    /* This effectively is a return ceil(n * φ).
       φ is approximatively 207 / (2^7), so we shift our result by
       6, then perform our ceil by adding the remainder of the last division
       by 2 of the result to itself. */
//    n = (n * 207) >> 6;
//    n = (n >> 1) + (n & 1);
    n = n + TP_BUF_INC;
    return n;
}

pvt int tp_grow_buf_if_needed(TP_Cookie *cookie, size required) {
    if (cookie->cursor > SIZE_MAX - required) {
        errno = EOVERFLOW;
        return -1;
    }
    required += cookie->cursor;

    size newsize = cookie->tp->buf_sz;
    if (required <= newsize) return 0;

    while (required > newsize) newsize = tp_new_size(newsize);

    // OPEN: reallocInBuckets does not copy - handle this
    char *p = reallocInBuckets(cookie->tp->buckets, cookie->tp->buf, newsize, 1);
    if (!p) return -1;

    cookie->tp->buf = p;
    cookie->tp->buf_sz = newsize;
    return 0;
}

pvt int tp_apply_cursor(S8 *buf, TP_Cookie *c) {
    if (c->tp->buf_sz < c->cursor) return -1;
    buf->cs = c->tp->buf + c->cursor;
    buf->sz = c->tp->buf_sz - c->cursor;
    return 0;
}

pvt size tp_copy(S8 *from, S8 *to) {
    size copied = from->sz < to->sz ? from->sz : to->sz;
    memcpy(to->cs, from->cs, copied);
    return copied;
}


// ---------------------------------------------------------------------------------------------------------------------
// std style stream fns
// ---------------------------------------------------------------------------------------------------------------------

pvt int tp_write(void *p, char const *buf, int sz) {
    if (sz < 0) {
        errno = EINVAL;
        return -1;
    }
    TP_Cookie *c = p;

    S8 from = { sz, (char *) buf };
    S8 to;

    if (tp_grow_buf_if_needed(c, sz) < 0) return -1;
    if (tp_apply_cursor(&to, c) < 0) return 0;

    size copied = tp_copy(&from, &to);
    c->cursor += copied;
    if (c->tp->end < c->cursor) c->tp->end = c->cursor;
    if (copied > INT_MAX) {
        errno = EOVERFLOW;
        return -1;
    }
    return copied;
}

pvt int tp_read(void *p, char *buf, int sz) {
    if (sz < 0) {
        errno = EINVAL;
        return -1;
    }
    TP_Cookie *c = p;
    S8 from;
    S8 to = { sz, buf };

    if (tp_apply_cursor(&from, c) < 0) return 0;

    size copied = tp_copy(&from, &to);
    c->cursor += copied;
    if (copied > INT_MAX) {
        errno = EOVERFLOW;
        return -1;
    }
    return copied;
}

pvt off_t tp_seek(void *p, off_t off, int whence) {
    TP_Cookie *c = p;
    size newoff;
    switch (whence) {
        case SEEK_SET: newoff = off; break;
        case SEEK_CUR: newoff = c->cursor + off; break;
        case SEEK_END: newoff = c->tp->end + off; break;
        default: errno = EINVAL; return -1;
    }
    if (newoff > c->tp->end || (off_t)newoff < 0 || newoff > (size)OFF_MAX) {
        errno = EOVERFLOW;
        return -1;
    }
    c->cursor = newoff;
    return newoff;
}

pvt int tp_close(__attribute__((unused)) void *p) {
//    free(p);
    return 0;
}


tdd FILE *tp_open(BK_TP *tp, char const *mode) {
    b32 bufAllocated = 0;  size pos;

    if (strcmp(mode, "r") == 0) {
        // Open text file for reading.  The stream is positioned at the
        // beginning of the file.
        pos = 0;
    } else if (strcmp(mode, "r+") == 0) {
        // Open for reading and writing.  The stream is positioned at the
        // beginning of the file.
        pos = 0;
    } else if (strcmp(mode, "w") == 0) {
        // Truncate file to zero length or create text file for writing.
        // The stream is positioned at the beginning of the file.
        pos = 0;
    } else if (strcmp(mode, "w+") == 0) {
        // Open for reading and writing.  The file is created if it does not
        // exist, otherwise it is truncated.  The stream is positioned at
        // the beginning of the file.
        pos = 0;
    } else if (strcmp(mode, "a") == 0) {
        // Open for writing.  The file is created if it does not exist.  The
        // stream is positioned at the end of the file.  Subsequent writes
        // to the file will always end up at the then current end of file,
        // irrespective of any intervening fseek(3) or similar.
        pos = tp->end;
    } else if (strcmp(mode, "a+") == 0) {
        // Open for reading and writing.  The file is created if it does not
        // exist.  The stream is positioned at the end of the file.  Subse-
        // quent writes to the file will always end up at the then current
        // end of file, irrespective of any intervening fseek(3) or similar.
        pos = tp->end;
    } else return 0;

    if (!tp->buf) {
        tp->buf_sz = 128;
        tp->buf = allocInBuckets(tp->buckets, tp->buf_sz, 1);
        bufAllocated = 1;
    }
    if (!tp->buf) return 0;

    TP_Cookie *c = (TP_Cookie *) allocInBuckets(tp->buckets, sizeof(TP_Cookie), bk_alignof(TP_Cookie));
    if (!c) {
        if (bufAllocated) {
//            tp->mm->free(tp->buf);
            tp->buf = 0;
            tp->buf_sz = 0;
        }
        return 0;
    }
    c->tp = tp;
    c->cursor = pos;

    // https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man3/funopen.3.html
    FILE *f = funopen(c, tp_read, tp_write, tp_seek, tp_close);
    if (!f) {
        if (bufAllocated) {
//            tp->mm->free(tp->buf);
            tp->buf = 0;
            tp->buf_sz = 0;
        }
//        tp->mm->free(c);
    }
    return f;
}

#endif      // SRC_BK_TP_C
