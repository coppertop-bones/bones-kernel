// ---------------------------------------------------------------------------------------------------------------------
// Copyright 2025 David Briant, https://github.com/coppertop-bones. Licensed under the Apache License, Version 2.0
//
// FUNCTION IMPLEMENTATION
// KEEPER REQUISITES: core
// ---------------------------------------------------------------------------------------------------------------------

#ifndef __BK_TM_FN_C
#define __BK_TM_FN_C "bk/tm/fn.c"


#include "core.c"



pub btypeid_t tm_fn(BK_TM *tm, btypeid_t btypeid, btypeid_t argsid, btypeid_t retid) {
    i32 outcome;  TM_DETAILID_T fncid;  TM_T1T2 t1t2;  u32 idx;  bool hasT;

    // answers the validated function type corresponding to argsid and retid, creating if necessary

    // check each btypeid is valid
    if (!btypeid) return _err_invalid_btype_B_NAT(B_NAT, __FILE__, FN_NAME, __LINE__);
    if (btypeid >= tm->next_btypeId) return _err_btypeidOutOfRange(B_NAT, __FILE__, FN_NAME, __LINE__, btypeid);

    if (!(TM_FIRST_VALID_BTYPEID <= argsid && argsid < tm->next_btypeId)) return B_NAT;
    if (TM_BMT_ID(tm->btsummary_by_btypeid[argsid]) != bmttup) return B_NAT;

    if (!(TM_FIRST_VALID_BTYPEID <= retid && retid < tm->next_btypeId)) return B_NAT;
    if (TM_BMT_ID(tm->btsummary_by_btypeid[retid]) == bmterr) return B_NAT;

    t1t2.tArgs = argsid;
    t1t2.tRet = retid;

    // get the btypeid for the t1t2
    idx = hi_put_idx(TM_DETAILID_BY_T1T2HASH, tm->fncid_by_t1t2hash, t1t2, &outcome);
    switch (outcome) {
        default:
            die("%s:%i: HI_TOMBSTONE2!", FN_NAME, __LINE__);
        case HI_LIVE:
            fncid = tm->fncid_by_t1t2hash->tokens[idx];
            if (btypeid == B_NEW) return tm->btypid_by_fncid[fncid];
            else if (btypeid == tm->btypid_by_fncid[fncid]) return btypeid;
            else return B_NAT;
        case HI_EMPTY:
            // missing so commit the function type for t1t2
            if (btypeid == B_NEW) {
                btypeid = tm->next_btypeId;
            } else if (TM_BMT_ID(tm->btsummary_by_btypeid[btypeid]) != bmterr)
                // btypeid is already in use so given the t1t2 lookup above we cannot be referring to the same btype
                return B_NAT;
            fncid = tm->next_fncid++;
            if (fncid >= tm->max_fncid) {
                tm->max_fncid += TM_MAX_ID_INC_SIZE;
                _growTo((void **)&tm->t1t2_by_fncid, tm->max_fncid * sizeof(TM_T1T2), tm->mm, FN_NAME);
                _growTo((void **)&tm->btypid_by_fncid, tm->max_fncid * sizeof(btypeid_t), tm->mm, FN_NAME);
            }
            tm->t1t2_by_fncid[fncid] = t1t2;
            hasT = TM_HAS_T(tm->btsummary_by_btypeid[argsid]) || TM_HAS_T(tm->btsummary_by_btypeid[retid]);
            btypeid = _update_type_summary(tm, btypeid, fncid, 0, hasT);
            tm->btsummary_by_btypeid[btypeid] |= bmtfnc;
            tm->btypid_by_fncid[fncid] = btypeid;
            hi_replace_empty(TM_DETAILID_BY_T1T2HASH, tm->fncid_by_t1t2hash, idx, fncid);
            return btypeid;
    }
}

pub TM_T1T2 tm_fn_targs_tret(BK_TM *tm, btypeid_t btypeid) {
    if (!(TM_FIRST_VALID_BTYPEID <= btypeid && btypeid < tm->next_btypeId)) return (TM_T1T2) {{0}, {0}};
    btsummary *sum = tm->btsummary_by_btypeid + btypeid;
    if (TM_BMT_ID(*sum) != bmtfnc) return (TM_T1T2) {{0}, {0}};
    return tm->t1t2_by_fncid[TM_DETAILS_ID(*sum)];
}

#endif  // __BK_TM_FN_C