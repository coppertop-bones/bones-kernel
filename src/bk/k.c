// ---------------------------------------------------------------------------------------------------------------------
// K - KERNEL
// ---------------------------------------------------------------------------------------------------------------------

#ifndef __BK_K_C
#define __BK_K_C "bk/k.c"

#include "../../include/bk/k.h"
#include "tp.c"
#include "sm.c"
#include "em.c"
#include "tm.c"

pub BK_K * K_create(BK_MM *mm, Buckets *buckets) {
    BK_K *k = mm->malloc(sizeof(BK_K));
    k->mm = mm;
    k->buckets = buckets;
    k->sm = SM_create(mm);
    k->em = EM_create(mm, k->sm);
    TP_init(&(k->tp), 0, buckets);
    k->tm = TM_create(mm, k->buckets, k->sm, &(k->tp));

    int n = 0;
    BK_TM *tm = k->tm;
    n += tm_nominal(tm, B_M8, 0, "m8") == 0;
    n += tm_nominal(tm, B_M16, 0, "m16") == 0;
    n += tm_nominal(tm, B_M32, 0, "m32") == 0;
    n += tm_nominal(tm, B_M64, 0, "m64") == 0;
    n += tm_nominal(tm, B_LITINT, 0, "litint") == 0;
    n += tm_nominal(tm, B_I32, 0, "i32") == 0;

    if (n) {
        mm->free(tm);
        die("%i conflicts in tm_nominal\n", n);
    }
    return k;
}

pub int K_trash(BK_K *k) {
    TM_trash(k->tm);
    EM_trash(k->em);
    SM_trash(k->sm);
    k->mm->free(k);
    return 0;
}

#endif // __BK_K_C
