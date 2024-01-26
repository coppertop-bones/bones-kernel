// ---------------------------------------------------------------------------------------------------------------------
// EM - ENUM MANAGER
// ---------------------------------------------------------------------------------------------------------------------


#ifndef __BK_EM_C
#define __BK_EM_C "bk/em.c"

#include "../../include/bk/mm.h"
#include "../../include/bk/em.h"

pub BK_EM *EM_create(BK_MM *mm, BK_SM *sm) {
    BK_EM *em = (BK_EM*) mm->malloc(sizeof(BK_EM));
    em->mm = mm;
    em->sm = sm;
    return em;
}

pub int EM_trash(BK_EM *em) {
    em->mm->free(em);
    return 0;
}


#endif // __BK_EM_C
