// ---------------------------------------------------------------------------------------------------------------------
// Copyright 2025 David Briant, https://github.com/coppertop-bones. Licensed under the Apache License, Version 2.0
//
// SCHEMA VARIABLE IMPLEMENTATION
// KEEPER REQUISITES: core
// ---------------------------------------------------------------------------------------------------------------------

#ifndef SRC_BK_TM_SCHEMAVAR_C
#define SRC_BK_TM_SCHEMAVAR_C "bk/tm/schemavar.c"


#include "core.c"



pub btypeid_t tm_schemavar(BK_TM *tm, btypeid_t btype) {
    // initialises btype as a schema variable (allocating if necessary) and returns btype or B_NAT if already initialized
    if (!btype) return _err_invalid_btype_B_NAT(B_NAT, __FILE__, FN_NAME, __LINE__);
    else if (btype >= tm->next_btypeId) return _err_btypeidOutOfRange(B_NAT, __FILE__, FN_NAME, __LINE__, btype);
    else if (btype == B_NEW) btype = tm->next_btypeId;
    else if (TM_BMT_ID(tm->btsummary_by_btypeid[btype]) != bmterr) return B_NAT;
    btype = _update_type_summary(tm, btype, bmtsvr, 0, true);
    return btype;
}

#endif  // SRC_BK_TM_SCHEMAVAR_C