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

pvt btypeid_t * init_tl(btypeid_t * tl, int num, ...) {
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



pvt TPN test_construction(BK_MM *mm, Buckets *buckets, BK_TP *tp) {
    btypeid_t t, t2, t3, expected;
    BK_K *k = K_create(mm, buckets);

    PP(debug, "kernel created");


    // atom
    t = tm_btypeid(k->tm, "joe");
    check(t == 0, "%s @ %i: id != B_NAT (should be %i)", __FILE__, __LINE__, t);

    t = tm_btypeid(k->tm, "sally");
    check(t == 0, "%s @ %i: id != B_NAT (should be %i)", __FILE__, __LINE__, t);

    expected =  B_EXTERN_FN_PTR;
    t = tm_atom(k->tm, expected, "fred");
    check(t == expected, "%s @ %i: t == %i (should be %i)", __FILE__, __LINE__, t, expected);

    expected = B_FIRST_UNRESERVED_TYPEID;
    t = tm_atom(k->tm, B_NEW, "joe");

    check(t == expected, "%s @ %i: t == %i (should be %i)", __FILE__, __LINE__, t, expected);
    t = tm_btypeid(k->tm, "joe");
    check(t == expected, "%s @ %i: t == %i (should be %i)", __FILE__, __LINE__, t, expected);

    btypeid_t *typelist = malloc(3 * sizeof(btypeid_t));
    symid_t *symlist = malloc(3 * sizeof(symid_t));
    typelist[0] = 2;
    typelist[1] = tm_btypeid(k->tm, "fred");
    typelist[2] = tm_btypeid(k->tm, "joe");
    symlist[0] = 2;
    symlist[1] = sm_id(k->sm, "fred");
    symlist[2] = sm_id(k->sm, "joe");


    // intersection
    t = tm_inter(k->tm, B_NEW, typelist);
    expected ++;
    check(t == expected, "%s @ %i: t == %i (should be %i)", __FILE__, __LINE__, t, expected);

    t = tm_inter(k->tm, B_NEW, typelist);
    check(t == expected, "%s @ %i: t == %i (should be %i)", __FILE__, __LINE__, t, expected);


    // union
    t = tm_union(k->tm, B_NEW, typelist);
    expected ++;
    check(t == expected, "%s @ %i: t == %i (should be %i)", __FILE__, __LINE__, t, expected);

    t2 = tm_union(k->tm, B_NEW, typelist);
    check(t2 == expected, "%s @ %i: t == %i (should be %i)", __FILE__, __LINE__, t2, expected);


    // tuple
    typelist[0] = 0;
    t = tm_tuple(k->tm, B_NEW, typelist);
    expected ++;
    check(t == expected, "%s @ %i: t == %i (should be %i)", __FILE__, __LINE__, t, expected);

    typelist[0] = 2;
    t = tm_tuple(k->tm, B_NEW, typelist);
    expected ++;
    check(t == expected, "%s @ %i: t == %i (should be %i)", __FILE__, __LINE__, t, expected);


    // struct
    SM_SLID_T slid = sm_slid(k->sm, symlist);
    btypeid_t tupid = tm_tuple(k->tm, B_NEW, typelist);

    t = tm_struct(k->tm, B_NEW, slid, tupid);
    expected ++;
    check(t == expected, "%s @ %i: t == %i (should be %i)", __FILE__, __LINE__, t, expected);

    t = tm_struct(k->tm, B_NEW, slid, tupid);
    check(t == expected, "%s @ %i: t == %i (should be %i)", __FILE__, __LINE__, t, expected);


     // sequence
    t = tm_seq(k->tm, B_NEW, t2);
    expected ++;
    check(t == expected, "%s @ %i: t == %i (should be %i)", __FILE__, __LINE__, t, expected);

    t = tm_seq(k->tm, B_NEW, t2);
    check(t == expected, "%s @ %i: t == %i (should be %i)", __FILE__, __LINE__, t, expected);

    t3 = tm_seq_t(k->tm, t);
    check(t3 == t2, "%s @ %i: t3 == %i (should be %i)", __FILE__, __LINE__, t3, t2);


    // map


    // fn


    // schemavar
    // done in kernel creation


    K_trash(k);

    return tp_tpn_printf(tp, "test_construction() passed");
}



pvt TPN test_construction_extended(BK_MM *mm, Buckets *buckets, BK_TP *tp) {
    // test construction with spaces, recursive, explicit and implicit in

    char *s=0;  symid_t *sl;  btypeid_t *tl;  BK_K *k;  BK_TM *tm;
    btypeid_t T1, T2, mem, null, ptrSpc, ptr, ptrptr, constSpc, mut, const_, constPtrSpc, mutPtr, constPtr;
    btypeid_t constPtrPtrSpc, mutPtrPtr, constPtrPtr;
    btypeid_t f64, u32, u64, i64, i64fracStruct, i64frac, t, t7, t8, actual;

    k = K_create(mm, buckets);
    PP(debug, "%s @ %i: kernel created", __FILE__, __LINE__);

    tm = k->tm;
    sl = malloc(11 * sizeof(symid_t));     // 10 items + size field
    tl = malloc(11 * sizeof(btypeid_t));

    T1 = tm_schemavar(tm, B_NEW, "T1");
    T2 = tm_schemavar(tm, B_NEW, "T2");

    mem = tm_atom(tm, B_NEW, "mem");        // mem is a built-in type?

    null = tm_atom(tm, tm_new(tm, B_NEW, 0, mem, 0, 0), "null");

    ptrSpc = tm_atom(tm, B_NEW, "ptrSpc");
    ptr = check_nnat(tm_atom(tm, tm_new(tm, B_NEW, 0, ptrSpc, 0, 0), "ptr"), "ptr == B_NAT");
    ptrptr = tm_atom(tm, tm_new(tm, B_NEW, 0, ptrSpc, 0 ,0), "ptrptr");

    constSpc = tm_atom(tm, B_NEW, "constSpc");
    mut = check_nnat(tm_atom(tm, tm_new(tm, B_NEW, 0, constSpc, true, 0), "mut"), "%s @ %i: mut == B_NAT", __FILE__, __LINE__);          // mut is implicit in C but explicit in bones
    const_ = check_nnat(tm_atom(tm, tm_new(tm, B_NEW, 0, constSpc, 0, true), "const"), "%s @ %i: const_ == B_NAT", __FILE__, __LINE__);  // const is implicit in bones

    constPtrSpc = tm_atom(tm, B_NEW, "constPtrSpc");
    mutPtr = tm_atom(tm, tm_new(tm, B_NEW, 0, constPtrSpc, true, 0), "mutPtr");
    constPtr = tm_atom(tm, tm_new(tm, B_NEW, 0, constPtrSpc, 0, true), "constPtr");

    constPtrPtrSpc = tm_atom(tm, B_NEW, "constPtrSpc");
    mutPtrPtr = tm_atom(tm, tm_new(tm, B_NEW, 0, constPtrPtrSpc, true, 0), "mutPtrPtr");
    constPtrPtr = tm_atom(tm, tm_new(tm, B_NEW, 0, constPtrPtrSpc, 0, true), "constPtrPtr");


    // f64 is mem & itself named as 'f64'
    // create recursive type
    f64 = check_nnat(tm_new(tm, B_NEW, true, 0, 0, 0), "f64 == B_NAT");
    check(f64 != 0, "%s @ %i: f64 == B_NAT", __FILE__, __LINE__);
    check(tm_btypeid(tm, "f64") != f64, "%s @ %i: t == %i (should not be %i)", __FILE__, __LINE__, tm_btypeid(tm, "f64"), f64);
    check(tm_bmetatypeid(tm, f64) == bmterr, "%s @ %i: tm_bmetatypeid(f64) != bmterr", __FILE__, __LINE__);

    // name as
    f64 = tm_name_as(tm, f64, "f64");
    check(f64 != 0, "%s @ %i: f64 == B_NAT", __FILE__, __LINE__);
    check(tm_btypeid(tm, "f64") == f64, "%s @ %i: t == %i (should be %i)", __FILE__, __LINE__, tm_btypeid(tm, "f64"), f64);
    check(strcmp(tm_s8(tm, tp, f64).cs, "f64") == 0, "%s @ %i: pp(f64) != \"f64\" but got \"%s\"", __FILE__, __LINE__, tm_s8(tm, tp, f64).cs);

    // define the recursive type as an intersection
    f64 = tm_inter_v(tm, f64, 2, f64, mem);       // tm, self, vspace, numTypes, t1, t2...
    check(f64 != 0, "%s @ %i: f64 == B_NAT", __FILE__, __LINE__);
    check(tm_bmetatypeid(tm, f64) == bmtint, "%s @ %i: tm_bmetatypeid(f64) != bmtint", __FILE__, __LINE__);


    u32 = tm_name_as(tm, tm_new(tm, B_NEW, true, 0, 0, 0), "u32");
    u32 = tm_inter_v(tm, u32, 2, mem, u32);

    u64 = tm_name_as(tm, tm_new(tm, B_NEW, true, 0, 0, 0), "u64");
    u64 = tm_inter_v(tm, u64, 2, mem, u64);

    i64 = tm_name_as(tm, tm_new(tm, B_NEW, true, 0, 0, 0), "i64");
    i64 = tm_inter_v(tm, i64, 2, mem, i64);


    // i64frac: mem & {num:i64, den:i64}
    i64fracStruct = tm_struct(tm, B_NEW,
        sm_slid(k->sm, init_sl(k, sl, 2, "num", "den")),
        tm_tuple(tm, B_NEW, init_tl(tl, 2, i64, i64))
    );
    i64frac = tm_name_as(tm, tm_inter_v(tm, B_NEW, 2, mem, i64fracStruct), "i64frac");


    // model ccy and fx such that storage can be f64 or frac
    btypeid_t ccyfx, CCY, ccy, ccyfrac, GBP, GBPFrac, USD;
    ccyfx = tm_atom(tm, B_NEW, "ccyfx");

    CCY = tm_atom(tm, tm_new(tm, B_NEW, 0, ccyfx, 0, 0), "CCY");


    // ccy and ccy frac are part of the space of CCY
    ccy = tm_name_as(tm, check_nnat(tm_new(tm, B_NEW, true, CCY, 0, 0), "tm_recur_in(...) == B_NAT"), "ccy");
    ccy = tm_inter_v(tm, ccy, 2, ccy, f64);

    ccyfrac = tm_name_as(tm, tm_new(tm, B_NEW, true, CCY, 0, 0), "ccyfrac");
    ccyfrac = tm_inter_v(tm, ccyfrac, 2, ccyfrac, i64frac);

    GBP = tm_name_as(tm, tm_new(tm, B_NEW, true, 0, 0, 0), "GBP");
    GBP = tm_inter_v(tm, GBP, 2, ccy, GBP);

    GBPFrac = tm_name_as(tm, tm_new(tm, B_NEW, true, 0, 0, 0), "GBPFrac");
    GBPFrac = tm_inter_v(tm, GBPFrac, 2, ccyfrac, GBPFrac);

    USD = tm_name_as(tm, tm_new(tm, B_NEW, true, 0, 0, 0), "USD");
    USD = tm_inter_v(tm, USD, 2, ccy, USD);


    // FX: FX & {dom:CCY[T1], for:CCY[T2]}
    btypeid_t FX, fxStruct, fx, fxfrac, GBPUSD, gbpusdStruct;
    FX = tm_name_as(tm, tm_new(tm, B_NEW, true, ccyfx, 0, 0), "FX");
    fxStruct = tm_struct(tm, B_NEW,
        sm_slid(k->sm, init_sl(k, sl, 2, "dom", "for")),
        tm_tuple(tm, B_NEW, init_tl(tl, 2, tm_inter_v(tm, B_NEW, 2, CCY, T1), tm_inter_v(tm, B_NEW, 2, CCY, T2)))
    );
    FX = tm_inter_v(tm, FX, 2, FX, fxStruct);

    fx = tm_name_as(tm, tm_new(tm, B_NEW, true, FX, 0, 0), "fx");
    fx = tm_inter_v(tm, fx, 2, fx, f64);

    fxfrac = tm_name_as(tm, tm_new(tm, B_NEW, true, FX, 0, 0), "fxfrac");
    fxfrac = tm_inter_v(tm, fxfrac, 2, fxfrac, i64frac);


    // GBPUSD: fx & {dom:GBP, for:USD}

    GBPUSD = tm_name_as(tm, tm_new(tm, B_NEW, true, 0, 0, 0), "GBPUSD");
    gbpusdStruct = tm_struct(tm, B_NEW,
        sm_slid(k->sm, init_sl(k, sl, 2, "dom", "for")),
        tm_tuple(tm, B_NEW, init_tl(tl, 2, GBP, USD))
    );
    // IMPORTANT: here {dom:CCY[T1], for:CCY[T2]} & {dom:GBP, for:USD} collapses to {dom:GBP, for:USD} as no residual
    // A & B answers the most concrete bit - so intersections must be done deeply - how about unions?
    // if A <: B then answer A else if B <: A answer B alse answer A & B
    GBPUSD = tm_inter_v(tm, GBPUSD, 3, fx, GBPUSD, gbpusdStruct);


    // abstractF64Tree: {lhs: abstractF64Tree + f64 + null, rhs: abstractF64Tree + f64 + null}
    // recursive types do not need to be named
    btypeid_t abstractF64Tree;
    abstractF64Tree = tm_new(tm, B_NEW, true, 0, 0, 0);
    abstractF64Tree = tm_struct(tm, abstractF64Tree,
        sm_slid(k->sm, init_sl(k, sl, 2, "lhs", "rhs")),
        tm_tuple(tm, B_NEW, init_tl(tl, 2,
            tm_union_v(tm, 3, abstractF64Tree, f64, null),
            tm_union_v(tm, 3, abstractF64Tree, f64, null)
        ))
    );


    // do a C struct where f64Tree <: abstractF64Tree
    // f64Tree: abstractF64Tree & {lhs: ptr & f64Tree + f64 + null, rhs: ptr & f64Tree + f64 + null}
    // what about:
    // f64Tree: abstractF64Tree & {T1: ptr & f64Tree + f64 + null, T2: ptr & f64Tree + f64 + null}
    btypeid_t f64Tree;
    f64Tree = tm_name_as(tm, tm_new(tm, B_NEW, true, 0, 0, 0), "f64Tree");
    f64Tree = tm_inter_v(tm, f64Tree, 4,
        mem,
        f64Tree,
        abstractF64Tree,
        tm_struct(tm, B_NEW,
            sm_slid(k->sm, init_sl(k, sl, 2, "lhs", "rhs")),
            tm_tuple(tm, B_NEW, init_tl(tl, 2,
                tm_union_v(tm, 3, tm_inter_v(tm, B_NEW, 2, ptr, f64Tree), f64, null),
                tm_union_v(tm, 3, tm_inter_v(tm, B_NEW, 2, ptr, f64Tree), f64, null)
            )
        )
    ));


    // do a tree[T1] such that f64Tree <: tree[T1]
    // tree: {lhs: tree[T1] + T1 + null, rhs: tree[T1] + T1 + null}
    btypeid_t tree;
    tree = tm_name_as(tm, tm_new(tm, B_NEW, true, 0, 0, 0), "tree");
    tree = tm_struct(tm, tree,
        sm_slid(k->sm, init_sl(k, sl, 2, "lhs", "rhs")),
        tm_tuple(tm, B_NEW, init_tl(tl, 2,
            tm_union_v(tm, 3, tm_inter_v(tm, B_NEW, 2, tree, T1), T1, null),
            tm_union_v(tm, 3, tm_inter_v(tm, B_NEW, 2, tree, T1), T1, null)
        ))
    );





    PP(debug, "u32, u64, ccy, GBP, USD: %i, %i, %i, %i, %i", tm_btypeid(tm, "u32"), tm_btypeid(tm, "u64"), tm_btypeid(tm, "ccy"), tm_btypeid(tm, "GBP"), tm_btypeid(tm, "USD"));

    // check construction returns identical objects
    t7 = tm_inter_v(tm, B_NEW, 2, GBP, u32);
    t8 = tm_inter_v(tm, B_NEW, 2, GBP, u32);
    check(t7 == t8, "%s @ %i: t7 != t8 got %i and %i", __FILE__, __LINE__, t7, t8);

    // check u32 doesn't mix with u64
    actual = tm_inter_v(tm, B_NEW, 2, u32, u64);
    check(actual == 0, "%s @ %i: t == %i (should be %i)", __FILE__, __LINE__, actual, 0);

    // check u32 doesn't mix with u64 - nested
    t7 = tm_inter_v(tm, B_NEW, 2, u32, ccy);
    PP(debug, "u32 & ccy: %i, %s", t7, tm_s8_typelist(tm, tp, tm_inter_tl(tm, t7)).cs);
    t8 = tm_inter_v(tm, B_NEW, 2, u64, ccy);
    PP(debug, "u64 & ccy: %i, %s", t8, tm_s8_typelist(tm, tp, tm_inter_tl(tm, t8)).cs);
    actual = tm_inter_v(tm, B_NEW, 2, t7, t8);
    if (actual != 0) s = tm_s8_typelist(tm, tp, tm_inter_tl(tm, actual)).cs;
    check(actual == 0, "%s @ %i: t == %i (should be %i) - %s", __FILE__, __LINE__, actual, s);

    // check GBP mixes with USD
    actual = tm_inter_v(tm, B_NEW, 2, GBP, USD);
    check(actual != 0, "%s @ %i: t == %i (should not be %i)", __FILE__, __LINE__, actual, 0);
    t = tm_inter_v(tm, B_NEW, 2, GBP, u32);
    actual = tm_inter_v(tm, B_NEW, 2, t, USD);
    check(actual != 0, "%s @ %i: t == %i (should not be %i)", __FILE__, __LINE__, actual, 0);


    free(tl);
    free(sl);


    K_trash(k);

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
    PP(info, tp_s8(&tp, test_construction_extended(mm, &buckets, &tp)).cs);
    PP(debug, "passed");
    Buckets_finalise(&buckets);
    MM_trash(mm);
    return 0;
}
