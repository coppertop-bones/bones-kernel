// ---------------------------------------------------------------------------------------------------------------------
// Copyright 2025 David Briant, https://github.com/coppertop-bones. Licensed under the Apache License, Version 2.0
//
// TP - TEXT PAD
// OPEN:
//  - handle case where text is longer than the buffer increment in buckets, e.g. use BDW or start collecting a TPN
//    in increment sizes
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

// tp FILE api
#if defined _WIN64 || defined _WIN32
#include "tp_win64.c"
#elif defined _APPLE_ || defined __MACH__
#include "tp_macos.c"
#elif defined __linux__
#include "tp_linux.c"
#endif

#define TPN_SZ_MASK 0x3fffffffffffffff
#define TPN_TP_MASK 0xc000000000000000

#define TPN_VT_S8       0x0000000000000000     /* 00 -> 0000 - S8 */
#define TPN_VT_SEQ      0x4000000000000000     /* 40 -> 0100 - length prefixed sequence of nodes */
#define TPN_VT_SLICE    0x8000000000000000     /* 80 -> 1000 - slice */
#define TPN_VT_RESERVED 0xc000000000000000     /* c0 -> 1100 - reserved for future use */


// utils

#define _tp_encode_as_s8(n) ((n) | TPN_VT_S8)
#define _tp_encode_as_seq(n) ((n) | TPN_VT_SEQ)
#define _tp_encode_as_slice(n) ((n) | TPN_VT_SLICE)
#define _tpn_t(x) (x.vtsz & TPN_TP_MASK)
#define _tpn_sz(x) (size)((x.vtsz & TPN_SZ_MASK))
#define _tpn_at(x, i) (((TPN*) x.p)[i])
#define _tpn_nseq(x) (*((int *)x.p))

pvt char * _tp_render(TPN x, char *buf);


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

pub void TP_finalise(BK_TP *tp) {
    tp->buf_sz = 0;
}


// ---------------------------------------------------------------------------------------------------------------------
// tp api
// ---------------------------------------------------------------------------------------------------------------------

pub int tp_sz(TPN x) {
    int answer = 0;
    if (_tpn_t(x) == TPN_VT_SEQ) {
        int n = _tpn_nseq(x);
        for (int i = 1; i <= n; i++) answer += tp_sz(_tpn_at(x, i));
    } else
        answer = _tpn_sz(x);
    return answer;
}

pub S8 tp_s8(BK_TP *tp, TPN x) {
    // gets total size, allocates in buckets, materialises and returns an s8 pointing to the buffer
    // traverse twice rather than reallocInBuckets (which doesn't work yet) as counting is faster than reallocation
    if (_tpn_t(x) == TPN_VT_SEQ || _tpn_t(x) == TPN_VT_SLICE) {
        size32 n = tp_sz(x);
        char *start = allocInBuckets(tp->buckets, n + 1, 1);
        char *end = _tp_render(x, start);
        *end = 0;
        return (S8) {.sz = n, .cs = start};
    } else {
        // TPN_VT_S8
        return (S8) {.sz = _tpn_sz(x), .cs = x.p};
    }
}

pub void tp_buf_printf(BK_TP *tp, char const *template, ...) {
    // as printf but allocates memory from the textpad's buckets
    // OPEN: if we run out of buffer and previous size > 0 then start a sequence rather than copying the prior buffered
    char *buf;  size buf_sz, n, total;  va_list args;
    buf = tp->buf + tp->end;
    buf_sz = tp->buf_sz - tp->end;
    va_start(args, template);
    n = vsnprintf(buf, buf_sz, template, args);
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
        n = vsnprintf(buf, buf_sz, template, args);                           // n does not include null terminator
    }
    tp->end += n;
    va_end(args);
}

pub TPN tp_flush(BK_TP *tp) {
    // advances the buffer pointers and answers a tpn
    // OPEN: handle case where we go over the bucket boundary, i.e. answer a sequence
    size32 start = tp->start;
    tp->start = tp->end;
    return (TPN) {.vtsz = _tp_encode_as_slice(tp->end - start), .p = tp->buf + start};
}

pub TPN tp_flush_with_null(BK_TP *tp) {
    // advances the buffer pointers and answers a null terminated tpn (copying if necessary)
    if (tp->end < tp->buf_sz) {
        size32 start = tp->start, end = tp->end;
        *(tp->buf + end) = 0;
        tp->start = tp->end = tp->end + 1;
        return (TPN) {.vtsz = _tp_encode_as_s8(end - start), .p = tp->buf + start};
    } else {
        // OPEN: test this branch
        // allocate a new buffer from buckets that can contain the unflushed buffer and the new formatted output then
        // copy the data we haven't flushed yet from the old buffer into the new buffer and finally add the null
        size32 buf_sz = TP_BUF_INC;
        size32 n = tp->end - tp->start + 1;                     // + 1 for null terminator
        while (n > buf_sz) buf_sz += TP_BUF_INC;
        char *buf = allocInBuckets(tp->buckets, buf_sz, 1);
        if (buf == 0) return (TPN) {.vtsz = 0, .p = 0};         // OPEN: check that allocInBuckets sets an error
        memcpy(buf, tp->buf + tp->start, tp->end - tp->start);
        *(tp->buf + tp->end - 1) = 0;
        tp->buf = buf;
        tp->buf_sz = buf_sz;
        tp->start = 0;
        tp->end = n;
        // OPEN: instead answer a sequence with a SLICE wrapping the old sequence. to the old buffer,
        return (TPN) {.vtsz = _tp_encode_as_s8(n - 1), .p = tp->buf};
    }
}

// BK_TP *tp -> TPC ctx, ... ?
//pub void tp_concat(BK_TP *tp, ...) {

// either null terminate the arg list or length prefix e.g.:

//size_t __print__(char * str1, ...) {
//    va_list args;
//    va_start(args, str1);
//    size_t out_char = 0;
//    char * tmp_str;
//    while((tmp_str = va_arg(args, char *)) != NULL)
//    out_char = out_char + write(1, tmp_str,strlen(tmp_str));
//    va_end(args);
//    return out_char;
//}

//    TPN *buf;  int avail, buf_sz;  size n;  va_list args;  TPN answer, x;  size count = 0;
//    va_start(args, tp);
//    x = va_arg(args, TPN);

//    x.vtsz = tpvt_empty;
//    while (_tpn_t(x) != tpvt_err) {
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
//        n = vsnprintf(buf, buf_sz, template, args);
//    }
//    tp->end += n;
//    va_end(args);
//}

pub TPN tp_tpn_printf(BK_TP *tp, char const *template, ...) {
    // as printf but instead answers a TPN of the formatted output
    // OPEN: split template into chunks, adding %tp as a valid template and maybe %s8
    va_list args;
    va_start(args, template);
    tp_buf_printf(tp, template, args);
    va_end(args);
    return tp_flush(tp);
}


// ---------------------------------------------------------------------------------------------------------------------
// utils
// ---------------------------------------------------------------------------------------------------------------------

pvt char * _tp_render(TPN x, char *buf) {
    if (_tpn_t(x) == TPN_VT_S8 || _tpn_t(x) == TPN_VT_SLICE) {
        size n = _tpn_sz(x);
        memcpy(buf, x.p, n);
        return buf + n;
    } else {
        // sequence
        size n = _tpn_nseq(x);
        for (int i = 1; i <= n; i++) buf = _tp_render(_tpn_at(x, i), buf);
        return buf;
    }
}


#undef _tp_encode_as_s8
#undef _tp_encode_as_seq
#undef _tp_encode_as_slice
#undef _tpn_t
#undef _tpn_sz
#undef _tpn_at
#undef _tpn_nseq

#endif      // SRC_BK_TP_C
