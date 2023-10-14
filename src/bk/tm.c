#ifndef __BK_BTYPE_C
#define __BK_BTYPE_C "bk/btype.c"


#include "../../include/all.cfg"
#include "buckets.c"
#include "../../include/bk/mm.h"
#include "../../include/bk/tm.h"


tdd int tm_newNominal(char *name, struct TM *tm) {
    // get symId for name
    // check doesn't exist
    return 1;
}


tdd int tm_setNominal(char *name, btype bt, struct TM *tm) {
    // get symId for name
    // check doesn't exist
    return 1;
}


pub struct TM * TM_create(struct MM *mm, struct SM *sm) {
    struct TM *tm = (struct TM *) mm->malloc(sizeof(struct TM));
    tm->mm = mm;
    tm->sm = sm;
//    tm->bType_byBTypeId = (struct BType *) malloc(1001 * sizeof(struct BType));

    tm_setNominal("nat", _nat, tm);
    tm_setNominal("m8", _m8, tm);
    tm_setNominal("m16", _m16, tm);
    tm_setNominal("m32", _m32, tm);
    tm_setNominal("m64", _m64, tm);
    tm_setNominal("p64", _p64, tm);
    tm_setNominal("p64", _litint, tm);
    tm_setNominal("p64", _i32, tm);

    return tm;
}

pub int TM_trash(struct TM *tm) {
    tm->mm->free(tm);
    return 0;
}


#endif  // __BK_BTYPE_C