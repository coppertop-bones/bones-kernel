#ifndef __BK_EM_C
#define __BK_EM_C "bk/Em.c"

#include "../../include/bk/mm.h"
#include "../../include/bk/em.h"

pub struct EM *EM_create(struct MM *mm, struct SM *sm) {
    struct EM *em = (struct EM*) mm->malloc(sizeof(struct EM));
    em->mm = mm;
    em->sm = sm;
    return em;
}

pub int EM_trash(struct EM *em) {
    em->mm->free(em);
    return 0;
}


#endif // __BK_EM_C
