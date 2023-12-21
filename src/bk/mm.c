// ---------------------------------------------------------------------------------------------------------------------
//                                                Memory Manager
// ---------------------------------------------------------------------------------------------------------------------

#ifndef __BK_MM_C
#define __BK_MM_C "bk/mm.c"

#include "../../include/bk/mm.h"


pub BK_MM * MM_create() {
    BK_MM *mm = (BK_MM *) malloc(sizeof(BK_MM));
    mm->malloc = malloc;
    mm->realloc = realloc;
    mm->free = free;
    return mm;
}

pub int MM_trash(BK_MM *mm) {
    free(mm);
    return 0;
}

#endif // __BK_MM_C
