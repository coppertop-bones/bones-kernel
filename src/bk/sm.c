#ifndef __BK_SYM_C
#define __BK_SYM_C "bk/sym.c"

#include "../../include/bk/sm.h"

pub void sm_init(struct SM *sm) {
    // reserve a bunch (4GB) of virtual memory
    sm->istrings = malloc(0xFFFF);
    sm->next = 2;   // loose 2 bytes so 0 is an error code
    return;
}

pub RP sm_sym(struct SM *sm, char const * const name) {
    // can find it in hash? if so answer it
    // count num chars - ensure <= MAX SYM SIZE, if not set last err and return 0
    // set not sorted flag
    // place count at start
    //
    return (RP) 2;
}

pub char * sm_get(struct SM *sm, RP symRp) {
    return 0;
}

pub bool sm_le(struct SM *sm, RP a, RP b) {
    return false;
}

#endif // __BK_SYM_C
