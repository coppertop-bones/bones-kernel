// ---------------------------------------------------------------------------------------------------------------------
// K - KERNEL
// ---------------------------------------------------------------------------------------------------------------------

#ifndef INC_BK_K_H
#define INC_BK_K_H "bk/k.h"

#include "mm.h"
#include "sm.h"
#include "em.h"
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

#endif // INC_BK_K_H

