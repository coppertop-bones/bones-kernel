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
// SEQUENCE IMPLEMENTATION
// KEEPER REQUISITES: core
// ---------------------------------------------------------------------------------------------------------------------

#ifndef __BK_TM_SEQ_C
#define __BK_TM_SEQ_C "bk/tm/seq.c"


#include "core.c"



pub btypeid_t tm_seq(BK_TM *tm, btypeid_t self, btypeid_t containedid) {
    i32 outcome;  btsummary *sum;  btypeid_t containerid;  u32 idx;

    // answers the validated sequence type corresponding to tContained, creating if necessary
    if (!self || self >= tm->next_btypeId) return B_NAT;

    // check that containedid is valid
    if (!(TM_FIRST_VALID_BTYPEID <= containedid && containedid < tm->next_btypeId)) return B_NAT;
    sum = tm->btsummary_by_btypeid + containedid;
    if (!TM_IS_RECURSIVE(*sum) && TM_BMT_ID(*sum) == bmterr) return B_NAT;

    // get the btypeid for the tContained
    idx = hi_put_idx(TM_BTYPID_BY_SEQIDHASH, tm->containerid_by_containedidhash, containedid, &outcome);
    switch (outcome) {
        default:
            die("%s:%i: HI_TOMBSTONE2!", FN_NAME, __LINE__);
        case HI_LIVE:
            containerid = tm->containerid_by_containedidhash->tokens[idx];
            if (self == B_NEW) return containerid;
            else if (self == containerid) return self;
            else return B_NAT;
        case HI_EMPTY:
            if (self == B_NEW)
                self = tm->next_btypeId;
            else if (TM_BMT_ID(tm->btsummary_by_btypeid[self]) != bmterr)
                // self is already in use so given the type list lookup above we cannot be referring to the same btype
                return B_NAT;
            self = _update_type_summary(tm, self, containedid, 0, TM_HAS_T(*sum));
            tm->btsummary_by_btypeid[self] |= bmtseq;
            hi_replace_empty(TM_BTYPID_BY_SEQIDHASH, tm->containerid_by_containedidhash, idx, self);
            return self;
    }
}

pub btypeid_t tm_seq_t(BK_TM *tm, btypeid_t self) {
    btsummary *sum;
    if (!(TM_FIRST_VALID_BTYPEID <= self && self < tm->next_btypeId)) return B_NAT;
    sum = tm->btsummary_by_btypeid + self;
    return TM_BMT_ID(*sum) == bmtseq ? TM_DETAILS_ID(*sum) : B_NAT;
}

#endif  // __BK_TM_SEQ_C