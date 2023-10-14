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
