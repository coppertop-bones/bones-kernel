// ---------------------------------------------------------------------------------------------------------------------
// Copyright 2025 David Briant, https://github.com/coppertop-bones. Licensed under the Apache License, Version 2.0
//
// TP - TEXT PAD
// ---------------------------------------------------------------------------------------------------------------------

#ifndef BK_TP_H
#define BK_TP_H "tp.h"
#define BK_TP_H "tp.h"

#include "stdio.h"
#include "mm.h"


typedef struct {
    Buckets *buckets;   // 8
    char *buf;          // 8
    size buf_sz;        // 8
    size32 start;       // 4
    size32 end;         // 4
    TPN *wip;           // 8
} BK_TP;

pub void TP_init(BK_TP *, size, Buckets *);
pub void TP_finalise(BK_TP *);

pub TPN tp_concat(BK_TP *, size, ...);
pub void tp_buf_printf(BK_TP *, char const *format, ...);
pub TPN tp_tpn_printf(BK_TP *, char const *format, ...);
pub S8 tp_s8(BK_TP *, TPN);
pub int tp_sz(TPN);
pub TPN tp_flush(BK_TP *);
pub TPN tp_flush_with_null(BK_TP *);


#endif      // BK_TP_H
