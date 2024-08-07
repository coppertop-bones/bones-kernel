#include "../bk/pp.c"
#include "../bk/k.c"

pvt void die_(char *preamble, char *msg, va_list args) {
    fprintf(stdout, "%s", preamble);
    vfprintf(stdout, msg, args);
    fprintf(stdout, "\n");
    exit(1);
}


#define countArgs(x, y...)  countArgsImpl((x) , ## y, 0)


int countArgsImpl(int x, ...) {
    va_list ap;  int e;  int count = 1;
    va_start(ap, x);
    while ((e = va_arg(ap, int)) != 0) count ++;
    va_end(ap);
    return count;
}


void test_tm() {
    btypeid_t t, t2, expected;
    BK_MM *mm = MM_create();
    Buckets buckets;
    initBuckets(&buckets, 64);
    BK_K *k = K_create(mm, &buckets);

    PP(debug, "kernel created");

    t = tm_btypeid(k->tm, "joe");
    check(t == 0, "%s @ %i: id == %i (should be %i)", __FILE__, __LINE__, t, 0);

    t = tm_btypeid(k->tm, "sally");
    check(t == 0, "%s @ %i: id == %i (should be %i)", __FILE__, __LINE__, t, 0);

    expected =  B_EXTERN_FN_PTR + 1;
    t = tm_nominal(k->tm, "fred", expected);
    check(t == expected, "%s @ %i: t == %i (should be %i)", __FILE__, __LINE__, t, expected);

    t = tm_nominal(k->tm, "joe", 0);
    expected++;
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
    t = tm_inter(k->tm, typelist, 0);
    expected ++;
    check(t == expected, "%s @ %i: t == %i (should be %i)", __FILE__, __LINE__, t, expected);

    t = tm_inter(k->tm, typelist, 0);
    check(t == expected, "%s @ %i: t == %i (should be %i)", __FILE__, __LINE__, t, expected);


    // union
    t = tm_union(k->tm, typelist, 0);
    expected ++;
    check(t == expected, "%s @ %i: t == %i (should be %i)", __FILE__, __LINE__, t, expected);

    t2 = tm_union(k->tm, typelist, 0);
    check(t2 == expected, "%s @ %i: t == %i (should be %i)", __FILE__, __LINE__, t2, expected);


    // sequence
    t = tm_seq(k->tm, t2, 0);
    expected ++;
    check(t == expected, "%s @ %i: t == %i (should be %i)", __FILE__, __LINE__, t, expected);

    t = tm_seq(k->tm, t2, 0);
    check(t == expected, "%s @ %i: t == %i (should be %i)", __FILE__, __LINE__, t, expected);


    // tuple
    typelist[0] = 0;
    t = tm_tuple(k->tm, typelist, 0);
    expected ++;
    check(t == expected, "%s @ %i: t == %i (should be %i)", __FILE__, __LINE__, t, expected);

    typelist[0] = 2;
    t = tm_tuple(k->tm, typelist, 0);
    expected ++;
    check(t == expected, "%s @ %i: t == %i (should be %i)", __FILE__, __LINE__, t, expected);


    // struct
    SM_SLID_T slid = sm_slid(k->sm, symlist);
    btypeid_t tupid = tm_tuple(k->tm, typelist, 0);

    t = tm_struct(k->tm, slid, tupid, 0);
    expected ++;
    check(t == expected, "%s @ %i: t == %i (should be %i)", __FILE__, __LINE__, t, expected);

    t = tm_struct(k->tm, slid, tupid, 0);
    check(t == expected, "%s @ %i: t == %i (should be %i)", __FILE__, __LINE__, t, expected);


    K_trash(k);
    freeBuckets(buckets.first_bucket);
    MM_trash(mm);
}



void test_exclusions() {
    btypeid_t tCcy, u32, u64, t, t7, t8, GBP, EUR, *tl, actual, _GBP;  BK_TP tp;  char *s=0;
    BK_MM *mm = MM_create();
    Buckets buckets;
    initBuckets(&buckets, 64);
    BK_K *k = K_create(mm, &buckets);
    BK_TM *tm = k->tm;
    TP_init(&tp, 0, &buckets);
    S8 txt = tp_s8(&tp, tp_pp_printf(&tp, "kernel created"));
    PP(debug, txt.cs);

    _GBP = tm_nominal(tm, "_GBP", 100);
    u32 = tm_exclnominal(tm, "u32", btememory, 0, 0);
    u64 = tm_exclnominal(tm, "u64", btememory, 0, 0);
    tCcy = tm_exclnominal(tm, "ccy", bteuser1, 0, 0);


    GBP = tm_interv(tm, 2, tCcy, tm_nominal(tm, "_GBP", 0));
    check(GBP != 0, "%s @ %i: t == %i (should not be %i)", __FILE__, __LINE__, GBP, 0);

    EUR = tm_interv(tm, 2, tCcy, tm_nominal(tm, "_EUR", 0));
    check(EUR != 0, "%s @ %i: t == %i (should not be %i)", __FILE__, __LINE__, EUR, 0);

    PP(debug, "u32, u64, ccy, _GBP, _EUR: %i, %i, %i, %i, %i", tm_btypeid(tm, "u32"), tm_btypeid(tm, "u64"), tm_btypeid(tm, "ccy"), tm_btypeid(tm, "_GBP"), tm_btypeid(tm, "_EUR"));

    // name as
    check(tm_btypeid(tm, "EUR") != EUR, "%s @ %i: t == %i (should not be %i)", __FILE__, __LINE__, tm_btypeid(tm, "EUR"), EUR);
    tm_name_as(tm, EUR, "EUR");
    check(tm_btypeid(tm, "EUR") == EUR, "%s @ %i: t == %i (should be %i)", __FILE__, __LINE__, tm_btypeid(tm, "EUR"), EUR);
    check(strcmp(tm_s8(tm, &tp, EUR).cs, "EUR") == 0, "%s @ %i: pp(EUR) != \"EUR\" but got \"%s\"", __FILE__, __LINE__, tm_s8(tm, &tp, EUR).cs);

    // check construction returns identical objects
    t7 = tm_interv(tm, 2, GBP, u32, 0);
    t8 = tm_interv(tm, 2, GBP, u32, 0);
    check(t7 == t8, "%s @ %i: t7 != t8 got %i and %i", __FILE__, __LINE__, t7, t8);

    // check u32 doesn't mix with u64
    actual = tm_interv(tm, 2, u32, u64, 0);
    check(actual == 0, "%s @ %i: t == %i (should be %i)", __FILE__, __LINE__, actual, 0);

    // check u32 doesn't mix with u64 - nested
    t7 = tm_interv(tm, 2, u32, tCcy, 0);
    PP(debug, "u32 & ccy: %i, %s", t7, tm_s8_typelist(tm, &tp, tm_inter_tl(tm, t7)).cs);
    t8 = tm_interv(tm, 2, u64, tCcy, 0);
    PP(debug, "u64 & ccy: %i, %s", t8, tm_s8_typelist(tm, &tp, tm_inter_tl(tm, t8)).cs);
    actual = tm_interv(tm, 2, t7, t8, 0);
    if (actual != 0) s = tm_s8_typelist(tm, &tp, tm_inter_tl(tm, actual)).cs;
    check(actual == 0, "%s @ %i: t == %i (should be %i) - %s", __FILE__, __LINE__, actual, 0, s);

    // check GBP mixes with EUR
    actual = tm_interv(tm, 2, GBP, EUR, 0);
    check(actual != 0, "%s @ %i: t == %i (should not be %i)", __FILE__, __LINE__, actual, 0);
    t = tm_interv(tm, 2, GBP, u32, 0);
    actual = tm_interv(tm, 2, t, EUR, 0);
    check(actual != 0, "%s @ %i: t == %i (should not be %i)", __FILE__, __LINE__, actual, 0);

    TP_free(&tp);
    K_trash(k);
    freeBuckets(buckets.first_bucket);
    MM_trash(mm);
}


int main() {
    g_logging_level = debug;

    Buckets buckets; BK_TP tp;
    initBuckets(&buckets, 64);
    TP_init(&tp, 0, &buckets);


//    TPN fred = (TPN) {.p = cv._tp->buf + start, .vtsz = tp_encode_as_s8((end - start))};
    S8 txt = tp_s8(&tp, tp_pp_printf(&tp, "kernel created"));


    PP(debug, "%i", countArgs(1));
    PP(debug, "%i", countArgs(1,2));
    PP(debug, "%i", os_cache_line_size());
    PP(debug, "%i", os_page_size());
    test_tm();
    test_exclusions();
    PP(info, "passed");
    return 0;
}
