// ---------------------------------------------------------------------------------------------------------------------
// Copyright 2025 David Briant, https://github.com/coppertop-bones. Licensed under the Apache License, Version 2.0
//
// STRUCT IMPLEMENTATION
// KEEPER REQUISITES: core
// ---------------------------------------------------------------------------------------------------------------------

#ifndef __BK_TM_STRUCT_C
#define __BK_TM_STRUCT_C "bk/tm/struct.c"


#include "core.c"



pub btypeid_t tm_struct(BK_TM *tm, btypeid_t btypeid, SM_SLID_T slid, TM_TLID_T tlid) {
    i32 i, outcome, numTypes;  btsummary *sum;  TM_DETAILID_T strid;  TM_SLID_TLID slid_tlid;  u32 idx;  bool hasT;
    btypeid_t *typelist;  symid_t *symlist;

    if (!btypeid) return _err_invalid_btype_B_NAT(B_NAT, __FILE__, FN_NAME, __LINE__);
    if (btypeid >= tm->next_btypeId) return _err_btypeidOutOfRange(B_NAT, __FILE__, FN_NAME, __LINE__, btypeid);

    slid_tlid.slid = slid;
    slid_tlid.tlid = tlid;

    // get the btypeid for the slid_tlid
    idx = hi_put_idx(TM_DETAILID_BY_SLIDTLIDHASH, tm->strid_by_slidtlidhash, slid_tlid, &outcome);
    switch (outcome) {
        default:
            die("%s:%i: HI_TOMBSTONE!", FN_NAME, __LINE__);
        case HI_LIVE:
            strid = tm->strid_by_slidtlidhash->tokens[idx];
            if (btypeid == B_NEW) return tm->btypid_by_strid[strid];
            else if (btypeid == tm->btypid_by_strid[strid]) return btypeid;
            else {
                PP(info, "%s:%i slid=%i, tlid=%i is already defined by btypeid=%i", FN_NAME, __LINE__, slid, tlid, btypeid);
                return B_NAT;
            }
        case HI_EMPTY:
            // missing so commit the struct type for slid_tlid
            if (btypeid == B_NEW)
                btypeid = tm->next_btypeId;
            else if (TM_BMT_ID(tm->btsummary_by_btypeid[btypeid]) != bmterr)
                // btypeid is already in use so given the slid_tlid lookup above we cannot be referring to the same btype
                return _err_btypeAlreadyInitialised(B_NAT, __FILE__, __LINE__, btypeid);
            // PP(info, "%s:%i defining slid=%i, tlid=%i, btypeid=%i", FN_NAME, __LINE__, slid, tlid, btypeid);

            typelist = tm->typelist_buf + tm->tlrp_by_tlid[tlid];
            symlist =  tm->sm->symlist_buf + tm->sm->slrp_by_slid[slid];
            if ((numTypes = typelist[0]) != symlist[0] || numTypes == 0) {
                PP(info, "%s:%i slid=%i, len=%i, tlid=%i, len=%i is problematic", FN_NAME, __LINE__, slid, symlist[0], tlid, typelist[0]);
                return B_NAT;
            }

            strid = tm->next_strid++;
            if (strid >= tm->max_strid) {
                tm->max_strid += TM_MAX_ID_INC_SIZE;
                _growTo((void **)&tm->tlid_by_strid, tm->max_strid * sizeof(TM_T1T2), tm->mm, FN_NAME);
                _growTo((void **)&tm->slid_by_strid, tm->max_strid * sizeof(TM_T1T2), tm->mm, FN_NAME);
                _growTo((void **)&tm->btypid_by_strid, tm->max_strid * sizeof(btypeid_t), tm->mm, FN_NAME);
            }

            hasT = false;
            for (i = 1; i <= numTypes; i++) {
                sum = tm->btsummary_by_btypeid + typelist[i];
                hasT = hasT | TM_HAS_T(*sum);
            }

            tm->slid_by_strid[strid] = slid_tlid.slid;
            tm->tlid_by_strid[strid] = slid_tlid.tlid;
            btypeid = _update_type_summary(tm, btypeid, strid, 0, hasT);
            tm->btsummary_by_btypeid[btypeid] |= bmtstr;
            tm->btypid_by_strid[strid] = btypeid;
            hi_replace_empty(TM_DETAILID_BY_SLIDTLIDHASH, tm->strid_by_slidtlidhash, idx, strid);
            return btypeid;
    }
}

pub SM_SLID_T tm_struct_slid(BK_TM *tm, btypeid_t btypeid) {
    if (!btypeid || btypeid >= tm->next_btypeId) return 0;
    btsummary sum = tm->btsummary_by_btypeid[btypeid];
    return (TM_BMT_ID(sum) == bmtstr) ? tm->slid_by_strid[TM_DETAILS_ID(sum)] : 0;
}

pub TM_TLID_T tm_struct_tlid(BK_TM *tm, btypeid_t btypeid) {
    if (!btypeid || btypeid >= tm->next_btypeId) return 0;
    btsummary sum = tm->btsummary_by_btypeid[btypeid];
    return (TM_BMT_ID(sum) == bmtstr) ? tm->tlid_by_strid[TM_DETAILS_ID(sum)] : 0;
}

#endif  // __BK_TM_STRUCT_C