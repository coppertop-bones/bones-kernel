#include <errno.h>
#include <limits.h>
#include <libc.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/bk/bk.h"
#include "../../include/bk/tp.h"


struct PTPCookie {
    struct PTP *ptp;    
    size cursor;
};

typedef struct szbuf {
    size sz;
    char *p;
} szbuf;

pvt inline size tp_new_size(size n) {
    /* This effectively is a return ceil(n * φ).
       φ is approximatively 207 / (2^7), so we shift our result by
       6, then perform our ceil by adding the remainder of the last division
       by 2 of the result to itself. */
    n = (n * 207) >> 6;
    n = (n >> 1) + (n & 1);
    return n;
}

pvt int tp_grow_buf_if_needed(struct PTPCookie *cookie, size required) {
    if (cookie->cursor > SIZE_MAX - required) {
        errno = EOVERFLOW;
        return -1;
    }
    required += cookie->cursor;

    size newsize = cookie->ptp->buf_size;
    if (required <= newsize) return 0;

    while (required > newsize) newsize = tp_new_size(newsize);

    char *p = cookie->ptp->mm->realloc(cookie->ptp->buf, newsize);
    if (!p) return -1;

    cookie->ptp->buf = p;
    cookie->ptp->buf_size = newsize;
    return 0;
}

pvt int tp_apply_cursor(struct szbuf *buf, struct PTPCookie *c) {
    if (c->ptp->sz < c->cursor) return -1;
    buf->p = c->ptp->buf + c->cursor;
    buf->sz = c->ptp->buf_size - c->cursor;
    return 0;
}

pvt size tp_copy(struct szbuf *to, struct szbuf *from) {
    size copied = from->sz < to->sz ? from->sz : to->sz;
    memcpy(to->p, from->p, copied);
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
    struct PTPCookie *c = p;

    struct szbuf from = { sz, (char *) buf };
    struct szbuf to;

    if (tp_grow_buf_if_needed(c, sz) < 0) return -1;
    if (tp_apply_cursor(&to, c) < 0) return 0;

    size copied = tp_copy(&to, &from);
    c->cursor += copied;
    if (c->ptp->sz < c->cursor) c->ptp->sz = c->cursor;
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
    struct PTPCookie *c = p;
    struct szbuf from;
    struct szbuf to = { sz, buf };

    if (tp_apply_cursor(&from, c) < 0) return 0;

    size copied = tp_copy(&to, &from);
    c->cursor += copied;
    if (copied > INT_MAX) {
        errno = EOVERFLOW;
        return -1;
    }
    return copied;
}

pvt off_t tp_seek(void *p, off_t off, int whence) {
    struct PTPCookie *c = p;
    size newoff;
    switch (whence) {
        case SEEK_SET: newoff = off; break;
        case SEEK_CUR: newoff = c->cursor + off; break;
        case SEEK_END: newoff = c->ptp->sz + off; break;
        default: errno = EINVAL; return -1;
    }
    if (newoff > c->ptp->sz || (off_t)newoff < 0 || newoff > (size)OFF_MAX) {
        errno = EOVERFLOW;
        return -1;
    }
    c->cursor = newoff;
    return newoff;
}

pvt int tp_close(void *p) {
    free(p);
    return 0;
}


// ---------------------------------------------------------------------------------------------------------------------
// TP lifecycle
// ---------------------------------------------------------------------------------------------------------------------

tdd void tp_init(TP *tp, size initSz, struct MM *mm) {
    union TP_PTP cv = { .tp = tp };                     // UB?
    cv.ptp->sz = 0;
    cv.ptp->mm = mm;
    cv.ptp->buf_size = initSz;
    cv.ptp->buf = initSz ? cv.ptp->mm->malloc(initSz) : 0;
}

tdd void tp_free(TP *tp) {
    union TP_PTP cv = { .tp = tp };
    free(cv.ptp->buf);
    cv.ptp->buf_size = 0;
}


// ---------------------------------------------------------------------------------------------------------------------
// tp api
// ---------------------------------------------------------------------------------------------------------------------

tdd FILE *tp_open(TP *tp, char const *mode) {
    b32 bufAllocated = 0;  size pos;
    union TP_PTP cv = {.tp = tp};

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
        pos = cv.ptp->sz;
    } else if (strcmp(mode, "a+") == 0) {
        // Open for reading and writing.  The file is created if it does not
        // exist.  The stream is positioned at the end of the file.  Subse-
        // quent writes to the file will always end up at the then current
        // end of file, irrespective of any intervening fseek(3) or similar.
        pos = cv.ptp->sz;
    } else return 0;

    if (!cv.ptp->buf) {
        cv.ptp->buf_size = 128;
        cv.ptp->buf = cv.ptp->mm->malloc(cv.ptp->buf_size);
        bufAllocated = 1;
    }
    if (!cv.ptp->buf) return 0;

    struct PTPCookie *c = cv.ptp->mm->malloc(sizeof (struct PTPCookie));
    if (!c) {
        if (bufAllocated) {
            cv.ptp->mm->free(cv.ptp->buf);
            cv.ptp->buf_size = 0;
        }
        return 0;
    }
    c->ptp = cv.ptp;
    c->cursor = pos;

    // https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man3/funopen.3.html
    FILE *f = funopen(c, tp_read, tp_write, tp_seek, tp_close);
    if (!f) {
        if (bufAllocated) {
            cv.ptp->mm->free(cv.ptp->buf);
            cv.ptp->buf_size = 0;
        }
        cv.ptp->mm->free(c);
    }
    return f;
}

tdd s8 tp_getS8(TP *tp) {
    // OPEN: may need flushing, e.g. we may have a list of slices
    union TP_PTP cv = { .tp = tp };
    return (s8) {.szs = cv.ptp->sz, .cs = cv.ptp->buf};
}


