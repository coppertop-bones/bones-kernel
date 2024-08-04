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
    n += tm_atom(tm, B_M8, "m8") == 0;
    n += tm_atom(tm, B_M16, "m16") == 0;
    n += tm_atom(tm, B_M32, "m32") == 0;
    n += tm_atom(tm, B_M64, "m64") == 0;
    n += tm_atom(tm, B_LITINT, "litint") == 0;
    n += tm_atom(tm, B_I32, "i32") == 0;

    n += tm_schemavar(tm, B_T, "T") == 0;
    n += tm_schemavar(tm, B_T1, "T1") == 0;
    n += tm_schemavar(tm, B_T2, "T2") == 0;
    n += tm_schemavar(tm, B_T3, "T3") == 0;

    n += tm_schemavar(tm, B_N, "N") == 0;
    n += tm_schemavar(tm, B_N1, "N1") == 0;
    n += tm_schemavar(tm, B_N2, "N2") == 0;
    n += tm_schemavar(tm, B_N3, "N3") == 0;
    n += tm_schemavar(tm, B_N4, "N4") == 0;
    n += tm_schemavar(tm, B_N5, "N5") == 0;
    n += tm_schemavar(tm, B_N6, "N6") == 0;
    n += tm_schemavar(tm, B_N7, "N7") == 0;
    n += tm_schemavar(tm, B_N8, "N8") == 0;
    n += tm_schemavar(tm, B_N9, "N9") == 0;

    if (n) {
        mm->free(tm);
        die("%i conflicts in tm_atom\n", n);
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
