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



pub btypeid_t tm_schemavar(BK_TM *tm, btypeid_t btype) {
    // initialises btype as a schema variable (allocating if necessary) and returns btype or B_NAT if already initialized
    if (!btype) return _err_invalid_btype_B_NAT(B_NAT, __FILE__, FN_NAME, __LINE__);
    else if (btype >= tm->next_btypeId) return _err_selfOutOfRange(B_NAT, __FILE__, FN_NAME, __LINE__, btype);
    else if (btype == B_NEW) btype = tm->next_btypeId;
    else if (TM_BMT_ID(tm->btsummary_by_btypeid[btype]) != bmterr) return B_NAT;
    _update_type_summary(tm, btype, bmtsvr, 0, true);
    return btype;
}

#endif  // __BK_TM_SCHEMAVAR_C