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
// K - KERNEL
// ---------------------------------------------------------------------------------------------------------------------

#ifndef __BK_K_C
#define __BK_K_C "bk/k.c"

#include "../../include/bk/k.h"
#include "tp.c"
#include "sm.c"
#include "em.c"
#include "tm.c"

pub BK_K * K_create(BK_MM *mm, Buckets *buckets) {
    BK_K *k = mm->malloc(sizeof(BK_K));
    k->mm = mm;
    k->buckets = buckets;
    k->sm = SM_create(mm);
    k->em = EM_create(mm, k->sm);
    TP_init(&(k->tp), 0, buckets);
    k->tm = TM_create(mm, k->buckets, k->sm, &(k->tp));

    int n = 0;
    BK_TM *tm = k->tm;
    tm_reserve_btypeids(tm, B_FIRST_UNRESERVED_TYPEID);
//    n += tm_bind("m8", tm_init_atom(tm, B_M8, 0)) == 0;
//    n += tm_init_atom(tm, B_M16, "m16") == 0;
//    n += tm_init_atom(tm, B_M32, "m32") == 0;
//    n += tm_init_atom(tm, B_M64, "m64") == 0;
//    n += tm_init_atom(tm, B_LITINT, "litint") == 0;
//    n += tm_init_atom(tm, B_I32, "i32") == 0;
//
//    n += tm_bind("T", tm_schemavar(tm, B_T)) == 0;
//    n += tm_bind("T1", tm_schemavar(tm, B_T1)) == 0;
//    n += tm_bind("T2", tm_schemavar(tm, B_T2)) == 0;
//    n += tm_bind("T3", tm_schemavar(tm, B_T3)) == 0;
//
//    n += tm_bind("N, tm_schemavar(tm, B_N)) == 0;
//    n += tm_bind("N1", tm_schemavar(tm, B_N1)) == 0;
//    n += tm_bind("N2", tm_schemavar(tm, B_N2)) == 0;
//    n += tm_bind("N3", tm_schemavar(tm, B_N3)) == 0;
//    n += tm_bind("N4", tm_schemavar(tm, B_N4)) == 0;
//    n += tm_bind("N5", tm_schemavar(tm, B_N5)) == 0;
//    n += tm_bind("N6", tm_schemavar(tm, B_N6)) == 0;
//    n += tm_bind("N7", tm_schemavar(tm, B_N7)) == 0;
//    n += tm_bind("N8", tm_schemavar(tm, B_N8)) == 0;
//    n += tm_bind("N9", m_schemavar(tm, B_N9)) == 0;

    if (n) {
        mm->free(tm);
        die("%i conflicts in tm_init_atom\n", n);
    }
    return k;
}

pub int K_trash(BK_K *k) {
    TM_trash(k->tm);
    EM_trash(k->em);
    SM_trash(k->sm);
    k->mm->free(k);
    return 0;
}

#endif // __BK_K_C
