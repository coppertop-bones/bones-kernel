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
// EM - ENUM MANAGER
// ---------------------------------------------------------------------------------------------------------------------


#ifndef SRC_BK_EM_C
#define SRC_BK_EM_C "bk/em.c"

#include "../../include/bk/mm.h"
#include "../../include/bk/em.h"

pub BK_EM *EM_create(BK_MM *mm, BK_SM *sm) {
    BK_EM *em = (BK_EM*) mm->malloc(sizeof(BK_EM));
    em->mm = mm;
    em->sm = sm;
    return em;
}

pub int EM_trash(BK_EM *em) {
    em->mm->free(em);
    return 0;
}


#endif // SRC_BK_EM_C
