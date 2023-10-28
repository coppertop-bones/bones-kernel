// ---------------------------------------------------------------------------------------------------------------------
//                                                Memory Manager
// ---------------------------------------------------------------------------------------------------------------------

#ifndef __BK_MM_C
#define __BK_MM_C "bk/mm.c"

#include "../../include/bk/mm.h"


pub struct MM * MM_create() {
    struct MM *mm = (struct MM *) malloc(sizeof(struct MM));
    mm->malloc = malloc;
    mm->realloc = realloc;
    mm->free = free;
    return mm;
}

pub int MM_trash(struct MM *mm) {
    free(mm);
    return 0;
}

#endif // __BK_MM_C
