// ---------------------------------------------------------------------------------------------------------------------
// Copyright 2025 David Briant, https://github.com/coppertop-bones. Licensed under the Apache License, Version 2.0
//
// K - KERNEL
// ---------------------------------------------------------------------------------------------------------------------

#ifndef SRC_BK_K_C
#define SRC_BK_K_C "bk/k.c"

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
    tm_reserve_block(tm, B_FIRST_UNRESERVED_TYPEID);
    n += tm_bind(tm, "mem", tm_init_atom(tm, B_MEM, B_NAT, false)) == 0;
//    n += tm_bind(tm, "m8", tm_init_atom(tm, B_M8, 0)) == 0;
//    n += tm_init_atom(tm, B_M16, "m16") == 0;
//    n += tm_init_atom(tm, B_M32, "m32") == 0;
//    n += tm_init_atom(tm, B_M64, "m64") == 0;
//    n += tm_init_atom(tm, B_LITINT, "litint") == 0;
//    n += tm_init_atom(tm, B_I32, "i32") == 0;

    n += tm_bind(tm, "T", tm_schemavar(tm, B_T)) == 0;
    // OPEN: do some other time
//    char *label = "T#"
//    for (int i=1; i <= 20; i++) {
//        label[1] = ''
//        n += tm_bind("T1", tm_schemavar(tm, B_T + i)) == 0;
//    }
    n += tm_bind(tm, "T1", tm_schemavar(tm, B_T1)) == 0;
    n += tm_bind(tm, "T2", tm_schemavar(tm, B_T2)) == 0;
    n += tm_bind(tm, "T3", tm_schemavar(tm, B_T3)) == 0;
    n += tm_bind(tm, "T4", tm_schemavar(tm, B_T4)) == 0;
    n += tm_bind(tm, "T5", tm_schemavar(tm, B_T5)) == 0;
    n += tm_bind(tm, "T6", tm_schemavar(tm, B_T6)) == 0;
    n += tm_bind(tm, "T7", tm_schemavar(tm, B_T7)) == 0;
    n += tm_bind(tm, "T8", tm_schemavar(tm, B_T8)) == 0;
    n += tm_bind(tm, "T9", tm_schemavar(tm, B_T9)) == 0;
    // n += tm_bind(tm, "T10", tm_schemavar(tm, B_T10)) == 0;
    // n += tm_bind(tm, "T11", tm_schemavar(tm, B_T11)) == 0;
    // n += tm_bind(tm, "T12", tm_schemavar(tm, B_T12)) == 0;
    // n += tm_bind(tm, "T13", tm_schemavar(tm, B_T13)) == 0;
    // n += tm_bind(tm, "T14", tm_schemavar(tm, B_T14)) == 0;
    // n += tm_bind(tm, "T15", tm_schemavar(tm, B_T15)) == 0;
    // n += tm_bind(tm, "T16", tm_schemavar(tm, B_T16)) == 0;
    // n += tm_bind(tm, "T17", tm_schemavar(tm, B_T17)) == 0;
    // n += tm_bind(tm, "T18", tm_schemavar(tm, B_T18)) == 0;
    // n += tm_bind(tm, "T19", tm_schemavar(tm, B_T19)) == 0;
    // n += tm_bind(tm, "T20", tm_schemavar(tm, B_T20)) == 0;

   n += tm_bind(tm, "N", tm_init_atom(tm, B_N, B_NAT, false)) == 0;
   n += tm_bind(tm, "N1", tm_init_atom(tm, B_N1, B_NAT, false)) == 0;
   n += tm_bind(tm, "N2", tm_init_atom(tm, B_N2, B_NAT, false)) == 0;
   n += tm_bind(tm, "N3", tm_init_atom(tm, B_N3, B_NAT, false)) == 0;
   n += tm_bind(tm, "N4", tm_init_atom(tm, B_N4, B_NAT, false)) == 0;
   n += tm_bind(tm, "N5", tm_init_atom(tm, B_N5, B_NAT, false)) == 0;
   n += tm_bind(tm, "N6", tm_init_atom(tm, B_N6, B_NAT, false)) == 0;
   n += tm_bind(tm, "N7", tm_init_atom(tm, B_N7, B_NAT, false)) == 0;
   n += tm_bind(tm, "N8", tm_init_atom(tm, B_N8, B_NAT, false)) == 0;
   n += tm_bind(tm, "N9", tm_init_atom(tm, B_N9, B_NAT, false)) == 0;

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

#endif  // SRC_BK_K_C
