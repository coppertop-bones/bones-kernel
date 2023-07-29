// ---------------------------------------------------------------------------------------------------------------------
// Copyright 2025 David Briant, https://github.com/coppertop-bones. Licensed under the Apache License, Version 2.0
//
// TUPLE IMPLEMENTATION
// KEEPER REQUISITES: core
// ---------------------------------------------------------------------------------------------------------------------

#ifndef __BK_TM_TUPLE_C
#define __BK_TM_TUPLE_C "bk/tm/tuple.c"


#include "core.c"



pub btypeid_t tm_tuple(BK_TM *tm, btypeid_t btypeid, TM_TLID_T tlid) {
    i32 i, outcome, numTypes;  btsummary *sum;  TM_DETAILID_T tupid;  u32 idx;  bool hasT;
    btypeid_t *typelist, other;

    // answers the validated tuple type corresponding to typelist, creating if necessary

    if (!btypeid) return _err_invalid_btype_B_NAT(B_NAT, __FILE__, FN_NAME, __LINE__);
    if (btypeid >= tm->next_btypeId) return _err_btypeidOutOfRange(B_NAT, __FILE__, FN_NAME, __LINE__, btypeid);

    // get the btypeid for the tlid
    idx = hi_put_idx(TM_DETAILID_BY_TLIDHASH, tm->tupid_by_tlidhash, tlid, &outcome);
    switch (outcome) {
        default:
            die("%s:%i: HI_TOMBSTONE!", FN_NAME, __LINE__);
        case HI_LIVE:
            tupid = tm->tupid_by_tlidhash->tokens[idx];
            if (btypeid == B_NEW) return tm->btypid_by_tupid[tupid];
            else if (TM_BMT_ID(tm->btsummary_by_btypeid[btypeid]) != bmttup)
                return _err_btypeAlreadyInitialised(B_NAT, __FILE__, __LINE__, btypeid);
            else if (btypeid == (other = tm->btypid_by_tupid[tupid])) return btypeid;
            else return _err_otherAlreadyRepresentsTL(B_NAT, __FILE__, __LINE__, btypeid, other);
        case HI_EMPTY:
            // missing so commit the tuple type for tlid
            if (btypeid == B_NEW)
                btypeid = tm->next_btypeId;
            else if (TM_BMT_ID(tm->btsummary_by_btypeid[btypeid]) != bmterr)
                // btypeid is already in use so given the type list lookup above we cannot be referring to the same btype
                return _err_btypeAlreadyInitialised(B_NAT, __FILE__, __LINE__, btypeid);
            tupid = tm->next_tupid++;
            if (tupid >= tm->max_tupid) {
                tm->max_tupid += TM_MAX_ID_INC_SIZE;
                _growTo((void **)&tm->tlid_by_tupid, tm->max_tupid * sizeof(TM_TLID_T), tm->mm, FN_NAME);
                _growTo((void **)&tm->btypid_by_tupid, tm->max_tupid * sizeof(btypeid_t), tm->mm, FN_NAME);
            }
            tm->tlid_by_tupid[tupid] = tlid;
            typelist = tm->typelist_buf + tm->tlrp_by_tlid[tlid];
            numTypes = typelist[0];
            hasT = false;
            for (i = 1; i <= numTypes; i++) {
                sum = tm->btsummary_by_btypeid + typelist[i];
                hasT = hasT | TM_HAS_T(*sum);
            }
            btypeid = _update_type_summary(tm, btypeid, tupid, 0, hasT);
            tm->btsummary_by_btypeid[btypeid] |= bmttup;
            tm->btypid_by_tupid[tupid] = btypeid;
            hi_replace_empty(TM_DETAILID_BY_TLIDHASH, tm->tupid_by_tlidhash, idx, tupid);
            return btypeid;
    }
}

pub btypeid_t tm_tuplev(BK_TM *tm, btypeid_t btypeid, u32 numTypes, ...) {
    // OPEN: implement
    va_list args;  btypeid_t *typelist;  int i;
    va_start(args, numTypes);
    typelist = malloc((1 + numTypes) * sizeof(btypeid_t));      // OPEN: use a typelist buffer of big enough size
    for (i = 1; i <= numTypes; i++) typelist[i] = va_arg(args, btypeid_t);
    typelist[0] = numTypes;
    btypeid = tm_union(tm, btypeid, typelist);
    free(typelist);
    va_end(args);
    return btypeid;
}

pub TM_TLID_T tm_tuple_tlid(BK_TM *tm, btypeid_t btypeid) {
    if (!btypeid || btypeid >= tm->next_btypeId) return 0;
    btsummary sum = tm->btsummary_by_btypeid[btypeid];
    return (TM_BMT_ID(sum) == bmttup) ? tm->tlid_by_tupid[TM_DETAILS_ID(sum)] : 0;
}

#endif  // __BK_TM_TUPLE_C