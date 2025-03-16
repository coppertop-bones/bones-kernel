// ---------------------------------------------------------------------------------------------------------------------
// ATOM IMPLEMENTATION
// KEEPER REQUISITES: core
// ---------------------------------------------------------------------------------------------------------------------

#ifndef __BK_TM_ATOM_C
#define __BK_TM_ATOM_C "bk/tm/atom.c"


#include "core.c"



pub btypeid_t tm_atom(BK_TM *tm, btypeid_t self, char const *name) {
    int outcome;  symid_t symid;  u32 idx;  btsummary sum, otherSum;  btypeid_t other;

    // answers the validated atom type corresponding to name, creating if necessary
    if (!self || self >= tm->next_btypeId) return B_NAT;
    if (self == B_NEW) {
        symid = sm_id(tm->sm, name);
        idx = hi_put_idx(TM_BTYPEID_BY_SYMIDHASH, tm->btypeid_by_symidhash, symid, &outcome);
        if (outcome == HI_LIVE) {
            // name already in use so check it's an atom
            other = hi_token(tm->btypeid_by_symidhash, idx);
            return (TM_BMT_ID(tm->btsummary_by_btypeid[other]) == bmtatm) ? other : B_NAT;
        } else {
            self = tm->next_btypeId;
            _update_type_summary(tm, self, 0, 0, 0);
        }
    } else {
        if (TM_BMT_ID(sum = tm->btsummary_by_btypeid[self]) == bmterr) {
            // we are finalising a preallocated type here - this is a construction operation that cannot be interpreted
            // as a retrieve operation - if we discover we already have defined a type with the same attributes but
            // different btypeid return B_NAT. This makes preallocated types a bit more fiddly to use.
            symid = sm_id(tm->sm, name);
            idx = hi_put_idx(TM_BTYPEID_BY_SYMIDHASH, tm->btypeid_by_symidhash, symid, &outcome);
            if (outcome == HI_LIVE) {
                // name in use by another type - check to see if I am the the same as the other type
                other = hi_token(tm->btypeid_by_symidhash, idx);
                if (other != self) {
                    otherSum = tm->btsummary_by_btypeid[other];
                    if (TM_BMT_ID(otherSum) != bmtatm)
                        // trying to create an atom with a name that is already in use by a non-atom
                        return B_NAT;
                    tm->btsummary_by_btypeid[self] |= bmtatm;
                    if (sum == otherSum) {
                        // trying to recreate the same btype but with a different btypeid
                        return B_NAT;
                    } else {
                        // trying to create a btype with different attributes but the same name
                        return B_NAT;
                    }
                }
            }
        } else {
            // check we are referring to the same atom
            if (TM_BMT_ID(sum) != bmtatm) return B_NAT;
            if (strcmp(name, tm_name(tm, self)) != 0) return B_NAT;
            return self;
        }
    }

    // initialise
    tm->btsummary_by_btypeid[self] |= bmtatm;
    hi_replace_empty(TM_BTYPEID_BY_SYMIDHASH, tm->btypeid_by_symidhash, idx, self);
    tm->symid_by_btypeid[self] = symid;
    return self;
}

#endif  // __BK_TM_ATOM_C