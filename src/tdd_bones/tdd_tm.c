// ---------------------------------------------------------------------------------------------------------------------
//
//                             Copyright (c) 2023-2025 David Briant. All rights reserved.
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
// TDD TESTS - TYPE MANAGER
// ---------------------------------------------------------------------------------------------------------------------

#include "../bk/pp.c"
#include "../bk/k.c"
#include "../bk/tp.c"
#include "../bk/tp.c"
//#include "../bk/k.c"

pvt void die_(char const *preamble, char const *msg, va_list args) {
    fprintf(stdout, "%s", preamble);
    vfprintf(stdout, msg, args);
    fprintf(stdout, "\n");
    exit(1);
}


#define countArgs(x, y...)  countArgsImpl((x) , ## y, 0)


pvt int countArgsImpl(int x, ...) {
    va_list ap;  int e;  int count = 1;
    va_start(ap, x);
    while ((e = va_arg(ap, int)) != 0) count ++;
    va_end(ap);
    return count;
}

pvt TM_TLID_T * init_tl(btypeid_t * tl, int num, ...) {
    va_list ap;
    va_start(ap, num);
    tl[0] = num;
    for (int i=1; i <= num; i++) tl[i] = va_arg(ap, btypeid_t);
    va_end(ap);
    return tl;
}

pvt symid_t * init_sl(BK_K *k, symid_t * sl, int num, ...) {
    va_list ap;
    va_start(ap, num);
    sl[0] = num;
    for (int i=1; i <= num; i++) sl[i] = sm_id(k->sm, va_arg(ap, char*));
    va_end(ap);
    return sl;
}

pvt btypeid_t check_nnat(btypeid_t t, char const *msg, ...) {
    if (t == B_NAT) {
        va_list args;
        va_start(args, msg);
        die_("", msg, args);
        va_end(args);
    };
    return t;
}

pvt btypeid_t check_btype(BK_TM *tm, btypeid_t t, char const *name, char const *filename, int lineno) {
    if (t == B_NAT) {
        fprintf(stdout, "%s @ %i: %s is NaT", filename, lineno, name);
        fprintf(stdout, "\n");
        exit(1);
    };
    if (TM_BMT_ID(tm->btsummary_by_btypeid[t]) == bmterr) {
        fprintf(stdout, "%s @ %i: %s is bmterr, i.e. uninitialised", filename, lineno, name);
        fprintf(stdout, "\n");
        exit(1);
    };
    return t;
}



pvt TPN test_construction(BK_MM *mm, Buckets *buckets, BK_TP *tp) {
    btypeid_t base, tFred, tJoe, tFredJoe, tFredOrJoe, tTup0, tTupFredJoe, tStruct, tSeq, tFn0, tFn2, t1, t2, t3, t;
    btypeid_t tupid;  TM_TLID_T tlid;

    BK_K *k = K_create(mm, buckets);  BK_TM* tm = k->tm;

    PP(debug, "kernel created");

    base = tm->next_btypeId;
    tFred = base++;
    tJoe = base++;
    tFredJoe = base++;
    tFredOrJoe = base++;
    tTup0 = base++;
    tTupFredJoe = base++;
    tStruct = base++;
    tSeq = base++;
    tFn0 = base++;
    tFn2 = base++;
    tm_reserve_btypeids(tm, base + 100);


    // atom
    t = tm_lookup(tm, "joe");
    check(t == 0, "%s @ %i: id != B_NAT (should be %i)", __FILE__, __LINE__, t);

    t = tm_lookup(tm, "sally");
    check(t == 0, "%s @ %i: id != B_NAT (should be %i)", __FILE__, __LINE__, t);
    
    t = tm_bind(tm, "fred", tm_init_atom(tm, tFred, 0));
    check(t == tFred, "%s @ %i: t == %i (should be %i)", __FILE__, __LINE__, t, tFred);

    t = tm_bind(tm, "joe", tm_init_atom(tm, tJoe, 0));
    check(t == tJoe, "%s @ %i: t == %i (should be %i)", __FILE__, __LINE__, t, tJoe);

    t = tm_bind(tm, "joe", tm_lookup(tm, "joe"));
    check(t == tJoe, "%s @ %i: t == %i (should be %i)", __FILE__, __LINE__, t, tJoe);
    
    t = tm_lookup(tm, "joe");
    check(t == tJoe, "%s @ %i: t == %i (should be %i)", __FILE__, __LINE__, t, tJoe);

    btypeid_t *typelist = malloc(3 * sizeof(btypeid_t));
    symid_t *symlist = malloc(3 * sizeof(symid_t));
    typelist[0] = 2;
    typelist[1] = tm_lookup(tm, "fred");
    typelist[2] = tm_lookup(tm, "joe");
    symlist[0] = 2;
    symlist[1] = sm_id(k->sm, "fred");
    symlist[2] = sm_id(k->sm, "joe");


    // intersection
    t = tm_inter(tm, tFredJoe, typelist);
    check(t == tFredJoe, "%s @ %i: t == %i (should be %i)", __FILE__, __LINE__, t, tFredJoe);

    t = tm_inter(tm, B_NEW, typelist);
    check(t == tFredJoe, "%s @ %i: t == %i (should be %i)", __FILE__, __LINE__, t, tFredJoe);


    // union
    t = tm_union(tm, tFredOrJoe, typelist);
    check(t == tFredOrJoe, "%s @ %i: t == %i (should be %i)", __FILE__, __LINE__, t, tFredOrJoe);

    t2 = tm_union(tm, B_NEW, typelist);
    check(t2 == tFredOrJoe, "%s @ %i: t == %i (should be %i)", __FILE__, __LINE__, t2, tFredOrJoe);


    // tuple
    typelist[0] = 0;
    tlid = tm_tlid(tm, typelist);
    t = tm_tuple(tm, tTup0, tlid);
    check(t == tTup0, "%s @ %i: t == %i (should be %i)", __FILE__, __LINE__, t, tTup0);

    t = tm_tuple(tm, B_NEW, tlid);
    check(t == tTup0, "%s @ %i: t == %i (should be %i)", __FILE__, __LINE__, t, tTup0);

    typelist[0] = 2;
    tlid = tm_tlid(tm, typelist);
    t = tm_tuple(tm, tTupFredJoe, tlid);
    check(t == tTupFredJoe, "%s @ %i: t == %i (should be %i)", __FILE__, __LINE__, t, tTupFredJoe);

    t = tm_tuple(tm, B_NEW, tlid);
    check(t == tTupFredJoe, "%s @ %i: t == %i (should be %i)", __FILE__, __LINE__, t, tTupFredJoe);


    // struct
    SM_SLID_T slid = sm_slid(k->sm, symlist);

    t = tm_struct(tm, tStruct, slid, tlid);
    check(t == tStruct, "%s @ %i: t == %i (should be %i)", __FILE__, __LINE__, t, tStruct);

    t = tm_struct(tm, B_NEW, slid, tlid);
    check(t == tStruct, "%s @ %i: t == %i (should be %i)", __FILE__, __LINE__, t, tStruct);


     // sequence
    t = tm_seq(tm, tSeq, tFredOrJoe);
    check(t == tSeq, "%s @ %i: t == %i (should be %i)", __FILE__, __LINE__, t, tSeq);

    t = tm_seq(tm, B_NEW, tFredOrJoe);
    check(t == tSeq, "%s @ %i: t == %i (should be %i)", __FILE__, __LINE__, t, tSeq);

    t3 = tm_seq_t(tm, tSeq);
    check(t3 == tFredOrJoe, "%s @ %i: t3 == %i (should be %i)", __FILE__, __LINE__, t3, tFredOrJoe);


    // map


    // fn

    // (fred, joe) -> sally
    t = tm_fn(tm, tFn2, tTupFredJoe, tm_bind(tm, "sally", tm_init_atom(tm, B_NEW, 0)));
    check(t == tFn2, "%s @ %i: t == %i (should be %i)", __FILE__, __LINE__, t, tFn2);

    t = tm_fn(tm, B_NEW, tupid, tm_lookup(tm, "sally"));
    check(t == tFn2, "%s @ %i: t == %i (should be %i)", __FILE__, __LINE__, t, tFn2);

    // () -> sally
    typelist[0] = 0;
    t = tm_fn(tm, tFn0, tTup0, tm_lookup(tm, "sally"));
    check(t == tFn0, "%s @ %i: t == %i (should be %i)", __FILE__, __LINE__, t, tFn0);


    // schemavar
    // done in kernel creation


    K_trash(k);

    return tp_tpn_printf(tp, "test_construction() passed");
}

pvt TPN test_orthogonal_spaces(BK_MM *mm, Buckets *buckets, BK_TP *tp) {
    // test construction with spaces
    char *s=0;  symid_t *sl;  btypeid_t *tl;  BK_K *k;  BK_TM *tm;  btypeid_t ccyfx, ccy, fx, t, CCY, GBP, USD;

    k = K_create(mm, buckets);  tm = k->tm;
    sl = malloc(11 * sizeof(symid_t));     // 10 items + size field
    tl = malloc(11 * sizeof(btypeid_t));

    // test simple orthogonality
    ccyfx = tm_bind(tm, "ccyfx", tm_init_atom(tm, B_NEW, 0);        // mem is a built-in type? don't think so
    ccy = tm_bind(tm, "ccy", tm_init_atom(tm, tm_reserve(tm, B_NEW, ccyfx, 0), 0));
    fx = tm_bind(tm, "fx", tm_init_atom(tm, tm_reserve(tm, B_NEW, ccyfx, 0), 0));

    tm_reserve(tm, B_NEW, 0, 0);

    check(tm_spaceid(tm, ccy) == ccyfx, "%s @ %i: ccy.space != ccyfx", __FILE__, __LINE__);
    check(tm_root_spaceid(tm, fx) == ccyfx, "%s @ %i: ccy.root != ccyfx", __FILE__, __LINE__);

    tl[0] = 2;  tl[1] = ccy;  tl[2] = fx;
    t = tm_inter(tm, B_NEW, tl);

    t = tm_interv(tm, B_NEW, 2, ccy, fx);
    check(t == B_NAT, "%s @ %i: ccy & fx != B_NAT", __FILE__, __LINE__);


    CCY = tm_bind(tm, "CCY", tm_init_atom(tm, B_NEW, 0);        // mem is a built-in type? don't think so

    GBP = tm_reserve(tm, B_NEW, CCY, 0);
    GBP = tm_interv(tm, GBP, 2, GBP, ccy);

    USD = tm_reserve(tm, B_NEW, CCY, 0;
    USD = tm_interv(tm, USD, 2, USD, ccy);

    t = tm_interv(tm, B_NEW, 2, GBP, USD);
    check(t == B_NAT, "%s @ %i: GBP & USD != B_NAT", __FILE__, __LINE__);

    t = tm_interv(tm, B_NEW, 2, GBP, tm_bind(tm, "fred", tm_init_atom(tm, B_NEW, 0)));
    check(t != B_NAT, "%s @ %i: GBP & USD != B_NAT", __FILE__, __LINE__);

    free(tl);  free(sl);  K_trash(k);
    return tp_tpn_printf(tp, "test_orthogonal_spaces() passed");
}

pvt TPN test_minus(BK_MM *mm, Buckets *buckets, BK_TP *tp) {
    // test construction with spaces
    char *s=0;  symid_t *sl;  btypeid_t *tl;  BK_K *k;  BK_TM *tm;  btypeid_t t, t1, t2, t3, t4;

    k = K_create(mm, buckets);  tm = k->tm;
    sl = malloc(11 * sizeof(symid_t));     // 10 items + size field
    tl = malloc(11 * sizeof(btypeid_t));

    // test simple orthogonality
    t3 = tm_bind(tm, "GBP", tm_init_atom(tm, B_NEW, 0));        // mem is a built-in type? don't think so
    t2 = tm_bind(tm, "ccy", tm_init_atom(tm, B_NEW, 0));
    t1 = tm_bind(tm, "f64", tm_init_atom(tm, B_NEW, 0));
    t4 = tm_bind(tm, "u32", tm_init_atom(tm, B_NEW, 0));

    t = tm_minus(tm, B_NEW, tm_interv(tm, B_NEW, 3, t1, t2, t3), t2);

    check(t == tm_interv(tm, B_NEW, 2, t1, t3), "%s @ %i: t1  &t2 & t3 - t2 != t1 & t3", __FILE__, __LINE__);

    free(tl);  free(sl);  K_trash(k);
    return tp_tpn_printf(tp, "test_minus() passed");
}

pvt TPN test_construction_extended(BK_MM *mm, Buckets *buckets, BK_TP *tp) {
    // test construction with spaces, recursive, explicit and implicit in

    char *s=0;  symid_t *sl;  btypeid_t *tl;  BK_K *k;  BK_TM *tm;
    btypeid_t T1, T2, mem, null, null2, ptrSpc, ptr, ptrptr, constSpc, mut, const_, constPtrSpc, mutPtr, constPtr;
    btypeid_t constPtrPtrSpc, mutPtrPtr, constPtrPtr;
    btypeid_t f64, u32, u64, i64, i64fracStruct, i64frac, t, t7, t8, actual;

    k = K_create(mm, buckets);
    PP(debug, "%s @ %i: kernel created", __FILE__, __LINE__);

    tm = k->tm;
    sl = malloc(11 * sizeof(symid_t));     // 10 items + size field
    tl = malloc(11 * sizeof(btypeid_t));

    T1 = tm_bind(tm, "T1", tm_schemavar(tm, B_NEW));
    T2 = tm_bind(tm, "T2", tm_schemavar(tm, B_NEW));

    mem = tm_bind(tm, "mem", tm_init_atom(tm, B_NEW, 0));        // mem is a built-in type? don't think so

    null = tm_bind(tm, "null", tm_init_atom(tm, tm_reserve(tm, B_NEW, mem, 0), 0));
    null2 = tm_bind(tm, "null", tm_init_atom(tm, tm_reserve(tm, B_NEW, mem, 0), 0));
    check(null2 == B_NAT, "%s @ %i: null2 != B_NAT", __FILE__, __LINE__);

    ptrSpc = tm_bind(tm, "ptrSpc", tm_init_atom(tm, B_NEW, 0));
    ptr = check_btype(tm, tm_bind(tm, "ptr", tm_init_atom(tm, tm_reserve(tm, B_P, ptrSpc, 0), 0)), "ptr == B_NAT", __FILE__, __LINE__);

    constSpc = tm_bind(tm, "constSpc", tm_init_atom(tm, B_NEW, 0));
    mut = check_nnat(tm_bind(tm, "mut", tm_init_atom(tm, tm_reserve(tm, B_NEW, constSpc, true), 0)), "%s @ %i: mut == B_NAT", __FILE__, __LINE__);          // mut is implicit in C but explicit in bones
    const_ = check_nnat(tm_bind(tm, "const", tm_init_atom(tm, tm_reserve(tm, B_NEW, constSpc, 0), 0)), "%s @ %i: const_ == B_NAT", __FILE__, __LINE__);  // const is implicit in bones

    constPtrSpc = tm_bind(tm, "constPtrSpc", tm_init_atom(tm, B_NEW, 0));
    mutPtr = tm_bind(tm, "mutPtr", tm_init_atom(tm, tm_reserve(tm, B_NEW, constPtrSpc, true), 0));
    constPtr = tm_bind(tm, "constPtr", tm_init_atom(tm, tm_reserve(tm, B_NEW, constPtrSpc, 0), 0));

    constPtrPtrSpc = tm_bind(tm, "constPtrSpc", tm_init_atom(tm, B_NEW, 0));
    mutPtrPtr = tm_bind(tm, "mutPtrPtr", tm_init_atom(tm, tm_reserve(tm, B_NEW, constPtrPtrSpc, true), 0));
    constPtrPtr = tm_bind(tm, "constPtrPtr", tm_init_atom(tm, tm_reserve(tm, B_NEW, constPtrPtrSpc, 0), 0));


    // f64: f64 & mem in mem
    f64 = check_nnat(tm_reserve(tm, B_F64, mem, 0, 0), "f64 == B_NAT");
    check(f64 != 0, "%s @ %i: f64 == B_NAT", __FILE__, __LINE__);
    check(tm_lookup(tm, "f64") != f64, "%s @ %i: t == %i (should not be %i)", __FILE__, __LINE__, tm_lookup(tm, "f64"), f64);
    check(tm_bmetatypeid(tm, f64) == bmterr, "%s @ %i: tm_bmetatypeid(f64) != bmterr", __FILE__, __LINE__);

    // name as
    f64 = tm_bind(tm, "f64", f64);
    check(f64 != 0, "%s @ %i: f64 == B_NAT", __FILE__, __LINE__);
    check(tm_lookup(tm, "f64") == f64, "%s @ %i: t == %i (should be %i)", __FILE__, __LINE__, tm_lookup(tm, "f64"), f64);
    check(strcmp(tm_s8(tm, tp, f64).cs, "f64") == 0, "%s @ %i: pp(f64) != \"f64\" but got \"%s\"", __FILE__, __LINE__, tm_s8(tm, tp, f64).cs);

    // define the recursive type as an intersection
    f64 = tm_interv(tm, f64, 2, f64, mem);       // tm, self, vspace, numTypes, t1, t2...
    check(f64 != 0, "%s @ %i: f64 == B_NAT", __FILE__, __LINE__);
    check(tm_bmetatypeid(tm, f64) == bmtint, "%s @ %i: tm_bmetatypeid(f64) != bmtint", __FILE__, __LINE__);

    // u32: atm in mem
    u32 = tm_bind(tm, "u32", tm_init_atom(tm, tm_reserve(tm, B_U32, mem, 0), 0));

    // u64: atm in mem
    u64 = tm_bind(tm, "u64", tm_init_atom(tm, tm_reserve(tm, B_U64, mem, 0), 0));

    // i64: atm in mem
    i64 = check_btype(tm, tm_bind(tm, "i64", tm_init_atom(tm, tm_reserve(tm, B_I64, mem, 0), 0)), "%s @ %i: i64 is NaT", __FILE__, __LINE__);


    // i64frac: {num:i64, den:i64} in mem
    i64fracStruct = tm_struct(tm, tm_reserve(tm, B_NEW, mem, 0),
        sm_slid(k->sm, init_sl(k, sl, 2, "num", "den")),
        tm_tlid(tm, init_tl(tl, 2, i64, i64))
    );
    i64frac = check_nnat(tm_bind(tm, "i64frac", i64fracStruct), "%s @ %i: i64fracStruct == B_NAT", __FILE__, __LINE__);


    // model ccy and fx such that storage can be f64 or frac
    btypeid_t ccyfx, CCY, ccy, ccyfrac, GBP, GBPFrac, USD;
    ccyfx = tm_bind(tm, "ccyfx", tm_init_atom(tm, B_NEW, 0));

    CCY = tm_bind(tm, "CCY", tm_init_atom(tm, tm_reserve(tm, B_NEW, ccyfx, 0), 0));


    // ccy and ccyfrac are part of the space of CCY
    ccy = tm_bind(tm, "ccy", tm_reserve(tm, B_NEW, CCY, 0));
    ccy = tm_interv(tm, ccy, 2, ccy, f64);

    ccyfrac = tm_bind(tm, "ccyfrac", tm_reserve(tm, B_NEW, CCY, 0));
    ccyfrac = tm_interv(tm, ccyfrac, 2, ccyfrac, i64frac);
    check_nnat(ccyfrac, "%s @ %i: ccyfrac == B_NAT", __FILE__, __LINE__);

    // GBP: GBP & ccy in ccy
    GBP = tm_bind(tm, "GBP", tm_reserve(tm, B_NEW, 0, 0));
    GBP = tm_interv_in(tm, GBP, ccy, 2, ccy, GBP);

    // GBPFrac: GBPFrac & ccyfrac in ccyfrac
    GBPFrac = tm_bind(tm, "GBPFrac", tm_reserve(tm, B_NEW, 0, 0));
    GBPFrac = tm_interv_in(tm, GBPFrac, ccyfrac, 2, ccyfrac, GBPFrac);

    // USD: USD & ccy in ccy
    USD = tm_bind(tm, "USD", tm_reserve(tm, B_NEW, 0, 0));
    USD = tm_interv_in(tm, USD, ccy, 2, ccy, USD);



    // FX: FX & {dom:CCY[T1], for:CCY[T2]}
    btypeid_t FX, fxStruct, fx, fxfrac, GBPUSD, gbpusdStruct;
    FX = tm_bind(tm, "FX", tm_reserve(tm, B_NEW, ccyfx, 0));
    fxStruct = tm_struct(tm, B_NEW,
        sm_slid(k->sm, init_sl(k, sl, 2, "dom", "for")),
        tm_tlid(tm, init_tl(tl, 2, tm_interv(tm, B_NEW, 2, CCY, T1), tm_interv(tm, B_NEW, 2, CCY, T2)))
    );
    FX = tm_interv(tm, FX, 2, FX, fxStruct);

    fx = tm_bind(tm, "fx", tm_reserve(tm, B_NEW, FX, 0));
    fx = tm_interv(tm, fx, 2, fx, f64);

    fxfrac = tm_bind(tm, "fxfrac", tm_reserve(tm, B_NEW, FX, 0));
    fxfrac = tm_interv(tm, fxfrac, 2, fxfrac, i64frac);


    // GBPUSD: fx & {dom:GBP, for:USD}

    GBPUSD = tm_bind(tm, "GBPUSD", tm_reserve(tm, B_NEW, 0, 0));
    gbpusdStruct = tm_struct(tm, B_NEW,
        sm_slid(k->sm, init_sl(k, sl, 2, "dom", "for")),
        tm_tlid(tm, init_tl(tl, 2, GBP, USD))
    );
    // IMPORTANT: here {dom:CCY[T1], for:CCY[T2]} & {dom:GBP, for:USD} collapses to {dom:GBP, for:USD} as no residual
    // A & B answers the most concrete bit - so intersections must be done deeply - how about unions?
    // if A <: B then answer A else if B <: A answer B alse answer A & B
    GBPUSD = tm_interv(tm, GBPUSD, 3, fx, GBPUSD, gbpusdStruct);


    // abstractF64Tree: {lhs: abstractF64Tree + f64 + null, rhs: abstractF64Tree + f64 + null}
    // recursive types do not need to be named
    btypeid_t abstractF64Tree;
    abstractF64Tree = tm_reserve(tm, B_NEW, 0, 0, 0);
    abstractF64Tree = tm_struct(tm, abstractF64Tree,
        sm_slid(k->sm, init_sl(k, sl, 2, "lhs", "rhs")),
        tm_tlid(tm, init_tl(tl, 2,
            tm_unionv(tm, B_NEW, 3, abstractF64Tree, f64, null),
            tm_unionv(tm, B_NEW, 3, abstractF64Tree, f64, null)
        ))
    );


    // do a C struct where f64Tree <: abstractF64Tree
    // f64Tree: abstractF64Tree & {lhs: ptr & f64Tree + f64 + null, rhs: ptr & f64Tree + f64 + null}
    // what about:
    // f64Tree: abstractF64Tree & {T1: ptr & f64Tree + f64 + null, T2: ptr & f64Tree + f64 + null}
    btypeid_t f64Tree;
    f64Tree = tm_bind(tm, tm_reserve(tm, B_NEW, 0, 0, 0), "f64Tree");
    f64Tree = tm_interv(tm, f64Tree, 4,
        mem,
        f64Tree,
        abstractF64Tree,
        tm_struct(tm, B_NEW,
            sm_slid(k->sm, init_sl(k, sl, 2, "lhs", "rhs")),
            tm_tlid(tm, init_tl(tl, 2,
                tm_unionv(tm, B_NEW, 3, tm_interv(tm, B_NEW, 2, ptr, f64Tree), f64, null),
                tm_unionv(tm, B_NEW, 3, tm_interv(tm, B_NEW, 2, ptr, f64Tree), f64, null)
            )
        )
    ));
    check_btype(tm, f64Tree, "f64Tree", __FILE__, __LINE__);


    // do a tree[T1] such that f64Tree <: tree[T1]
    // tree: {lhs: tree[T1] + T1 + null, rhs: tree[T1] + T1 + null}
    btypeid_t tree;
    tree = tm_bind(tm, tm_reserve(tm, B_NEW, 0, 0, 0), "tree");
    tree = tm_struct(tm, tree,
        sm_slid(k->sm, init_sl(k, sl, 2, "lhs", "rhs")),
        tm_tlid(tm, init_tl(tl, 2,
            tm_unionv(tm, B_NEW, 3, tm_interv(tm, B_NEW, 2, tree, T1), T1, null),
            tm_unionv(tm, B_NEW, 3, tm_interv(tm, B_NEW, 2, tree, T1), T1, null)
        ))
    );


    PP(debug, "u32, u64, ccy, GBP, USD: %i, %i, %i, %i, %i", tm_lookup(tm, "u32"), tm_lookup(tm, "u64"), tm_lookup(tm, "ccy"), tm_lookup(tm, "GBP"), tm_lookup(tm, "USD"));

    // check construction returns identical objects
    t7 = check_btype(tm, tm_interv(tm, B_NEW, 2, CCY, u32), "t7", __FILE__, __LINE__);
    t8 = check_btype(tm, tm_interv(tm, B_NEW, 2, CCY, u32), "t8", __FILE__, __LINE__);
    check(t7 == t8, "%s @ %i: t7 != t8 got %i and %i", __FILE__, __LINE__, t7, t8);

    // check u32 doesn't mix with u64
    actual = tm_interv(tm, B_NEW, 2, u32, u64);
    check(actual == 0, "%s @ %i: actual == %i (should be %i)", __FILE__, __LINE__, actual, 0);

    // check u32 doesn't mix with u64 - nested
    actual = tm_interv(tm, B_NEW, 2, GBP, USD);
    check(actual == 0, "%s @ %i: actual == %i (should be %i)", __FILE__, __LINE__, actual, 0);


    // need to rethink tests here - modelling GBP etc correctly is getting in way of writing clear tests
//    PP(info, "u32 & ccy: %i, (%s)", t7, tm_s8_typelist(tm, tp, tm_inter_tl(tm, t7)).cs);
//    t8 = tm_interv(tm, B_NEW, 2, u64, ccy);
//    PP(info, "u64 & ccy: %i, (%s)", t8, tm_s8_typelist(tm, tp, tm_inter_tl(tm, t8)).cs);
//    actual = tm_interv(tm, B_NEW, 2, t7, t8);
//    if (actual != 0) s = tm_s8_typelist(tm, tp, tm_inter_tl(tm, actual)).cs;
//    check(actual == 0, "%s @ %i: t == %i (should be %i) - %s", __FILE__, __LINE__, actual, s);
//
//    // check GBP mixes with USD
//    actual = tm_interv(tm, B_NEW, 2, GBP, USD);
//    check(actual != 0, "%s @ %i: t == %i (should not be %i)", __FILE__, __LINE__, actual, 0);
//    t = tm_interv(tm, B_NEW, 2, GBP, u32);
//    actual = tm_interv(tm, B_NEW, 2, t, USD);
//    check(actual != 0, "%s @ %i: t == %i (should not be %i)", __FILE__, __LINE__, actual, 0);


    free(tl);  free(sl);  K_trash(k);
    return tp_tpn_printf(tp, "test_orthogonal() passed");
}



int main() {
    Buckets buckets; BK_TP tp;

    g_logging_level = info;

    Buckets_init(&buckets, 64);
    TP_init(&tp, 0, &buckets);

    BK_MM *mm = MM_create();

    S8 txt = tp_s8(&tp, tp_tpn_printf(&tp, "kernel created"));

    PP(debug, "%i", os_cache_line_size());
    PP(debug, "%i", os_page_size());
    PP(info, tp_s8(&tp, test_construction(mm, &buckets, &tp)).cs);
    PP(info, tp_s8(&tp, test_orthogonal_spaces(mm, &buckets, &tp)).cs);
    PP(info, tp_s8(&tp, test_construction_extended(mm, &buckets, &tp)).cs);
    PP(info, tp_s8(&tp, test_minus(mm, &buckets, &tp)).cs);
    PP(debug, "passed");
    Buckets_finalise(&buckets);
    MM_trash(mm);
    return 0;
}
