// ---------------------------------------------------------------------------------------------------------------------
//                                                     Text Pad
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


#define tp_as_s8(n) (n << 32 | tpvt_s8)
#define tp_as_seq(n) (n << 32 | tpvt_s8)
#define tp_t(x) (enum tpvt)(x.opaque & 0x00000000FFFFFFFF)
#define tp_sz(x) (size)((x.opaque & 0xFFFFFFFF00000000) >> 32)
#define tp_at(x, i) (((TPN*) x.p)[i])
#define tp_nseq(tp) (*((int *)tp.p))


enum tpvt {
    tpvt_err = 0,
    tpvt_s8 = 1,
    tpvt_seq = 2,
    tpvt_empty = 3,
};

struct TP_ {
    Buckets *buckets;   // 8
    char *buf;          // 8
    size buf_sz;        // 8
    size32 start;       // 4
    size32 end;         // 4
};

union TP_PTP {
    BK_TP *tp;
    struct TP_ *_tp;
};

struct TP_pub {
    char reserved[sizeof(struct TP_)];
};

struct TP_Cookie {
    struct TP_ *ptp;
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
//    n = (n * 207) >> 6;
//    n = (n >> 1) + (n & 1);
    n = n + TP_BUF_INC;
    return n;
}

pvt int tp_grow_buf_if_needed(struct TP_Cookie *cookie, size required) {
    if (cookie->cursor > SIZE_MAX - required) {
        errno = EOVERFLOW;
        return -1;
    }
    required += cookie->cursor;

    size newsize = cookie->ptp->buf_sz;
    if (required <= newsize) return 0;

    while (required > newsize) newsize = tp_new_size(newsize);

    // OPEN: reallocInBuckets does not copy - handle this
    char *p = reallocInBuckets(cookie->ptp->buckets, cookie->ptp->buf, newsize, 1);
    if (!p) return -1;

    cookie->ptp->buf = p;
    cookie->ptp->buf_sz = newsize;
    return 0;
}

pvt int tp_apply_cursor(struct szbuf *buf, struct TP_Cookie *c) {
    if (c->ptp->buf_sz < c->cursor) return -1;
    buf->p = c->ptp->buf + c->cursor;
    buf->sz = c->ptp->buf_sz - c->cursor;
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
    struct TP_Cookie *c = p;

    struct szbuf from = { sz, (char *) buf };
    struct szbuf to;

    if (tp_grow_buf_if_needed(c, sz) < 0) return -1;
    if (tp_apply_cursor(&to, c) < 0) return 0;

    size copied = tp_copy(&to, &from);
    c->cursor += copied;
    if (c->ptp->end < c->cursor) c->ptp->end = c->cursor;
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
    struct TP_Cookie *c = p;
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
    struct TP_Cookie *c = p;
    size newoff;
    switch (whence) {
        case SEEK_SET: newoff = off; break;
        case SEEK_CUR: newoff = c->cursor + off; break;
        case SEEK_END: newoff = c->ptp->end + off; break;
        default: errno = EINVAL; return -1;
    }
    if (newoff > c->ptp->end || (off_t)newoff < 0 || newoff > (size)OFF_MAX) {
        errno = EOVERFLOW;
        return -1;
    }
    c->cursor = newoff;
    return newoff;
}

pvt int tp_close(void *p) {
//    free(p);
    return 0;
}


// ---------------------------------------------------------------------------------------------------------------------
// TP lifecycle
// ---------------------------------------------------------------------------------------------------------------------

pub void TP_init(BK_TP *tp, size initSz, Buckets *buckets) {
    union TP_PTP cv = { .tp = tp };
    cv._tp->start = 0;
    cv._tp->end = 0;
    cv._tp->buckets = buckets;
    int buf_sz = 0;
    while (buf_sz < initSz) buf_sz += TP_BUF_INC;
    cv._tp->buf_sz = buf_sz;
    void *buf = allocInBuckets(cv._tp->buckets, buf_sz, 1);     // OPEN: if 0 return an error code?
    cv._tp->buf = buf_sz ? buf : 0;
}

pub void TP_free(BK_TP *tp) {
    union TP_PTP cv = { .tp =  tp };
    cv._tp->buf_sz = 0;
}


// ---------------------------------------------------------------------------------------------------------------------
// tp api
// ---------------------------------------------------------------------------------------------------------------------

tdd FILE *tp_open(BK_TP *tp, char const *mode) {
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
        pos = cv._tp->end;
    } else if (strcmp(mode, "a+") == 0) {
        // Open for reading and writing.  The file is created if it does not
        // exist.  The stream is positioned at the end of the file.  Subse-
        // quent writes to the file will always end up at the then current
        // end of file, irrespective of any intervening fseek(3) or similar.
        pos = cv._tp->end;
    } else return 0;

    if (!cv._tp->buf) {
        cv._tp->buf_sz = 128;
        cv._tp->buf = allocInBuckets(cv._tp->buckets, cv._tp->buf_sz, 1);
        bufAllocated = 1;
    }
    if (!cv._tp->buf) return 0;

    struct TP_Cookie *c = allocInBuckets(cv._tp->buckets, sizeof(struct TP_Cookie), alignof(struct TP_Cookie));
    if (!c) {
        if (bufAllocated) {
//            cv._tp->mm->free(cv._tp->buf);
            cv._tp->buf = 0;
            cv._tp->buf_sz = 0;
        }
        return 0;
    }
    c->ptp = cv._tp;
    c->cursor = pos;

    // https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man3/funopen.3.html
    FILE *f = funopen(c, tp_read, tp_write, tp_seek, tp_close);
    if (!f) {
        if (bufAllocated) {
//            cv._tp->mm->free(cv._tp->buf);
            cv._tp->buf = 0;
            cv._tp->buf_sz = 0;
        }
//        cv._tp->mm->free(c);
    }
    return f;
}

pub int tp_sizeof(TPN tp) {
    int answer = 0;
    if (tp_t(tp) == tpvt_seq) {
        int n = tp_nseq(tp);
        for (int i = 1; i <= n; i++) answer += tp_sizeof(tp_at(tp, i));
    } else
        return tp_sz(tp);
}

pvt char * tp_render_(TPN x, char *p) {
    if (tp_t(x) == tpvt_s8) {
        size n = tp_sz(x);
        memcpy(p, x.p, n);
        return p + n;
    } else {
        size n = tp_nseq(x);
        for (int i = 1; i <= n; i++) p = tp_render_(tp_at(x, i), p);
        return p;
    }
}

pub S8 tp_render(BK_TP *tp, TPN x) {
    union TP_PTP cv = { .tp = tp };
    if (tp_t(x) == tpvt_seq) {
        // OPEN: rather than traversing twice we could reallocInBuckets (except it doesn't work yet)
        size32 n = tp_sizeof(x);
        char *buf = allocInBuckets(cv._tp->buckets, n, 1);
        return (S8) {.opaque = n, .cs = tp_render_(x, buf)};
    } else
        return (S8) {.opaque = tp_sz(x), .cs = x.p};
}

pub void tp_printfb(BK_TP *tp, char const *format, ...) {
    char *buf;  int avail, buf_sz;  size n;  va_list args;
    union TP_PTP cv = { .tp = tp };
    buf = cv._tp->buf + cv._tp->end;
    buf_sz = cv._tp->buf_sz - cv._tp->end;
    va_start(args, format);
    n = vsnprintf(buf, buf_sz, format, args);
    if (n > buf_sz) {
        // create a new buffer from buckets, forgetting the location of the old one
        buf_sz = TP_BUF_INC;
        while (n > buf_sz) buf_sz += TP_BUF_INC;
        buf = allocInBuckets(cv._tp->buckets, buf_sz, 1);
        if (buf == 0) return;
        cv._tp->buf = buf;
        cv._tp->buf_sz = buf_sz;
        cv._tp->end = 0;
        n = vsnprintf(buf, buf_sz, format, args);
    }
    cv._tp->end += n;
    va_end(args);
}

// BK_TP *tp -> TPC ctx, ... ?
//pub void tp_concat(BK_TP *tp, ...) {
//    TPN *buf;  int avail, buf_sz;  size n;  va_list args;  TPN answer, x;  size count = 0;
//    union TP_PTP cv = { .tp = tp };
//    va_start(args, tp);
//    x = va_arg(args, TPN);
//    x.opaque = tpvt_empty;
//    while (tp_t(x) != tpvt_err) {
//        count ++;
//        if (count == 1) {
//            answer = x;
//        } else if (count == 2) {
//            buf = allocInBuckets(cv._tp->buckets, sizeof(TPN*) * (count + 1), alignof(TPN*));
//            memset(buf, 0, sizeof(TPN*) * (count + 1));
//            buf[0] = count;
//            buf[1] = answer;
//            buf[2] = x;
//        } else {
//            buf = reallocInBuckets(cv._tp->buckets, buf, sizeof(TPN*) * (count + 1), alignof(TPN*));
//        }
//        x = va_arg(args, TPN);
//    }
//
//    if (n > buf_sz) {
//        // create a new buffer from buckets, forgetting the location of the old one
//        buf_sz = TP_BUF_INC;
//        while (n > buf_sz) buf_sz += TP_BUF_INC;
//        buf = allocInBuckets(cv._tp->buckets, buf_sz, 1);
//        if (buf == 0) return;
//        cv._tp->buf = buf;
//        cv._tp->buf_sz = buf_sz;
//        cv._tp->end = 0;
//        n = vsnprintf(buf, buf_sz, format, args);
//    }
//    cv._tp->end += n;
//    va_end(args);
//}

pub TPN tp_printftp(BK_TP *tp, char const *format, ...) {
    // OPEN: split format into chunks, adding %tp as a valid format and maybe %s8
    va_list args;
    va_start(args, format);
    tp_printfb(tp, format, args);
    va_end(args);
    return tp_flush(tp);
}

pub TPN tp_flush(BK_TP *tp) {
    union TP_PTP cv = { .tp = tp };
    size start = cv._tp->start;  size end = cv._tp->end;
    cv._tp->start = cv._tp->end;
    return (TPN) {.p = cv._tp->buf + start, .opaque = tp_as_s8((end - start))};
}


#endif      // SRC_BK_TP_C
