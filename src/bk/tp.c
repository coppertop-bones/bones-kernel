// ---------------------------------------------------------------------------------------------------------------------
// TP - TEXT PAD
// ---------------------------------------------------------------------------------------------------------------------

#ifndef SRC_BK_TP_C
#define SRC_BK_TP_C "bk/tp.c"

#include <errno.h>
#include <limits.h>
#include <libc.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "../../include/bk/bk.h"
#include "../../include/bk/tp.h"

#define TP_BUF_INC 1024

#define tp_as_s8(n) (n | tpvt_s8)
#define tp_as_seq(n) (n | tpvt_seq)
#define tp_t(x) (enum tpvt)(x.vtsz & 0xFFFFFFFF00000000)
#define tp_sz(x) (size)((x.vtsz & 0x00000000FFFFFFFF))
#define tp_at(x, i) (((TPN*) x.p)[i])
#define tp_nseq(tp) (*((int *)tp.p))


enum tpvt {
    tpvt_s8 = 0x0000000000000000,           // S8
    tpvt_seq = 0x4000000000000000,          // 4 -> 0100 - sequence of nodes
    tpvt_slice = 0x8000000000000000,        // 8 -> 1000 - slice
    tpvt_reserved = 0xC000000000000000,     // C -> 1100
};

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


// ---------------------------------------------------------------------------------------------------------------------
// TP lifecycle
// ---------------------------------------------------------------------------------------------------------------------

pub void TP_init(BK_TP *tp, size initSz, Buckets *buckets) {
    tp->start = 0;
    tp->end = 0;
    tp->buckets = buckets;
    int buf_sz = 0;
    while (buf_sz < initSz) buf_sz += TP_BUF_INC;
    tp->buf_sz = buf_sz;
    void *buf = allocInBuckets(tp->buckets, buf_sz, 1);     // OPEN: if 0 return an error code?
    tp->buf = buf_sz ? buf : 0;
}

pub void TP_free(BK_TP *tp) {
    tp->buf_sz = 0;
}


// ---------------------------------------------------------------------------------------------------------------------
// tp api
// ---------------------------------------------------------------------------------------------------------------------

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

    TP_Cookie *c = allocInBuckets(tp->buckets, sizeof(TP_Cookie), bk_alignof(TP_Cookie));
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

pub int tp_sizeof(TPN x) {
    int answer = 0;
    if (tp_t(x) == tpvt_seq) {
        int n = tp_nseq(x);
        for (int i = 1; i <= n; i++) answer += tp_sizeof(tp_at(x, i));
    } else
        answer = tp_sz(x);
    return answer;
}

pvt char * _tp_s8(TPN x, char *p) {
    if (tp_t(x) == tpvt_s8) {
        size n = tp_sz(x);
        memcpy(p, x.p, n);
        return p + n;
    } else {
        size n = tp_nseq(x);
        for (int i = 1; i <= n; i++) p = _tp_s8(tp_at(x, i), p);
        return p;
    }
}

pub S8 tp_s8(BK_TP *tp, TPN x) {
    if (tp_t(x) == tpvt_seq) {
        // OPEN: rather than traversing twice we could reallocInBuckets (except it doesn't work yet)
        size32 n = tp_sizeof(x);
        char *buf = allocInBuckets(tp->buckets, n, 1);
        return (S8) {.sz = n, .cs = _tp_s8(x, buf)};
    } else
        return (S8) {.sz = tp_sz(x), .cs = x.p};
}

pub void tp_buf_printf(BK_TP *tp, char const *format, ...) {
    char *buf;  int buf_sz;  size n;  va_list args;
    buf = tp->buf + tp->end;
    buf_sz = tp->buf_sz - tp->end;
    va_start(args, format);
    n = vsnprintf(buf, buf_sz, format, args);
    if (n > buf_sz) {
        // create a new buffer from buckets, forgetting the location of the old one
        buf_sz = TP_BUF_INC;
        while (n > buf_sz) buf_sz += TP_BUF_INC;
        buf = allocInBuckets(tp->buckets, buf_sz, 1);
        if (buf == 0) return;
        tp->buf = buf;
        tp->buf_sz = buf_sz;
        tp->end = 0;
        n = vsnprintf(buf, buf_sz, format, args);
    }
    tp->end += n;
    va_end(args);
}

pub TPN tp_buf_flush(BK_TP *tp) {
    size start = tp->start, end = tp->end;
    tp->start = tp->end;
    return (TPN) {.p = tp->buf + start, .vtsz = tp_as_s8((end - start))};
}

// BK_TP *tp -> TPC ctx, ... ?
//pub void tp_concat(BK_TP *tp, ...) {
//    TPN *buf;  int avail, buf_sz;  size n;  va_list args;  TPN answer, x;  size count = 0;
//    va_start(args, tp);
//    x = va_arg(args, TPN);

//    x.vtsz = tpvt_empty;
//    while (tp_t(x) != tpvt_err) {
//        count ++;
//        if (count == 1) {
//            answer = x;
//        } else if (count == 2) {
//            buf = allocInBuckets(tp->buckets, sizeof(TPN*) * (count + 1), bk_alignof(TPN*));
//            memset(buf, 0, sizeof(TPN*) * (count + 1));
//            buf[0] = count;
//            buf[1] = answer;
//            buf[2] = x;
//        } else {
//            buf = reallocInBuckets(tp->buckets, buf, sizeof(TPN*) * (count + 1), bk_alignof(TPN*));
//        }
//        x = va_arg(args, TPN);
//    }
//
//    if (n > buf_sz) {
//        // create a new buffer from buckets, forgetting the location of the old one
//        buf_sz = TP_BUF_INC;
//        while (n > buf_sz) buf_sz += TP_BUF_INC;
//        buf = allocInBuckets(tp->buckets, buf_sz, 1);
//        if (buf == 0) return;
//        tp->buf = buf;
//        tp->buf_sz = buf_sz;
//        tp->end = 0;
//        n = vsnprintf(buf, buf_sz, format, args);
//    }
//    tp->end += n;
//    va_end(args);
//}

pub TPN tp_printf(BK_TP *tp, char const *format, ...) {
    // OPEN: split format into chunks, adding %tp as a valid format and maybe %s8
    va_list args;
    va_start(args, format);
    tp_buf_printf(tp, format, args);
    va_end(args);
    return tp_buf_flush(tp);
}


#endif      // SRC_BK_TP_C
