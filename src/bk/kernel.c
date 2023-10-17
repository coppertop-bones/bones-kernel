#ifndef __BK_KERNEL_C
#define __BK_KERNEL_C "bk/kernel.c"

#include "../../include/bk/kernel.h"
#include "mm.c"
#include "sm.c"
#include "em.c"
#include "tm.c"


pub struct K * K_create(struct MM *mm) {
    struct K *k = (struct K *) mm->malloc(sizeof(struct K));
    k->mm = mm;
    k->sm = SM_create(mm);
    k->em = EM_create(mm, k->sm);
    k->tm = TM_create(mm, k->sm);

    int n = 0;
    struct TM *tm = k->tm;
    n += tm_setNominal(tm, "m8", _m8) == 0;
    n += tm_setNominal(tm, "m16", _m16) == 0;
    n += tm_setNominal(tm, "m32", _m32) == 0;
    n += tm_setNominal(tm, "m64", _m64) == 0;
    n += tm_setNominal(tm, "p64", _p64) == 0;
    n += tm_setNominal(tm, "litint", _litint) == 0;
    n += tm_setNominal(tm, "i32", _i32) == 0;

    if (n) {
        mm->free(tm);
        die("%i conflicts in tm_setNominal\n", n);
    }
    return k;
}

pub int K_trash(struct K *k) {
    TM_trash(k->tm);
    EM_trash(k->em);
    SM_trash(k->sm);
    k->mm->free(k);
    return 0;
}

#endif // __BK_KERNEL_C
