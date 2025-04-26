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
// FITSWITHIN IMPLEMENTATION
// KEEPER REQUISITES: core
// ---------------------------------------------------------------------------------------------------------------------

#ifndef __BK_TM_FITSWITHIN_C
#define __BK_TM_FITSWITHIN_C "bk/tm/fitswithin.c"


#include "core.c"



int tm_fitsWithin(BK_TM *tm, btypeid_t a, btypeid_t b) {
    // should answer a tuple {cacheID, doesFit, tByT, distance}
    // tByT can just be a T sorted list (not worth doing a hash)
    if (a == b) return 1;
    if ((b == B_EXTERN) && (a & B_EXTERN)) return 1;
    if ((b == B_FN) && ((a & 0x7f) == B_FN)) return 1;
    if ((b == B_EXTERN_FN_PTR) && ((a & 0xffff) == B_EXTERN_FN_PTR)) return 1;
    if ((b == B_FN_PTR) && ((a & 0xff7f) == B_FN_PTR)) return 1;
    if ((b == B_CHAR_STAR) && ((a & 0xff7f) == B_CHAR_STAR)) return 1;
    if ((b == B_VOID_STAR) && ((a & 0xff7f) == B_VOID_STAR)) return 1;
    if ((a & 0x000000FF) == b) return 1;
    return 0;
}

#endif  // __BK_TM_FITSWITHIN_C