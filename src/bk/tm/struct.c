// ---------------------------------------------------------------------------------------------------------------------
// STRUCT IMPLEMENTATION
// KEEPER REQUISITES: core
// ---------------------------------------------------------------------------------------------------------------------

#ifndef __BK_TM_STRUCT_C
#define __BK_TM_STRUCT_C "bk/tm/struct.c"


#include "core.c"



pub btypeid_t tm_struct(BK_TM *tm, btypeid_t self, SM_SLID_T slid, TM_TLID_T tlid) {
    i32 outcome;  TM_DETAILID_T strid;  TM_SLID_TLID slid_tlid;  u32 idx;

    // check each typeid is valid
    if (!self || self >= tm->next_btypeId) return B_NAT;

    slid_tlid.slid = slid;
    slid_tlid.tlid = tlid;

    // get the btypeid for the t1t2
    idx = hi_put_idx(TM_DETAILID_BY_SLIDTUPIDHASH, tm->strid_by_slidtlidhash, slid_tlid, &outcome);
    switch (outcome) {
        default:
            die("%s:%i: HI_TOMBSTONE!", FN_NAME, __LINE__);
        case HI_LIVE:
            strid = tm->strid_by_slidtlidhash->tokens[idx];
            if (self == B_NEW) return tm->btypid_by_strid[strid];
            else if (self == tm->btypid_by_strid[strid]) return self;
            else return B_NAT;
        case HI_EMPTY:
            // missing so commit the function type for t1t2
            if (self == B_NEW)
                self = tm->next_btypeId;
            else if (TM_BMT_ID(tm->btsummary_by_btypeid[self]) != bmterr)
                // self is already in use so given the t1t2 lookup above we cannot be referring to the same btype
                return B_NAT;
            strid = tm->next_strid++;
            if (strid >= tm->max_strid) {
                tm->max_strid += TM_MAX_ID_INC_SIZE;
                _growTo((void **)&tm->tlid_by_strid, tm->max_strid * sizeof(TM_T1T2), tm->mm, FN_NAME);
                _growTo((void **)&tm->slid_by_strid, tm->max_strid * sizeof(TM_T1T2), tm->mm, FN_NAME);
                _growTo((void **)&tm->btypid_by_strid, tm->max_strid * sizeof(btypeid_t), tm->mm, FN_NAME);
            }
            tm->slid_by_strid[strid] = slid_tlid.slid;
            tm->tlid_by_strid[strid] = slid_tlid.tlid;
            _update_type_summary(tm, self, strid, 0, TM_HAS_T(tm->btsummary_by_btypeid[strid]));
            tm->btsummary_by_btypeid[self] |= bmtstr;
            tm->btypid_by_strid[strid] = self;
            hi_replace_empty(TM_DETAILID_BY_SLIDTUPIDHASH, tm->strid_by_slidtlidhash, idx, strid);
            return self;
    }
}

pub symid_t * tm_struct_sl(BK_TM *tm, btypeid_t self) {
    // answer a symlist ptr to the given struct's field names or 0 for error
    return 0;           // OPEN: implement
}

pub btypeid_t * tm_struct_tl(BK_TM *tm, btypeid_t self) {
    // answer a typelist ptr to the given struct's field types or 0 for error
    return 0;           // OPEN: implement
}

#endif  // __BK_TM_STRUCT_C