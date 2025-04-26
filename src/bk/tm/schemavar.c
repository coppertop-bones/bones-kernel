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
// SCHEMA VARIABLE IMPLEMENTATION
// KEEPER REQUISITES: core
// ---------------------------------------------------------------------------------------------------------------------

#ifndef __BK_TM_SCHEMAVAR_C
#define __BK_TM_SCHEMAVAR_C "bk/tm/schemavar.c"


#include "core.c"



pub btypeid_t tm_schemavar(BK_TM *tm, btypeid_t self) {

    // answers a validated schema variable creating if necessary
    if (!self || self >= tm->next_btypeId) return B_NAT;
    if (self == B_NEW) {
//        symid = sm_id(tm->sm, name);
//        idx = hi_put_idx(TM_BTYPEID_BY_SYMIDHASH, tm->btypeid_by_symidhash, symid, &outcome);
//        if (outcome == HI_LIVE) {
//            // name already in use so check it's a schema variable
//            other = hi_token(tm->btypeid_by_symidhash, idx);
//            return (TM_BMT_ID(tm->btsummary_by_btypeid[other]) == bmtsvr) ? other : B_NAT;
//        } else {
            self = tm->next_btypeId;
            _update_type_summary(tm, self, 0, 0, true);
            tm->btsummary_by_btypeid[self] |= bmtsvr;
//        }
    } else {
//        if (TM_BMT_ID(sum = tm->btsummary_by_btypeid[self]) == bmterr) {
//            symid = sm_id(tm->sm, name);
//            idx = hi_put_idx(TM_BTYPEID_BY_SYMIDHASH, tm->btypeid_by_symidhash, symid, &outcome);
//            if (outcome == HI_LIVE) {
//                other = hi_token(tm->btypeid_by_symidhash, idx);
//                if (other != self) return B_NAT; // name in use by another type
//            }
//        } else {
//            // check we are referring to the same schema variable
            if (TM_BMT_ID(tm->btsummary_by_btypeid[self]) != bmterr) return B_NAT;
//            if (strcmp(name, tm_name_of(tm, self)) != 0) return B_NAT;
//            return self;
//        }
    }
    return self;
}

#endif  // __BK_TM_SCHEMAVAR_C