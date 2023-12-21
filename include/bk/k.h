#ifndef API_BK_KERNEL_H
#define API_BK_KERNEL_H "bk/kernel.h"

#include "mm.h"
#include "sm.h"
#include "buckets.h"
#include "em.h"
#include "tm.h"
#include "om.h"
#include "tm.h"

#define BUCKETS_CHUNK_SIZE 4096*4

typedef struct {
    BK_MM *mm;
    Buckets *buckets;
    BK_SM *sm;
    BK_EM *em;
    BK_TM *tm;
    struct TPM *tp;
} BK_K;

pub BK_K * K_create(BK_MM *mm, Buckets *buckets);
pub int K_trash(BK_K *k);

#endif // API_BK_KERNEL_H

