// ---------------------------------------------------------------------------------------------------------------------
// Copyright 2025 David Briant, https://github.com/coppertop-bones. Licensed under the Apache License, Version 2.0
//
// K - KERNEL
// ---------------------------------------------------------------------------------------------------------------------

#ifndef INC_BK_K_H
#define INC_BK_K_H "bk/k.h"

#include "mm.h"
#include "sm.h"
#include "em.h"
#include "tm.h"
#include "tp.h"

#define BUCKETS_CHUNK_SIZE 4096*4

// page manager would allocate number of pages and return to os and reuse them

typedef struct {
    BK_MM *mm;
    Buckets *buckets;   // buckets can be used by TP and for RST
    BK_SM *sm;
    BK_EM *em;
    BK_TM *tm;
    void *om;           // kernel may or may not have an object manager
    BK_TP tp;           // used by the TM to print type lists etc
} BK_K;

pub BK_K * K_create(BK_MM *mm, Buckets *buckets);
pub int K_trash(BK_K *k);

#endif // INC_BK_K_H

