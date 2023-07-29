// ---------------------------------------------------------------------------------------------------------------------
// Copyright 2025 David Briant, https://github.com/coppertop-bones. Licensed under the Apache License, Version 2.0
//
// MAP IMPLEMENTATION
// KEEPER REQUISITES: core
// ---------------------------------------------------------------------------------------------------------------------

#ifndef __BK_TM_MAP_C
#define __BK_TM_MAP_C "bk/tm/map.c"


#include "core.c"



pub btypeid_t tm_map(BK_TM *tm, btypeid_t self, btypeid_t tK, btypeid_t tV) {
    i32 outcome;  TM_DETAILID_T mapid;  TM_T1T2 t1t2;  u32 idx;  bool hasT;

    // answers the validated map type corresponding to tK and tV, creating if necessary

    // check each typeid is valid
    if (!self || self >= tm->next_btypeId) return B_NAT;
    if (!(TM_FIRST_VALID_BTYPEID <= tK && tK < tm->next_btypeId)) return B_NAT;
    if (TM_BMT_ID(*(tm->btsummary_by_btypeid + tK)) == bmterr) return B_NAT;
    if (!(TM_FIRST_VALID_BTYPEID <= tV && tV < tm->next_btypeId)) return B_NAT;
    if (TM_BMT_ID(*(tm->btsummary_by_btypeid + tV)) == bmterr) return B_NAT;

    t1t2.tK = tK;
    t1t2.tV = tV;

    // get the btypeid for the t1t2
    idx = hi_put_idx(TM_DETAILID_BY_T1T2HASH, tm->mapid_by_t1t2hash, t1t2, &outcome);
    switch (outcome) {
        default:
            die("%s:%i: HI_TOMBSTONE2!", FN_NAME, __LINE__);
        case HI_LIVE:
            mapid = tm->mapid_by_t1t2hash->tokens[idx];
            if (self == B_NEW) return tm->btypid_by_mapid[mapid];
            else if (self == tm->btypid_by_mapid[mapid]) return self;
            else return B_NAT;
        case HI_EMPTY:
            // missing so commit the function type for t1t2
            if (self == B_NEW) {
                self = tm->next_btypeId;
            } else if (TM_BMT_ID(tm->btsummary_by_btypeid[self]) != bmterr)
                // self is already in use so given the t1t2 lookup above we cannot be referring to the same btype
                return B_NAT;
            mapid = tm->next_mapid++;
            if (mapid >= tm->max_mapid) {
                tm->max_mapid += TM_MAX_ID_INC_SIZE;
                _growTo((void **)&tm->t1t2_by_mapid, tm->max_mapid * sizeof(TM_T1T2), tm->mm, FN_NAME);
                _growTo((void **)&tm->btypid_by_mapid, tm->max_mapid * sizeof(btypeid_t), tm->mm, FN_NAME);
            }
            tm->t1t2_by_mapid[mapid] = t1t2;
            hasT = TM_HAS_T(tm->btsummary_by_btypeid[tK]) || TM_HAS_T(tm->btsummary_by_btypeid[tV]);
            self = _update_type_summary(tm, self, mapid, 0, hasT);
            tm->btsummary_by_btypeid[self] |= bmtmap;
            tm->btypid_by_mapid[mapid] = self;
            hi_replace_empty(TM_DETAILID_BY_T1T2HASH, tm->mapid_by_t1t2hash, idx, mapid);
            return self;
    }
}

pub TM_T1T2 tm_map_tk_tv(BK_TM *tm, btypeid_t self) {
    // answer ...
    if (!(TM_FIRST_VALID_BTYPEID <= self && self < tm->next_btypeId)) return (TM_T1T2) {{0}, {0}};
    btsummary *sum = tm->btsummary_by_btypeid + self;       // OPEN: in general use pointer to summary rather than copying the struct
    if (TM_BMT_ID(*sum) != bmtmap) return (TM_T1T2) {{0}, {0}};
    return tm->t1t2_by_mapid[TM_DETAILS_ID(*sum)];
}

#endif  // __BK_TM_MAP_C