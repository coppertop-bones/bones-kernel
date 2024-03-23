// ---------------------------------------------------------------------------------------------------------------------
// TP - TEXT PAD
// ---------------------------------------------------------------------------------------------------------------------

#ifndef SRC_BK_TP_C
#define SRC_BK_TP_C "bk/tp.c"

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "../../include/bk/bk.h"
#include "../../include/bk/tp.h"

#define TP_BUF_INC 0x10
//#define TP_BUF_INC 0x8000

#define TPN_SZ_MASK 0x3fffffffffffffff
#define TPN_TP_MASK 0xc000000000000000

#define TPN_VT_S8       0x0000000000000000     /* 00 -> 0000 - S8 */
#define TPN_VT_SEQ      0x4000000000000000     /* 40 -> 0100 - sequence of nodes */
#define TPN_VT_SLICE    0x8000000000000000     /* 80 -> 1000 - slice */
#define TPN_VT_RESERVED 0xc000000000000000     /* c0 -> 1100 - reserved for future use */

#define tp_encode_as_s8(n) ((n) | TPN_VT_S8)
#define tp_encode_as_seq(n) ((n) | TPN_VT_SEQ)
#define tp_encode_as_slice(n) ((n) | TPN_VT_SLICE)
#define tp_t(x) (x.vtsz & TPN_TP_MASK)
#define tp_sz(x) (size)((x.vtsz & TPN_SZ_MASK))
#define tp_at(x, i) (((TPN*) x.p)[i])
#define tp_nseq(tp) (*((int *)tp.p))


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

pub int tp_sizeof(TPN x) {
    int answer = 0;
    if (tp_t(x) == TPN_VT_SEQ) {
        int n = tp_nseq(x);
        for (int i = 1; i <= n; i++) answer += tp_sizeof(tp_at(x, i));
    } else
        answer = tp_sz(x);
    return answer;
}

pub char * tp_render(TPN x, char *buf) {
    if (tp_t(x) == TPN_VT_S8 || tp_t(x) == TPN_VT_SLICE) {
        size n = tp_sz(x);
        memcpy(buf, x.p, n);
        return buf + n;
    } else {
        // sequence
        size n = tp_nseq(x);
        for (int i = 1; i <= n; i++) buf = tp_render(tp_at(x, i), buf);
        return buf;
    }
}

pub S8 tp_s8(BK_TP *tp, TPN x) {
    // gets total size, allocates in buckets, materialises and returns an s8 pointing to the buffer
    // traverse twice rather than reallocInBuckets (which doesn't work yet) as counting is faster than reallocation
    if (tp_t(x) == TPN_VT_SEQ || tp_t(x) == TPN_VT_SLICE) {
        size32 n = tp_sizeof(x);
        char *start = allocInBuckets(tp->buckets, n + 1, 1);
        char *end = tp_render(x, start);
        *end = 0;
        return (S8) {.sz = n, .cs = start};
    } else {
        // TPN_VT_S8
        return (S8) {.sz = tp_sz(x), .cs = x.p};
    }
}

pub void tp_pb_printf(BK_TP *tp, char const *format, ...) {
    // as printf but allocates memory from the textpad's buckets
    // OPEN: if we run out of buffer and previous size > 0 then start a sequence rather than copying the prior buffered
    char *buf;  size buf_sz, n, total;  va_list args;
    buf = tp->buf + tp->end;
    buf_sz = tp->buf_sz - tp->end;
    va_start(args, format);
    n = vsnprintf(buf, buf_sz, format, args);
    if (n > buf_sz) {
        // allocate a new buffer from buckets that can contain the unflushed buffer and the new formatted output then
        // copy the data we haven't flushed yet from the old buffer into the new buffer and finally append the
        // formatted output
        buf_sz = TP_BUF_INC;
        total = tp->end - tp->start + n + 1;
        while (total > buf_sz) buf_sz += TP_BUF_INC;
        buf = allocInBuckets(tp->buckets, buf_sz, 1);
        if (buf == 0) return;                                               // OPEN: need to set an error
        memcpy(buf, tp->buf + tp->start, tp->end - tp->start);
        tp->buf = buf;
        tp->buf_sz = buf_sz;
        tp->start = 0;
        tp->end = tp->end - tp->start;
        n = vsnprintf(buf, buf_sz, format, args);                           // n does not include null terminator
    }
    tp->end += n;
    va_end(args);
}

pub TPN tp_snap(BK_TP *tp) {
    // advances the buffer pointers and answers a tpn
    size32 start = tp->start;
    tp->start = tp->end;
    return (TPN) {.p = tp->buf + start, .vtsz = tp_encode_as_slice(tp->end - start)};
}

pub TPN tp_snap_with_null(BK_TP *tp) {
    // advances the buffer pointers and answers a null terminated tpn (copying if necessary)
    if (tp->end < tp->buf_sz) {
        tp->end += 1;
    } else {
        // OPEN: test this branch
        // allocate a new buffer from buckets that can contain the unflushed buffer and the new formatted output then
        // copy the data we haven't flushed yet from the old buffer into the new buffer and finally add the null
        size32 buf_sz = TP_BUF_INC;
        size32 n = tp->end - tp->start + 1;                     // + 1 for null terminator
        while (n > buf_sz) buf_sz += TP_BUF_INC;
        char *buf = allocInBuckets(tp->buckets, buf_sz, 1);
        if (buf == 0) return (TPN) {.p = 0, .vtsz = 0};         // OPEN: check that allocInBuckets sets an error
        memcpy(buf, tp->buf + tp->start, tp->end - tp->start);
        tp->buf = buf;
        tp->buf_sz = buf_sz;
        tp->start = 0;
        tp->end = n;
    }
    *(tp->buf + tp->end - 1) = 0;
    return (TPN) {.p = tp->buf + tp->start, .vtsz = tp_encode_as_s8(tp->end - tp->start - 1)};
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

pub TPN tp_pp_printf(BK_TP *tp, char const *format, ...) {
    // as printf but instead answers a TPN of the formatted output
    // OPEN: split format into chunks, adding %tp as a valid format and maybe %s8
    va_list args;
    va_start(args, format);
    tp_pb_printf(tp, format, args);
    va_end(args);
    return tp_snap(tp);
}


// ---------------------------------------------------------------------------------------------------------------------
// tp FILE api
// ---------------------------------------------------------------------------------------------------------------------


#if defined _WIN64 || defined _WIN32
#include "os_win64.c"
#elif defined _APPLE_ || defined __MACH__
#include "tp_macos.c"
#elif defined __linux__
#include "os_linux.c"
#endif


#endif      // SRC_BK_TP_C
