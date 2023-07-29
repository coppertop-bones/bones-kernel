// ---------------------------------------------------------------------------------------------------------------------
// Copyright 2025 David Briant, https://github.com/coppertop-bones. Licensed under the Apache License, Version 2.0
//
// TDD TESTS - TYPE MANAGER
// ---------------------------------------------------------------------------------------------------------------------

#include "../../src/bk/pp.c"
#include "../../src/bk/k.c"
#include "../../src/bk/tp.c"
#include "../../src/bk/tp.c"


pvt void die_(char const *preamble, char const *msg, va_list args) {
    fprintf(stderr, "%s", preamble);
    vfprintf(stderr, msg, args);
    fprintf(stderr, "\n");
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
    btypeid_t base, tFred, tJoe, tFredJoe, tFredOrJoe, tTup0, tTupFredJoe, tStruct, tSeq, tFn0, tFn2, t, t1, t2, t3, t4,
        tIsin, tTxt, tErr, t5, t6;
    TM_TLID_T tlid;

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
    tIsin = base++;
    tTxt = base++;
    tErr = base++;
    tm_reserve_block(tm, base + 100);


    // atom
    t = tm_lookup(tm, "joe");
    check(t == 0, "id != B_NAT (should be %i)", __FILE__, __LINE__, t);

    t = tm_lookup(tm, "sally");
    check(t == 0, "id != B_NAT (should be %i)", __FILE__, __LINE__, t);
    
    t = tm_bind(tm, "fred", tm_init_atom(tm, tFred, B_NAT, false));
    check(t == tFred, "t == %i (should be %i)", __FILE__, __LINE__, t, tFred);

    t = tm_bind(tm, "joe", tm_init_atom(tm, tJoe, B_NAT, false));
    check(t == tJoe, "t == %i (should be %i)", __FILE__, __LINE__, t, tJoe);

    t = tm_bind(tm, "joe", tm_lookup(tm, "joe"));
    check(t == tJoe, "t == %i (should be %i)", __FILE__, __LINE__, t, tJoe);

    t = tm_lookup(tm, "joe");
    check(t == tJoe, "t == %i (should be %i)", __FILE__, __LINE__, t, tJoe);

    t = tm_bind(tm, "isin", tm_init_atom(tm, tIsin, B_NAT, false));
    t = tm_bind(tm, "txt", tm_init_atom(tm, tTxt, B_NAT, false));
    t = tm_bind(tm, "err", tm_init_atom(tm, tErr, B_NAT, false));

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
    check(t == tFredJoe, "t == %i (should be %i)", __FILE__, __LINE__, t, tFredJoe);

    t = tm_inter(tm, B_NEW, typelist);
    check(t == tFredJoe, "t == %i (should be %i)", __FILE__, __LINE__, t, tFredJoe);


    // union
    typelist[1] = tm_lookup(tm, "fred");
    typelist[2] = tm_lookup(tm, "fred");
    t = tm_union(tm, B_NEW, typelist);
    check(t == tFred, "t == %i (should be %i)", __FILE__, __LINE__, t, tFred);

    typelist[1] = tm_lookup(tm, "joe");
    typelist[2] = tm_lookup(tm, "fred");
    t = tm_union(tm, tFredOrJoe, typelist);
    check(t == tFredOrJoe, "t == %i (should be %i)", __FILE__, __LINE__, t, tFredOrJoe);

    typelist[1] = tm_lookup(tm, "fred");
    typelist[2] = tm_lookup(tm, "joe");
    t2 = tm_union(tm, B_NEW, typelist);
    check(t2 == tFredOrJoe, "t == %i (should be %i)", __FILE__, __LINE__, t2, tFredOrJoe);


    // bug with inter and union
    typelist[1] = tTxt;
    typelist[2] = tIsin;
    t3 = tm_inter(tm, B_NEW, typelist);

    typelist[1] = t3;
    typelist[2] = tErr;
    t4 = tm_union(tm, B_NEW, typelist);

    typelist[1] = tIsin;
    typelist[2] = tTxt;

    tm_union_tl(tm, t4);
    t5 = tm_inter(tm, B_NEW, typelist);
    tm_union_tl(tm, t4);

    check(t3 == t5, "%i != %i", __FILE__, __LINE__, t3, t5);

    typelist[1] = tErr;
    typelist[2] = t5;
    t6 = tm_union(tm, B_NEW, typelist);

    check(t4 == t6, "%i != %i", __FILE__, __LINE__, t5, t6);


    typelist[1] = tm_lookup(tm, "fred");
    typelist[2] = tm_lookup(tm, "joe");


    // tuple
    typelist[0] = 0;
    tlid = tm_tlid_for(tm, typelist);
    t = tm_tuple(tm, tTup0, tlid);
    check(t == tTup0, "t == %i (should be %i)", __FILE__, __LINE__, t, tTup0);

    t = tm_tuple(tm, B_NEW, tlid);
    check(t == tTup0, "t == %i (should be %i)", __FILE__, __LINE__, t, tTup0);

    typelist[0] = 2;
    tlid = tm_tlid_for(tm, typelist);
    t = tm_tuple(tm, tTupFredJoe, tlid);
    check(t == tTupFredJoe, "t == %i (should be %i)", __FILE__, __LINE__, t, tTupFredJoe);

    t = tm_tuple(tm, B_NEW, tlid);
    check(t == tTupFredJoe, "t == %i (should be %i)", __FILE__, __LINE__, t, tTupFredJoe);


    // struct
    SM_SLID_T slid = sm_slid(k->sm, symlist);

    t = tm_struct(tm, tStruct, slid, tlid);
    check(t == tStruct, "t == %i (should be %i)", __FILE__, __LINE__, t, tStruct);

    t = tm_struct(tm, B_NEW, slid, tlid);
    check(t == tStruct, "t == %i (should be %i)", __FILE__, __LINE__, t, tStruct);


     // sequence
    t = tm_seq(tm, tSeq, tFredOrJoe);
    check(t == tSeq, "t == %i (should be %i)", __FILE__, __LINE__, t, tSeq);

    t = tm_seq(tm, B_NEW, tFredOrJoe);
    check(t == tSeq, "t == %i (should be %i)", __FILE__, __LINE__, t, tSeq);

    t3 = tm_seq_t(tm, tSeq);
    check(t3 == tFredOrJoe, "t3 == %i (should be %i)", __FILE__, __LINE__, t3, tFredOrJoe);


    // map


    // fn

    // (fred, joe) -> sally
    t = tm_fn(tm, tFn2, tTupFredJoe, tm_bind(tm, "sally", tm_init_atom(tm, B_NEW, B_NAT, false)));
    check(t == tFn2, "t == %i (should be %i)", __FILE__, __LINE__, t, tFn2);

    t = tm_fn(tm, B_NEW, tTupFredJoe, tm_lookup(tm, "sally"));
    check(t == tFn2, "t == %i (should be %i)", __FILE__, __LINE__, t, tFn2);

    // () -> sally
    typelist[0] = 0;
    t = tm_fn(tm, tFn0, tTup0, tm_lookup(tm, "sally"));
    check(t == tFn0, "t == %i (should be %i)", __FILE__, __LINE__, t, tFn0);


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
    ccyfx = tm_bind(tm, "ccyfx", tm_init_atom(tm, B_NEW, B_NAT, false));
    ccy = tm_bind(tm, "ccy", tm_init_atom(tm, tm_reserve_in(tm, ccyfx), B_NAT, false));
    fx = tm_bind(tm, "fx", tm_init_atom(tm, tm_reserve_in(tm, ccyfx), B_NAT, false));

    check(tm_spaceid(tm, ccy) == ccyfx, "ccy.space != ccyfx", __FILE__, __LINE__);
    check(tm_root_spaceid(tm, fx) == ccyfx, "ccy.root != ccyfx", __FILE__, __LINE__);

    tl[0] = 2;  tl[1] = ccy;  tl[2] = fx;
    t = tm_inter(tm, B_NEW, tl);

    t = tm_interv(tm, B_NEW, 2, ccy, fx);
    check(t == B_NAT, "ccy & fx != B_NAT", __FILE__, __LINE__);


    CCY = tm_bind(tm, "CCY", tm_init_atom(tm, B_NEW, B_NAT, false));

    GBP = tm_set_spaceid(tm, tm_reserve_tbc(tm), CCY);
    GBP = tm_clear_recursive_if_not(tm, tm_interv(tm, GBP, 2, GBP, ccy));

    USD = tm_set_spaceid(tm, tm_reserve_tbc(tm), CCY);
    USD = tm_clear_recursive_if_not(tm, tm_interv(tm, USD, 2, USD, ccy));

    t = tm_interv(tm, B_NEW, 2, GBP, USD);
    check(t == B_NAT, "GBP & USD != B_NAT", __FILE__, __LINE__);

    t = tm_interv(tm, B_NEW, 2, GBP, tm_bind(tm, "fred", tm_init_atom(tm, B_NEW, B_NAT, false)));
    check(t != B_NAT, "GBP & fred == B_NAT", __FILE__, __LINE__);

    free(tl);  free(sl);  K_trash(k);
    return tp_tpn_printf(tp, "test_orthogonal_spaces() passed");
}

pvt TPN debug2(BK_MM *mm, Buckets *buckets, BK_TP *tp) {
    // f64 linked list with recursive type
    // f64: null: atom
    // f64list: f64 * f64list + null

    char *s=0;  symid_t *sl;  btypeid_t *tl;  BK_K *k;  BK_TM *tm;  btypeid_t null, f64, f64list, tup1, u1, f64list2;
    TM_TLID_T tlid;

    k = K_create(mm, buckets);  tm = k->tm;
    sl = malloc(11 * sizeof(symid_t));          // 10 items + size field
    tl = malloc(11 * sizeof(btypeid_t));

    null = tm_bind(tm, "null", tm_init_atom(tm, B_NEW, B_NAT, false));
    f64 = tm_bind(tm, "f64", tm_init_atom(tm, B_NEW, B_NAT, false));
    f64list = tm_reserve_tbc(tm);

    tl[0] = 2;  tl[1] = f64list;  tl[2] = null;
    u1 = tm_union(tm, B_NEW, tl);
    check(u1 != B_NAT, "u1 == B_NAT", __FILE__, __LINE__);

    tl[0] = 2;  tl[1] = f64;  tl[2] = u1;
    tlid = tm_tlid_for(tm, tl);
    check(tlid != 0, "tlid == 0", __FILE__, __LINE__);

    tup1 = tm_tuple(tm, f64list, tlid);
    check(tup1 != B_NAT, "tlid == B_NAT", __FILE__, __LINE__);

    f64list2 = tm_bind(tm, "f64list", tup1);
    check(f64list2 == f64list, "f64list2 != f64list", __FILE__, __LINE__);

    f64list = tm_clear_recursive_if_not(tm, f64list);
    check(TM_IS_RECURSIVE(tm->btsummary_by_btypeid[f64list]), "f64list is not recursive", __FILE__, __LINE__);

    free(tl);  free(sl);  K_trash(k);
    return tp_tpn_printf(tp, "debug2() passed");
}

pvt TPN debug3(BK_MM *mm, Buckets *buckets, BK_TP *tp) {
    // C style const - ideally we would like mut & const  == NaT. However, we forego this for the vital features that:
    // a. mut <: const
    // b. unless otherwise indicated everything is implicitly mut (in C) or implicitly const (in bones)
    //
    // mut: tbc
    // const: const in const implicitly mut
    // mut: mut & const

    // joe: fred & const is legal
    // sally: fred & mut is legal
    // joe & arthur is legal
    // sally & arthur is legal
    // but fred & joe & sally & arthur is legal and equals sally & arthur
    // when reading types users will need to get used to the fact that const & mut is effectively

    // for display children of recursive types might be removed but the result is definition order dependent, e.g. consider:
    // sally: tbc
    // fred: fred & sally
    // joe: joe & fred
    // sally: sally & joe
    // fred & joe & sally would just display sally as it has the lowest btypeid

    char *s=0;  btypeid_t *tl;  BK_K *k;  BK_TM *tm;  btypeid_t mut, const_, fred, joe, sally, arthur, t;
    TM_TLID_T tlid, tlid2;

    k = K_create(mm, buckets);  tm = k->tm;
    tl = malloc(11 * sizeof(btypeid_t));        // 10 items + size field

    fred = tm_bind(tm, "fred", tm_init_atom(tm, B_NEW, B_NAT, false));
    sally = tm_bind(tm, "sally", tm_init_atom(tm, B_NEW, B_NAT, false));

    // mut: tbc
    mut = tm_bind(tm, "mut", tm_reserve_tbc(tm));

    // const: const in const implicitly mut
    const_ = tm_reserve_tbc(tm);
    const_ = tm_set_spaceid(tm, const_, const_);
    const_ = tm_bind(tm, "const", tm_init_atom(tm, const_, mut, false));

    // mut: mut & const
    tl[0] = 2;  tl[1] = mut;  tl[2] = const_;
    tlid = tm_inter_tlid_for(tm, tl);
    check(tlid != 0, "tlid == 0", __FILE__, __LINE__);
    t = tm_inter_for_tlid_or_create(tm, mut, tlid);
    check(t == mut, "mut != t", __FILE__, __LINE__);

    // get tlid and btypeid for const & mut
    tl[0] = 2;  tl[1] = const_;  tl[2] = mut;
    tlid2 = tm_inter_tlid_for(tm, tl);
    check(tlid2 == tlid, "tlid2 != tlid", __FILE__, __LINE__);
    t = tm_inter_for_tlid_or_create(tm, B_NEW, tlid);
    check(t == mut, "t != mut", __FILE__, __LINE__);
    t = tm_inter_for_tlid_or_create(tm, mut, tlid);
    check(t == mut, "t != mut", __FILE__, __LINE__);
    t = tm_inter_for_tlid_or_create(tm, B_NAT, tlid);
    check(t == 0, "t != 0", __FILE__, __LINE__);
    t = tm_inter_for_tlid_or_create(tm, const_, tlid);
    check(t == 0, "t != 0", __FILE__, __LINE__);

    // fred & const => (fred, const)
    tl[1] = fred;  tl[2] = const_;
    tlid = tm_inter_tlid_for(tm, tl);
    btypeid_t *res = tm->typelist_buf + tm->tlrp_by_tlid[tlid];
    check(res[0] == 2, "len(tlid) != 2", __FILE__, __LINE__);
    t = tm_inter_for_tlid_or_create(tm, B_NEW, tlid);
    check(t > const_, "t <= const_", __FILE__, __LINE__);

    // joe: fred & mut => (fred, mut, const)
    tl[1] = fred;  tl[2] = mut;
    PP(info, "TM_BMT_ID(fred): %i", TM_BMT_ID(*(tm->btsummary_by_btypeid + fred)) >> 28);
    PP(info, "TM_BMT_ID(mut): %i", TM_BMT_ID(*(tm->btsummary_by_btypeid + mut)) >> 28);
    tlid = tm_inter_tlid_for(tm, tl);
    res = tm->typelist_buf + tm->tlrp_by_tlid[tlid];
    check(res[0] == 3, "len(tlid) != 2", __FILE__, __LINE__);
    joe = tm_inter_for_tlid_or_create(tm, B_NEW, tlid);
    check(t != 0, "t == B_NAT", __FILE__, __LINE__);

    tl[1] = joe;  tl[2] = sally;
    tlid = tm_inter_tlid_for(tm, tl);
    check(tlid != 0, "tlid == 0", __FILE__, __LINE__);

    // for recursive intersections we could store two type lists - the first one is for partitioning and would be used for dispatch
    // the second would be the types, i.e. it's definition and used when creating new types
    // e.g. joe & sally = (fred & (mut & const)) & sally

    // the real problem is that we disallowed mut & const after the fact of creating it and it now bleeds all over the place

    free(tl);  K_trash(k);
    return tp_tpn_printf(tp, "debug3() passed");
}

pvt TPN debug4(BK_MM *mm, Buckets *buckets, BK_TP *tp) {
    // show that "intersection" is not enough but "intersection in" is required
    // ccy: atom
    // GBP_: GBP_ & ccy
    // USD_: USD_ & ccy
    // GBP: GBP & ccy in ccy
    // USD: USD & ccy in ccy

    // GBP_ & USD_ okay
    // GBP & USD illegal due to both being in ccy

    char *s=0;  symid_t *sl;  btypeid_t *tl;  BK_K *k;  BK_TM *tm;  btypeid_t ccy, GBP_, USD_, GBP, USD;
    TM_TLID_T tlid;

    k = K_create(mm, buckets);  tm = k->tm;
    sl = malloc(11 * sizeof(symid_t));     // 10 items + size field
    tl = malloc(11 * sizeof(btypeid_t));

    // ccy: atom
    ccy = tm_bind(tm, "ccy", tm_init_atom(tm, B_NEW, B_NAT, false));

    // GBP_: GBP_ & ccy
    GBP_ = tm_bind(tm, "GBP_", tm_reserve_tbc(tm));
    tl[0] = 2;  tl[1] = GBP_;  tl[2] = ccy;
    tlid = tm_inter_tlid_for(tm, tl);
    GBP_ = tm_clear_recursive_if_not(tm, tm_inter_for_tlid_or_create(tm, GBP_, tlid));

    // USD_: USD_ & ccy
    USD_ = tm_bind(tm, "USD_", tm_reserve_tbc(tm));
    tl[0] = 2;  tl[1] = USD_;  tl[2] = ccy;
    tlid = tm_inter_tlid_for(tm, tl);
    USD_ = tm_clear_recursive_if_not(tm, tm_inter_for_tlid_or_create(tm, USD_, tlid));

    // GBP_ & USD_
    tl[0] = 2;  tl[1] = GBP_;  tl[2] = USD_;
    tlid = tm_inter_tlid_for(tm, tl);
    check(tlid != 0, "tlid == 0", __FILE__, __LINE__);


    // GBP: GBP & ccy in ccy
    GBP = tm_bind(tm, "GBP", tm_set_spaceid(tm, tm_reserve_tbc(tm), ccy));
    tl[0] = 2;  tl[1] = GBP;  tl[2] = ccy;
    tlid = tm_inter_tlid_for(tm, tl);
    GBP = tm_clear_recursive_if_not(tm, tm_inter_for_tlid_or_create(tm, GBP, tlid));

    // USD: USD & ccy in ccy
    USD = tm_bind(tm, "USD", tm_set_spaceid(tm, tm_reserve_tbc(tm), ccy));
    tl[0] = 2;  tl[1] = USD;  tl[2] = ccy;
    tlid = tm_inter_tlid_for(tm, tl);
    USD = tm_clear_recursive_if_not(tm, tm_inter_for_tlid_or_create(tm, USD, tlid));

    // GBP & USD
    tl[0] = 2;  tl[1] = GBP;  tl[2] = USD;
    tlid = tm_inter_tlid_for(tm, tl);
    check(tlid == 0, "tlid != 0", __FILE__, __LINE__);


    free(tl);  free(sl);  K_trash(k);
    return tp_tpn_printf(tp, "debug4() passed");
}


pvt TPN debug5(BK_MM *mm, Buckets *buckets, BK_TP *tp) {
    // unit: dimension: mem: atom
    // f64: atom in mem
    // width: f64 & width in dimension
    // height: f64 & height in dimension
    // cm: f64 & cm in unit

    // width & height   == B_NAT
    // width & cm       != B_NATD

    char *s=0;  btypeid_t *tl;  BK_K *k;  BK_TM *tm;  btypeid_t mem, f64, unit, dimension, width, height, cm;
    TM_TLID_T tlid;

    k = K_create(mm, buckets);  tm = k->tm;
    tl = malloc(11 * sizeof(btypeid_t));

    // f64: unit: dimension: atom
    mem = tm_lookup(tm, "mem");
    dimension = tm_bind(tm, "dimension", tm_init_atom(tm, B_NEW, B_NAT, false));
    unit = tm_bind(tm, "unit", tm_init_atom(tm, B_NEW, B_NAT, false));
    f64 = tm_bind(tm, "f64", tm_init_atom(tm, tm_reserve_in(tm, mem), B_NAT, false));

    // width: f64 & width in dimension
    width = tm_bind(tm, "width", tm_set_spaceid(tm, tm_reserve_tbc(tm), dimension));
    tl[0] = 2;  tl[1] = width;  tl[2] = f64;
    tlid = tm_inter_tlid_for(tm, tl);
    width = tm_clear_recursive_if_not(tm, tm_inter_for_tlid_or_create(tm, width, tlid));

    // height: f64 & height in dimension
    height = tm_bind(tm, "height", tm_set_spaceid(tm, tm_reserve_tbc(tm), dimension));
    tl[0] = 2;  tl[1] = height;  tl[2] = f64;
    tlid = tm_inter_tlid_for(tm, tl);
    height = tm_clear_recursive_if_not(tm, tm_inter_for_tlid_or_create(tm, height, tlid));

    // cm: f64 & cm in unit
    cm = tm_set_spaceid(tm, tm_reserve_tbc(tm), unit);
    tl[0] = 2;  tl[1] = cm;  tl[2] = f64;
    tlid = tm_inter_tlid_for(tm, tl);
    cm = tm_clear_recursive_if_not(tm, tm_inter_for_tlid_or_create(tm, cm, tlid));
    check(cm != B_NAT, "cm == B_NAT", __FILE__, __LINE__);

    // width & height
    tl[0] = 2;  tl[1] = width;  tl[2] = height;
    tlid = tm_inter_tlid_for(tm, tl);
    check(tlid == 0, "tlid != 0", __FILE__, __LINE__);

    // width & cm = (width & (f64 in mem)) (cm & (f64 in mem))
    // want to be okay since lhs f64 in mem == rhs f64 in mem
    tl[0] = 2;  tl[1] = width;  tl[2] = cm;
    tlid = tm_inter_tlid_for(tm, tl);
    check(tlid != 0, "tlid == B_NAT", __FILE__, __LINE__);


    free(tl);  K_trash(k);
    return tp_tpn_printf(tp, "debug5() passed");
}


// expand intersections not in (a space)?

// algo
// rule A. if inter is in space A don't expand children in same space (because that would automarically fail)
// rule B. sort by space then btype
// rule C. in each space check max one btype

// height & (f64 in mem) in dimension
// width & (f64 in mem) in dimension
// [(dimension, height), (mem, f64), (dimension, height), (mem, f64)]   << conflict

// cm & (height (f64 in mem) in dimension)      <<< height in dimension conflicts with width in dimension
// cm & (width (f64 in mem) in dimension)

// show we need to sort on space and type
// isin: isin & (txt in mem)
// rate: rate & (f64 in mem)
// => isin, txt in mem, rate, f64 in mem            << conflict as desired

// isin: isin & (txt in mem)
// capitalised: capitalised & (txt in mem)
// => isin, txt in mem, capitalised, txt in mem     << no conflict as desired

// isin: isin & (txt in mem) in label
// rate: rate & (f64 in mem) in quantity
// => isin & (txt in mem) in label, txt in mem, rate & (f64 in mem) in quantity, f64 in mem

// f64 in mem
// txt in mem
// => f64 in mem, txt in mem

// const: const in const         => const in const
// mut: mut & const in const    => mut in const (const in const is not expanded byt rule A)
// => (const in const, mut & const in const)

// fred: atom
// mut: mut & const in const    => mut in const (const in const is not expanded byt rule A)
// => (fred, mut & const in const)

// if we expanded the const in const within the mut itersection then
// => (fred, mut & const in const, const in const)


// mut: mut & const in const    => mut in const, const in const
// mut: mut & const in const    => mut in const, const in const
// => (mut & const in const, mut & const in const)

// Rule A is useful and not untheoretical but chosen for an intuitive use case (where we want a subtype and a
// super-type to be orthogonal) rather than for elegant theory


pvt void test_sort() {
    void* buf = malloc(10 * sizeof(spaceid_btypeid_t));
    spaceid_btypeid_t *stuff = buf;

    // collate spaceid and btypeid from the types being intersected, expanding intersections but not including any
    // children in the same space as the parent intersection (as that is automatically a failure)
    int count = 6;
    stuff[5].spaceid = 0;  stuff[5].btypeid = 1;
    stuff[4].spaceid = 0;  stuff[4].btypeid = 2;
    stuff[3].spaceid = 1;  stuff[3].btypeid = 3;
    stuff[2].spaceid = 1;  stuff[2].btypeid = 3;
    stuff[1].spaceid = 2;  stuff[1].btypeid = 4;
    stuff[0].spaceid = 2;  stuff[0].btypeid = 5;

    // sort
    _sortBySpaceIdThenBtypeId(stuff, count);

    // check for conflicts
    bool okay = true;  btypeid_t spaceid, btypeid;
    spaceid = stuff[0].spaceid;
    btypeid = stuff[0].btypeid;
    for (int i = 1; i < count; i++) {
        if (stuff[i].spaceid == B_NAT) continue;
        if (stuff[i].spaceid != spaceid) {
            spaceid = stuff[i].spaceid;
            btypeid = stuff[i].btypeid;
        } else {
            if (stuff[i].btypeid != btypeid) {
                okay = false;
                break;
            }
        }
    }

    free(buf);
}



pvt TPN test_minus(BK_MM *mm, Buckets *buckets, BK_TP *tp) {
    // test construction with spaces
    char *s=0;  symid_t *sl;  btypeid_t *tl;  BK_K *k;  BK_TM *tm;  btypeid_t t, t1, t2, t3, t4;

    k = K_create(mm, buckets);  tm = k->tm;
    sl = malloc(11 * sizeof(symid_t));     // 10 items + size field
    tl = malloc(11 * sizeof(btypeid_t));

    // test simple orthogonality
    t3 = tm_bind(tm, "GBP", tm_init_atom(tm, B_NEW, B_NAT, false));
    t2 = tm_bind(tm, "ccy", tm_init_atom(tm, B_NEW, B_NAT, false));
    t1 = tm_bind(tm, "f64", tm_init_atom(tm, B_NEW, B_NAT, false));
    t4 = tm_bind(tm, "u32", tm_init_atom(tm, B_NEW, B_NAT, false));

    t = tm_minus(tm, B_NEW, tm_interv(tm, B_NEW, 3, t1, t2, t3), t2);

    check(t == tm_interv(tm, B_NEW, 2, t1, t3), "t1  &t2 & t3 - t2 != t1 & t3", __FILE__, __LINE__);

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
    PP(debug, "kernel created", __FILE__, __LINE__);

    tm = k->tm;
    sl = malloc(11 * sizeof(symid_t));     // 10 items + size field
    tl = malloc(11 * sizeof(btypeid_t));

    T1 = tm_lookup(tm, "T1");
    T2 = tm_lookup(tm, "T2");

    mem = tm_lookup(tm, "mem");

    null = tm_bind(tm, "null", tm_init_atom(tm, tm_reserve_in(tm, mem), B_NAT, false));
    null2 = tm_bind(tm, "null", tm_init_atom(tm, tm_reserve_in(tm, mem), B_NAT, false));
    check(null2 == B_NAT, "null2 != B_NAT", __FILE__, __LINE__);

    ptrSpc = tm_bind(tm, "ptrSpc", tm_init_atom(tm, B_NEW, B_NAT, false));
    ptr = check_btype(tm, tm_bind(tm, "ptr", tm_init_atom(tm, tm_set_spaceid(tm, B_P, ptrSpc), B_NAT, false)), "ptr == B_NAT", __FILE__, __LINE__);

    constSpc = tm_bind(tm, "constSpc", tm_init_atom(tm, B_NEW, B_NAT, false));
    mut = check_nnat(tm_bind(tm, "mut", tm_init_atom(tm, tm_reserve_in(tm, constSpc), B_NAT, false)), "mut == B_NAT", __FILE__, __LINE__);          // mut is implicit in C but explicit in bones
    const_ = check_nnat(tm_bind(tm, "const", tm_init_atom(tm, tm_reserve_in(tm, constSpc), B_NAT, false)), "const_ == B_NAT", __FILE__, __LINE__);  // const is implicit in bones

    constPtrSpc = tm_bind(tm, "constPtrSpc", tm_init_atom(tm, B_NEW, B_NAT, false));
    mutPtr = tm_bind(tm, "mutPtr", tm_init_atom(tm, tm_reserve_in(tm, constPtrSpc), B_NAT, false));
    constPtr = tm_bind(tm, "constPtr", tm_init_atom(tm, tm_reserve_in(tm, constPtrSpc), B_NAT, false));

    constPtrPtrSpc = tm_bind(tm, "constPtrSpc", tm_init_atom(tm, B_NEW, B_NAT, false));
    mutPtrPtr = tm_bind(tm, "mutPtrPtr", tm_init_atom(tm, tm_reserve_in(tm, constPtrPtrSpc), B_NAT, false));
    constPtrPtr = tm_bind(tm, "constPtrPtr", tm_init_atom(tm, tm_reserve_in(tm, constPtrPtrSpc), B_NAT, false));


    // f64: f64 & mem in mem
    f64 = check_nnat(tm_set_spaceid(tm, tm_set_tbc(tm, B_F64), mem), "f64 == B_NAT");
    check(f64 != 0, "f64 == B_NAT", __FILE__, __LINE__);
    check(tm_lookup(tm, "f64") != f64, "t == %i (should not be %i)", __FILE__, __LINE__, tm_lookup(tm, "f64"), f64);
    check(tm_bmetatypeid(tm, f64) == bmterr, "tm_bmetatypeid(f64) != bmterr", __FILE__, __LINE__);

    // name as
    f64 = tm_bind(tm, "f64", f64);
    check(f64 != 0, "f64 == B_NAT", __FILE__, __LINE__);
    check(tm_lookup(tm, "f64") == f64, "t == %i (should be %i)", __FILE__, __LINE__, tm_lookup(tm, "f64"), f64);
    check(strcmp(tm_s8(tm, tp, f64).cs, "f64") == 0, "pp(f64) != \"f64\" but got \"%s\"", __FILE__, __LINE__, tm_s8(tm, tp, f64).cs);

    // define the recursive type as an intersection
    f64 = tm_interv(tm, f64, 2, f64, mem);       // tm, self, vspace, numTypes, t1, t2...
    check(f64 != 0, "f64 == B_NAT", __FILE__, __LINE__);
    check(tm_bmetatypeid(tm, f64) == bmtint, "tm_bmetatypeid(f64) != bmtint", __FILE__, __LINE__);

    // u32: atm in mem
    u32 = tm_bind(tm, "u32", tm_init_atom(tm, tm_set_spaceid(tm, B_U32, mem), B_NAT, false));

    // u64: atm in mem
    u64 = tm_bind(tm, "u64", tm_init_atom(tm, tm_set_spaceid(tm, B_U64, mem), B_NAT, false));

    // i64: atm in mem
    i64 = check_btype(tm, tm_bind(tm, "i64", tm_init_atom(tm, tm_set_spaceid(tm, B_I64, mem), B_NAT, false)), "i64 is NaT", __FILE__, __LINE__);


    // i64frac: {num:i64, den:i64} in mem
    i64fracStruct = tm_struct(tm, tm_reserve_in(tm, mem),
        sm_slid(k->sm, init_sl(k, sl, 2, "num", "den")),
        tm_tlid_for(tm, init_tl(tl, 2, i64, i64))
    );
    i64frac = check_nnat(tm_bind(tm, "i64frac", i64fracStruct), "i64fracStruct == B_NAT", __FILE__, __LINE__);


    // model ccy and fx such that storage can be f64 or frac
    btypeid_t ccyfx, CCY, ccy, ccyfrac, GBP, GBPFrac, USD;
    ccyfx = tm_bind(tm, "ccyfx", tm_init_atom(tm, B_NEW, B_NAT, false));

    CCY = tm_bind(tm, "CCY", tm_init_atom(tm, tm_reserve_in(tm, ccyfx), B_NAT, false));


    // ccy and ccyfrac are part of the space of CCY
    ccy = tm_bind(tm, "ccy", tm_set_spaceid(tm, tm_reserve_tbc(tm), CCY));
    ccy = tm_interv(tm, ccy, 2, ccy, f64);

    ccyfrac = tm_bind(tm, "ccyfrac", tm_set_spaceid(tm, tm_reserve_tbc(tm), CCY));
    ccyfrac = tm_interv(tm, ccyfrac, 2, ccyfrac, i64frac);
    check_nnat(ccyfrac, "ccyfrac == B_NAT", __FILE__, __LINE__);

    // GBP: GBP & ccy in ccy
    GBP = tm_bind(tm, "GBP", tm_set_spaceid(tm, tm_reserve_tbc(tm), B_NAT));
    GBP = tm_interv_in(tm, GBP, ccy, 2, ccy, GBP);

    // GBPFrac: GBPFrac & ccyfrac in ccyfrac
    GBPFrac = tm_bind(tm, "GBPFrac", tm_set_spaceid(tm, tm_reserve_tbc(tm), B_NAT));
    GBPFrac = tm_interv_in(tm, GBPFrac, ccyfrac, 2, ccyfrac, GBPFrac);

    // USD: USD & ccy in ccy
    USD = tm_bind(tm, "USD", tm_set_spaceid(tm, tm_reserve_tbc(tm), B_NAT));
    USD = tm_interv_in(tm, USD, ccy, 2, ccy, USD);



    // FX: FX & {dom:CCY[T1], for:CCY[T2]}
    btypeid_t FX, fxStruct, fx, fxfrac, GBPUSD, gbpusdStruct;
    FX = tm_bind(tm, "FX", tm_set_spaceid(tm, tm_reserve_tbc(tm), ccyfx));
    fxStruct = tm_struct(tm, B_NEW,
        sm_slid(k->sm, init_sl(k, sl, 2, "dom", "for")),
        tm_tlid_for(tm, init_tl(tl, 2, tm_interv(tm, B_NEW, 2, CCY, T1), tm_interv(tm, B_NEW, 2, CCY, T2)))
    );
    FX = tm_interv(tm, FX, 2, FX, fxStruct);

    fx = tm_bind(tm, "fx", tm_set_spaceid(tm, tm_reserve_tbc(tm), FX));
    fx = tm_interv(tm, fx, 2, fx, f64);

    fxfrac = tm_bind(tm, "fxfrac", tm_set_spaceid(tm, tm_reserve_tbc(tm), FX));
    fxfrac = tm_interv(tm, fxfrac, 2, fxfrac, i64frac);
    check(fxfrac != B_NAT, "fxfrac == B_NAT", __FILE__, __LINE__);


    // GBPUSD: fx & GBPUSD & {dom:GBP, for:USD}

    GBPUSD = tm_bind(tm, "GBPUSD", tm_reserve_tbc(tm));
    gbpusdStruct = tm_struct(tm, B_NEW,
        sm_slid(k->sm, init_sl(k, sl, 2, "dom", "for")),
        tm_tlid_for(tm, init_tl(tl, 2, GBP, USD))
    );
    // IMPORTANT: here {dom:CCY[T1], for:CCY[T2]} & {dom:GBP, for:USD} collapses to {dom:GBP, for:USD} as no residual
    // A & B answers the most concrete bit - so intersections must be done deeply - how about unions?
    // if A <: B then answer A else if B <: A answer B alse answer A & B
    GBPUSD = tm_interv(tm, GBPUSD, 3, fx, GBPUSD, gbpusdStruct);
    check(GBPUSD != B_NAT, "GBPUSD == B_NAT", __FILE__, __LINE__);


    // abstractF64Tree: {lhs: abstractF64Tree + f64 + null, rhs: abstractF64Tree + f64 + null}
    // recursive types do not need to be named
    btypeid_t abstractF64Tree;
    abstractF64Tree = tm_reserve_tbc(tm);
    abstractF64Tree = tm_struct(tm, abstractF64Tree,
        sm_slid(k->sm, init_sl(k, sl, 2, "lhs", "rhs")),
        tm_tlid_for(tm, init_tl(tl, 2,
            tm_unionv(tm, B_NEW, 3, abstractF64Tree, f64, null),
            tm_unionv(tm, B_NEW, 3, abstractF64Tree, f64, null)
        ))
    );
    check(abstractF64Tree != B_NAT, "abstractF64Tree == B_NAT", __FILE__, __LINE__);


    // do a C struct where f64Tree <: abstractF64Tree
    // f64Tree: abstractF64Tree & {lhs: ptr & f64Tree + f64 + null, rhs: ptr & f64Tree + f64 + null}
    // what about:
    // f64Tree: abstractF64Tree & {T1: ptr & f64Tree + f64 + null, T2: ptr & f64Tree + f64 + null}
    btypeid_t f64Tree;
    f64Tree = tm_bind(tm, "f64Tree", tm_reserve_tbc(tm));
    f64Tree = tm_interv(tm, f64Tree, 4,
        mem,
        f64Tree,
        abstractF64Tree,
        tm_struct(tm, B_NEW,
            sm_slid(k->sm, init_sl(k, sl, 2, "lhs", "rhs")),
            tm_tlid_for(tm, init_tl(tl, 2,
                tm_unionv(tm, B_NEW, 3, tm_interv(tm, B_NEW, 2, ptr, f64Tree), f64, null),
                tm_unionv(tm, B_NEW, 3, tm_interv(tm, B_NEW, 2, ptr, f64Tree), f64, null)
            )
        )
    ));
    check_btype(tm, f64Tree, "f64Tree", __FILE__, __LINE__);


    // do a tree[T1] such that f64Tree <: tree[T1]
    // tree: {lhs: tree[T1] + T1 + null, rhs: tree[T1] + T1 + null}
    btypeid_t tree;
    tree = tm_bind(tm, "tree", tm_reserve_tbc(tm));
    tree = tm_struct(tm, tree,
        sm_slid(k->sm, init_sl(k, sl, 2, "lhs", "rhs")),
        tm_tlid_for(tm, init_tl(tl, 2,
            tm_unionv(tm, B_NEW, 3, tm_interv(tm, B_NEW, 2, tree, T1), T1, null),
            tm_unionv(tm, B_NEW, 3, tm_interv(tm, B_NEW, 2, tree, T1), T1, null)
        ))
    );


    PP(debug, "u32, u64, ccy, GBP, USD: %i, %i, %i, %i, %i", tm_lookup(tm, "u32"), tm_lookup(tm, "u64"), tm_lookup(tm, "ccy"), tm_lookup(tm, "GBP"), tm_lookup(tm, "USD"));

    // check construction returns identical objects
    t7 = check_btype(tm, tm_interv(tm, B_NEW, 2, CCY, u32), "t7", __FILE__, __LINE__);
    t8 = check_btype(tm, tm_interv(tm, B_NEW, 2, CCY, u32), "t8", __FILE__, __LINE__);
    check(t7 == t8, "t7 != t8 got %i and %i", __FILE__, __LINE__, t7, t8);

    // check u32 doesn't mix with u64
    actual = tm_interv(tm, B_NEW, 2, u32, u64);
    check(actual == 0, "actual == %i (should be %i)", __FILE__, __LINE__, actual, 0);

    // check u32 doesn't mix with u64 - nested
    actual = tm_interv(tm, B_NEW, 2, GBP, USD);
    check(actual == 0, "actual == %i (should be %i)", __FILE__, __LINE__, actual, 0);


    // need to rethink tests here - modelling GBP etc correctly is getting in way of writing clear tests
//    PP(info, "u32 & ccy: %i, (%s)", t7, tm_s8_typelist(tm, tp, tm_inter_tl(tm, t7)).cs);
//    t8 = tm_interv(tm, B_NEW, 2, u64, ccy);
//    PP(info, "u64 & ccy: %i, (%s)", t8, tm_s8_typelist(tm, tp, tm_inter_tl(tm, t8)).cs);
//    actual = tm_interv(tm, B_NEW, 2, t7, t8);
//    if (actual != 0) s = tm_s8_typelist(tm, tp, tm_inter_tl(tm, actual)).cs;
//    check(actual == 0, "t == %i (should be %i) - %s", __FILE__, __LINE__, actual, s);
//
//    // check GBP mixes with USD
//    actual = tm_interv(tm, B_NEW, 2, GBP, USD);
//    check(actual != 0, "t == %i (should not be %i)", __FILE__, __LINE__, actual, 0);
//    t = tm_interv(tm, B_NEW, 2, GBP, u32);
//    actual = tm_interv(tm, B_NEW, 2, t, USD);
//    check(actual != 0, "t == %i (should not be %i)", __FILE__, __LINE__, actual, 0);


    free(tl);  free(sl);  K_trash(k);
    return tp_tpn_printf(tp, "test_construction_extended() passed");
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
    test_sort();
    PP(info, tp_s8(&tp, debug5(mm, &buckets, &tp)).cs);
    PP(info, tp_s8(&tp, debug4(mm, &buckets, &tp)).cs);
    PP(info, tp_s8(&tp, debug3(mm, &buckets, &tp)).cs);
    PP(info, tp_s8(&tp, debug2(mm, &buckets, &tp)).cs);
    PP(info, tp_s8(&tp, test_construction(mm, &buckets, &tp)).cs);
    PP(info, tp_s8(&tp, test_orthogonal_spaces(mm, &buckets, &tp)).cs);
    PP(info, tp_s8(&tp, test_construction_extended(mm, &buckets, &tp)).cs);
    PP(info, tp_s8(&tp, test_minus(mm, &buckets, &tp)).cs);
    PP(info, "passed");
    Buckets_finalise(&buckets);
    MM_trash(mm);
    return 0;
}
