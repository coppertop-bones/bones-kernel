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
// TUPLE IMPLEMENTATION
// KEEPER REQUISITES: core
// ---------------------------------------------------------------------------------------------------------------------

#ifndef __BK_TM_TUPLE_C
#define __BK_TM_TUPLE_C "bk/tm/tuple.c"


#include "core.c"



pub btypeid_t tm_tuple(BK_TM *tm, btypeid_t self, TM_TLID_T tlid) {
    // answers the validated tuple type corresponding to typelist, creating if necessary

    i32 i, outcome, numTypes;  btsummary *sum;  TM_DETAILID_T tupid;  u32 idx;  bool hasT;
    btypeid_t *typelist, other;

    if (!self) return B_NAT;
    if (self >= tm->next_btypeId) return _err_selfOutOfRange(B_NAT, __FILE__, FN_NAME, __LINE__, self);

    // get the btypeid for the tlid
    idx = hi_put_idx(TM_DETAILID_BY_TLIDHASH, tm->tupid_by_tlidhash, tlid, &outcome);
    switch (outcome) {
        default:
            die("%s:%i: HI_TOMBSTONE2!", FN_NAME, __LINE__);
        case HI_LIVE:
            tupid = tm->tupid_by_tlidhash->tokens[idx];
            if (self == B_NEW) return tm->btypid_by_tupid[tupid];
            else if (TM_BMT_ID(tm->btsummary_by_btypeid[self]) != bmttup)
                return _err_btypeAlreadyInitialised(B_NAT, __FILE__, __LINE__, self);
            else if (self == (other = tm->btypid_by_tupid[tupid])) return self;
            else return _otherAlreadyRepresentsTL(B_NAT, __FILE__, __LINE__, self, other);
        case HI_EMPTY:
            // missing so commit the tuple type for tlid
            if (self == B_NEW)
                self = tm->next_btypeId;
            else if (TM_BMT_ID(tm->btsummary_by_btypeid[self]) != bmterr)
                // self is already in use so given the type list lookup above we cannot be referring to the same btype
                return _err_btypeAlreadyInitialised(B_NAT, __FILE__, __LINE__, self);
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
            _update_type_summary(tm, self, tupid, 0, hasT);
            tm->btsummary_by_btypeid[self] |= bmttup;
            tm->btypid_by_tupid[tupid] = self;
            hi_replace_empty(TM_DETAILID_BY_TLIDHASH, tm->tupid_by_tlidhash, idx, tupid);
            return self;
    }
}

pub btypeid_t tm_tuplev(BK_TM *tm, btypeid_t self, u32 numTypes, ...) {
    // OPEN: implement
    va_list args;  btypeid_t *typelist;  int i;
    va_start(args, numTypes);
    typelist = malloc((1 + numTypes) * sizeof(btypeid_t));      // OPEN: use a typelist buffer of big enough size
    for (i = 1; i <= numTypes; i++) typelist[i] = va_arg(args, btypeid_t);
    typelist[0] = numTypes;
    self = tm_union(tm, self, typelist);
    free(typelist);
    va_end(args);
    return self;
}

pub btypeid_t * tm_tuple_tl(BK_TM *tm, btypeid_t self) {
    // answer a typelist ptr to the given tuple's types or 0 for error
    btsummary *sum;
    // OPEN: do we bounds check self here?
    sum = tm->btsummary_by_btypeid + self;
    if (TM_BMT_ID(*sum) == bmttup) {
        return tm->typelist_buf + tm->tlrp_by_tlid[tm->tlid_by_tupid[TM_DETAILS_ID(*sum)]];
    } else {
        return 0;
    }
}

#endif  // __BK_TM_TUPLE_C