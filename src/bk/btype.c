#ifndef __BK_BTYPE_C
#define __BK_BTYPE_C "bk/btype.c"


#include "../../include/all.cfg"
#include "buckets.c"
#include "../../include/bk/btype.h"


tdd int newNominal(char *name, struct BTypeManager *tm) {
    // get symId for name
    // check doesn't exist
    return 1;
}


tdd int setNominal(char *name, btype bt, struct BTypeManager *tm) {
    // get symId for name
    // check doesn't exist
    return 1;
}


pvt struct BTypeManager * newBTypeManager() {
    struct BTypeManager *tm = (struct BTypeManager *) malloc(sizeof(struct BTypeManager));
    tm->bType_byBTypeId = (struct BType*) malloc(1001 * sizeof(struct BType));

    setNominal("nat", _nat, tm);
    setNominal("m8", _m8, tm);
    setNominal("m16", _m16, tm);
    setNominal("m32", _m32, tm);
    setNominal("m64", _m64, tm);
    setNominal("p64", _p64, tm);
    setNominal("p64", _litint, tm);
    setNominal("p64", _i32, tm);

    return tm;
}


#endif  // __BK_BTYPE_C