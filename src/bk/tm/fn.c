// ---------------------------------------------------------------------------------------------------------------------
// FUNCTION IMPLEMENTATION
// KEEPER REQUISITES: core
// ---------------------------------------------------------------------------------------------------------------------

#ifndef __BK_TM_FN_C
#define __BK_TM_FN_C "bk/tm/fn.c"


#include "core.c"



pub btypeid_t tm_fn(BK_TM *tm, btypeid_t self, btypeid_t tArgs, btypeid_t retid) {
    i32 outcome;  TM_DETAILID_T fncid;  TM_T1T2 t1t2;  u32 idx;  bool hasT;

    // answers the validated function type corresponding to tArgs and retid, creating if necessary

    // check each typeid is valid
    if (!self) return B_NAT;
    if (self >= tm->next_btypeId) return B_NAT;
    if (!(TM_FIRST_VALID_BTYPEID <= tArgs && tArgs < tm->next_btypeId)) return B_NAT;
    if (TM_BMT_ID(tm->btsummary_by_btypeid[tArgs]) != bmttup) return B_NAT;
    if (!(TM_FIRST_VALID_BTYPEID <= retid && retid < tm->next_btypeId)) return B_NAT;
    if (TM_BMT_ID(tm->btsummary_by_btypeid[retid]) == bmterr) return B_NAT;

    t1t2.tArgs = tArgs;
    t1t2.tRet = retid;

    // get the btypeid for the t1t2
    idx = hi_put_idx(TM_DETAILID_BY_T1T2HASH, tm->fncid_by_t1t2hash, t1t2, &outcome);
    switch (outcome) {
        default:
            die("%s:%i: HI_TOMBSTONE2!", FN_NAME, __LINE__);
        case HI_LIVE:
            fncid = tm->fncid_by_t1t2hash->tokens[idx];
            if (self == B_NEW) return tm->btypid_by_fncid[fncid];
            else if (self == tm->btypid_by_fncid[fncid]) return self;
            else return B_NAT;
        case HI_EMPTY:
            // missing so commit the function type for t1t2
            if (self == B_NEW) {
                self = tm->next_btypeId;
            } else if (TM_BMT_ID(tm->btsummary_by_btypeid[self]) != bmterr)
                // self is already in use so given the t1t2 lookup above we cannot be referring to the same btype
                return B_NAT;
            fncid = tm->next_fncid++;
            if (fncid >= tm->max_fncid) {
                tm->max_fncid += TM_MAX_ID_INC_SIZE;
                _growTo((void **)&tm->t1t2_by_fncid, tm->max_fncid * sizeof(TM_T1T2), tm->mm, FN_NAME);
                _growTo((void **)&tm->btypid_by_fncid, tm->max_fncid * sizeof(btypeid_t), tm->mm, FN_NAME);
            }
            tm->t1t2_by_fncid[fncid] = t1t2;
            hasT = TM_HAS_T(tm->btsummary_by_btypeid[tArgs]) || TM_HAS_T(tm->btsummary_by_btypeid[retid]);
            _update_type_summary(tm, self, fncid, 0, hasT);
            tm->btsummary_by_btypeid[self] |= bmtfnc;
            tm->btypid_by_fncid[fncid] = self;
            hi_replace_empty(TM_DETAILID_BY_T1T2HASH, tm->fncid_by_t1t2hash, idx, fncid);
            return self;
    }
}

pub TM_T1T2 tm_fn_targs_tret(BK_TM *tm, btypeid_t self) {
    if (!(TM_FIRST_VALID_BTYPEID <= self && self < tm->next_btypeId)) return (TM_T1T2) {{0}, {0}};
    btsummary *sum = tm->btsummary_by_btypeid + self;       // OPEN: in general use pointer to summary rather than copying the struct
    if (TM_BMT_ID(*sum) != bmtfnc) return (TM_T1T2) {{0}, {0}};
    return tm->t1t2_by_fncid[TM_DETAILS_ID(*sum)];
}

#endif  // __BK_TM_FN_C