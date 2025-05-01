// ---------------------------------------------------------------------------------------------------------------------
//
//                             Copyright (c) 2019-2025 David Briant. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance
// with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software distributed under the License is distributed
// on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for
// the specific language governing permissions and limitations under the License.
//
// ---------------------------------------------------------------------------------------------------------------------


// ---------------------------------------------------------------------------------------------------------------------
// ATOM IMPLEMENTATION
// KEEPER REQUISITES: core
// ---------------------------------------------------------------------------------------------------------------------

#ifndef __BK_TM_ATOM_C
#define __BK_TM_ATOM_C "bk/tm/atom.c"


#include "core.c"


pub btypeid_t tm_init_atom(BK_TM *tm, btypeid_t self, btypeid_t implicitid, bool explicit) {
    // answers a validated atom type creating if necessary
    if (!self || self >= tm->next_btypeId) return B_NAT;
    if (self == B_NEW) {
//        symid = sm_id(tm->sm, name);
//        idx = hi_put_idx(TM_BTYPEID_BY_SYMIDHASH, tm->btypeid_by_symidhash, symid, &outcome);
//        if (outcome == HI_LIVE) {
//            // name already in use so check it's an atom
//            other = hi_token(tm->btypeid_by_symidhash, idx);
//            return (TM_BMT_ID(tm->btsummary_by_btypeid[other]) == bmtatm) ? other : B_NAT;
//        } else {
            self = tm->next_btypeId;
        _update_type_summary(tm, self, 0, 0, 0);
    } else {
        // check we are referring to an uninitialised atom
        if (TM_BMT_ID(tm->btsummary_by_btypeid[self]) != bmterr) return B_NAT;
    }

//        if (TM_BMT_ID(sum = tm->btsummary_by_btypeid[self]) == bmterr) {
//            // we are finalising a preallocated type here - this is a construction operation that cannot be interpreted
//            // as a retrieve operation - if we discover we already have defined a type with the same attributes but
//            // different btypeid return B_NAT. This makes preallocated types a bit more fiddly to use.
//            symid = sm_id(tm->sm, name);
//            idx = hi_put_idx(TM_BTYPEID_BY_SYMIDHASH, tm->btypeid_by_symidhash, symid, &outcome);
//            if (outcome == HI_LIVE) {
//                // name in use by another type - check to see if I am the same as the other type
//                other = hi_token(tm->btypeid_by_symidhash, idx);
//                if (other != self) {
//                    otherSum = tm->btsummary_by_btypeid[other];
//                    if (TM_BMT_ID(otherSum) != bmtatm)
//                        // trying to create an atom with a name that is already in use by a non-atom
//                        return B_NAT;
//                    tm->btsummary_by_btypeid[self] |= bmtatm;
//                    if (sum == otherSum) {
//                        // trying to recreate the same btype but with a different btypeid
//                        return B_NAT;
//                    } else {
//                        // trying to create a btype with different attributes but the same name
//                        return B_NAT;
//                    }
//                }
//            }
//        } else {

    // initialise
    tm->btsummary_by_btypeid[self] |= bmtatm | (explicit ? TM_IS_EXPLICIT_MASK : 0);
//    hi_replace_empty(TM_BTYPEID_BY_SYMIDHASH, tm->btypeid_by_symidhash, idx, self);
    if (implicitid) tm->implicitid_by_spaceid[self] = implicitid;
    return self;
}

pub btypeid_t tm_check_atom(BK_TM *tm, btypeid_t self, btypeid_t implicitid, bool explicit, btypeid_t spaceid) {
    // An existing atom is being defined a second time check that the attributes don't conflict
    //      a) if current.space is already set then space may be the same or missing
    //      b) if current.implicitly is already set then implicitly may be the same or missing
    //      c) if current.explicit is true then explicit may be True or missing

    return self;
}


#endif  // __BK_TM_ATOM_C