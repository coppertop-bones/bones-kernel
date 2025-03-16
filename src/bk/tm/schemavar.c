// ---------------------------------------------------------------------------------------------------------------------
// SCHEMA VARIABLE IMPLEMENTATION
// KEEPER REQUISITES: core
// ---------------------------------------------------------------------------------------------------------------------

#ifndef __BK_TM_SCHEMAVAR_C
#define __BK_TM_SCHEMAVAR_C "bk/tm/schemavar.c"


#include "core.c"



pub btypeid_t tm_schemavar(BK_TM *tm, btypeid_t self, char const *name) {
    int outcome;  symid_t symid;  u32 idx;  btsummary sum;  btypeid_t other;

    // answers the validated schema variable corresponding to name, creating if necessary
    if (!self || self >= tm->next_btypeId) return B_NAT;
    if (self == B_NEW) {
        symid = sm_id(tm->sm, name);
        idx = hi_put_idx(TM_BTYPEID_BY_SYMIDHASH, tm->btypeid_by_symidhash, symid, &outcome);
        if (outcome == HI_LIVE) {
            // name already in use so check it's a schema variable
            other = hi_token(tm->btypeid_by_symidhash, idx);
            return (TM_BMT_ID(tm->btsummary_by_btypeid[other]) == bmtsvr) ? other : B_NAT;
        } else {
            self = tm->next_btypeId;
            _update_type_summary(tm, self, 0, 0, true);
        }
    } else {
        if (TM_BMT_ID(sum = tm->btsummary_by_btypeid[self]) == bmterr) {
            symid = sm_id(tm->sm, name);
            idx = hi_put_idx(TM_BTYPEID_BY_SYMIDHASH, tm->btypeid_by_symidhash, symid, &outcome);
            if (outcome == HI_LIVE) {
                other = hi_token(tm->btypeid_by_symidhash, idx);
                if (other != self) return B_NAT; // name in use by another type
            }
        } else {
            // check we are referring to the same schema variable
            if (TM_BMT_ID(sum) != bmtsvr) return B_NAT;
            if (strcmp(name, tm_name(tm, self)) != 0) return B_NAT;
            return self;
        }
    }

    // initialise
    tm->btsummary_by_btypeid[self] |= bmtsvr;
    hi_replace_empty(TM_BTYPEID_BY_SYMIDHASH, tm->btypeid_by_symidhash, idx, self);
    tm->symid_by_btypeid[self] = symid;
    return self;
}

#endif  // __BK_TM_SCHEMAVAR_C