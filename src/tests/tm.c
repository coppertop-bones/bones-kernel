#include "../bk/pp.c"
#include "../bk/k.c"
#include "../bk/tp.c"

pvt void die_(char *preamble, char *msg, va_list args) {
    fprintf(stdout, "%s", preamble);
    vfprintf(stdout, msg, args);
    fprintf(stdout, "\n");
    exit(1);
}

pub btypeid_t _intersect(BK_TM *tm, size_t numTypes, ...) {
    va_list args;  btypeid_t *typelist;  int i;  btypeid_t btypeid;
    va_start(args, numTypes);
    typelist = malloc((1 + numTypes) * sizeof(btypeid_t));
    for (i = 1; i <= numTypes; i++) typelist[i] = va_arg(args, btypeid_t);
    typelist[0] = numTypes;
    btypeid = tm_inter(tm, typelist);
    free(typelist);
    va_end(args);
    return btypeid;
}

pub btypeid_t _intersect2(BK_TM *tm, size_t numTypes, btypeid_t *args) {
    btypeid_t *typelist;  int i;  btypeid_t btypeid;
    typelist = malloc((1 + numTypes) * sizeof(btypeid_t));
    for (i = 1; i <= numTypes; i++) typelist[i] = args[i-1];
    typelist[0] = numTypes;
    btypeid = tm_inter(tm, typelist);
    free(typelist);
    return btypeid;
}

#define intersect(tm, ...) ({                                                                                           \
    btypeid_t args[] = { __VA_ARGS__ };                                                                                 \
    _intersect2((tm), sizeof(args) / sizeof(args[0]), args);                                                            \
})

#define countArgs(x, y...)  countArgsImpl((x) , ## y, 0)


int countArgsImpl(int x, ...) {
    va_list ap;  int e;  int count = 1;
    va_start(ap, x);
    while ((e = va_arg(ap, int)) != 0) count ++;
    va_end(ap);
    return count;
}


void test_tm() {
    btypeid_t t, expected;
    BK_MM *mm = MM_create();
    Buckets buckets;
    initBuckets(&buckets, 64);
    BK_K *k = K_create(mm, &buckets);

    PP(debug, "kernel created");

    t = tm_btypeid(k->tm, "joe");
    check(t == 0, "id == %i (should be 0) @ %i", t, __LINE__);

    t = tm_btypeid(k->tm, "sally");
    check(t == 0, "id == %i (should be 0) @ %i", t, __LINE__);

    expected =  k->tm->next_btypeId;
    t = tm_nominal(k->tm, "fred");
    check(t == expected, "t == %i (should be %i)", t, expected);
    t = tm_nominal(k->tm, "joe");
    expected++;
    check(t == expected, "t == %i (should be %i)", t, expected);
    t = tm_btypeid(k->tm, "joe");
    check(t == expected, "t == %i (should be %i)", t, expected);

    btypeid_t *typelist = malloc(3 * sizeof(btypeid_t));
    typelist[0] = 2;
    typelist[1] = 1;
    typelist[2] = 2;
    t = tm_inter(k->tm, typelist);
    expected ++;
    check(t == expected, "t == %i (should be %i)", t, expected);

    t = tm_inter(k->tm, typelist);
    check(t == expected, "t == %i (should be %i)", t, expected);

    K_trash(k);
    freeBuckets(buckets.first_bucket);
    MM_trash(mm);
}



void test_exclusions() {
    btypeid_t tCcy, u32, u64, t7, t8, GBP, EUR, *tl, actual, _GBP;  BK_TP tp;
    BK_MM *mm = MM_create();
    Buckets buckets;
    initBuckets(&buckets, 64);
    BK_K *k = K_create(mm, &buckets);
    BK_TM *tm = k->tm;
    TP_init(&tp, 0, &buckets);
    S8 txt = tp_render(&tp, tp_printftp(&tp, "kernel created"));
    PP(debug, txt.cs);

    _GBP = tm_nominal(tm, "_GBP");
    u32 = tm_exclnominal(tm, "u32", btememory, 0, 0);
    u64 = tm_exclnominal(tm, "u64", btememory, 0, 0);
    tCcy = tm_exclnominal(tm, "ccy", bteuser1, 0, 0);

    GBP = intersect(tm, tCcy, tm_nominal(tm, "_GBP"));
    check(GBP != 0, "t == %i (should not be %i)", GBP, 0);

    EUR = intersect(tm, tCcy, tm_nominal(tm, "_EUR"));
    check(EUR != 0, "t == %i (should not be %i)", EUR, 0);

    // name as
    check(tm_btypeid(tm, "EUR") != EUR, "t == %i (should not be %i) @ %i", tm_btypeid(tm, "EUR"), EUR, __LINE__);
    tm_name_as(tm, EUR, "EUR");
    check(tm_btypeid(tm, "EUR") == EUR, "t == %i (should be %i) @ %i", tm_btypeid(tm, "EUR"), EUR, __LINE__);
    check(strcmp(tm_pp(tm, &tp, EUR).cs, "EUR") == 0, "pp(EUR) != \"EUR\" but got \"%s\" @ %i", tm_pp(tm, &tp, EUR).cs, __LINE__);

    // check construction returns identical objects
    t7 = intersect(tm, GBP, u32);
    t8 = intersect(tm, GBP, u32);
    check(t7 == t8, "t7 != t8 got %i and %i @ %i", t7, t8, __LINE__);

    // check u32 doesn't mix with u64
    actual = intersect(tm, intersect(tm, u32, tCcy), intersect(tm, u64, tCcy));
    check(actual == 0, "t == %i (should be %i)", actual, 0);

    // check GBP mixes with EUR
    actual = intersect(tm, GBP, EUR);
    check(actual != 0, "t == %i (should not be %i) @ %i", actual, 0, __LINE__);

    actual = intersect(tm, intersect(tm, GBP, u32), EUR);
    check(actual != 0, "t == %i (should not be %i) @ %i", actual, 0, __LINE__);

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


    TPN fred = (TPN) {.p = cv._tp->buf + start, .opaque = tp_as_s8((end - start))};
    S8 txt = tp_render(&tp, tp_printftp(&tp, "kernel created"));


    PP(debug, "%i", countArgs(1));
    PP(debug, "%i", countArgs(1,2));
    PP(debug, "%i", os_cache_line_size());
    PP(debug, "%i", os_page_size());
    test_tm();
    test_exclusions();
    PP(info, "passed");
    return 0;
}
