#include "../bk/pp.c"
#include "../bk/kernel.c"

pvt void die_(char *preamble, char *msg, va_list args) {
    fprintf(stdout, "%s", preamble);
    vfprintf(stdout, msg, args);
    fprintf(stdout, "\n");
    exit(1);
}

pub BTYPEID_T _intersect(struct TM *tm, size_t numTypes, ...) {
    va_list args;  BTYPEID_T *typelist;  int i;  BTYPEID_T btypeid;
    va_start(args, numTypes);
    typelist = malloc((1 + numTypes) * sizeof(BTYPEID_T));
    for (i = 1; i <= numTypes; i++) typelist[i] = va_arg(args, BTYPEID_T);
    typelist[0] = numTypes;
    btypeid = tm_inter(tm, typelist);
    free(typelist);
    va_end(args);
    return btypeid;
}

pub BTYPEID_T _intersect2(struct TM *tm, size_t numTypes, BTYPEID_T *args) {
    BTYPEID_T *typelist;  int i;  BTYPEID_T btypeid;
    typelist = malloc((1 + numTypes) * sizeof(BTYPEID_T));
    for (i = 1; i <= numTypes; i++) typelist[i] = args[i-1];
    typelist[0] = numTypes;
    btypeid = tm_inter(tm, typelist);
    free(typelist);
    return btypeid;
}

#define intersect(tm, ...) ({                                                                                           \
    BTYPEID_T args[] = { __VA_ARGS__ };                                                                                 \
    _intersect2((tm), sizeof(args) / sizeof(args[0]), args);                                                            \
})

#define countArgs(x, y...)  countArgsImpl((x) , ## y, 0)


int countArgsImpl(int x, ...) {
    va_list ap;
    int e;
    int count = 1;

    va_start(ap, x);
    while ((e = va_arg(ap, int)) != 0) {
        count ++;
    }
    va_end(ap);
    return count;
}


void test_tm() {
    BTYPEID_T t, expected;
    struct MM *mm = MM_create();
    struct K *k = K_create(mm);
    PP(debug, "kernel created");

    t = tm_btypeid(k->tm, "joe");
    check(t == 0, "id == %i (should be 0) @ %i", t, __LINE__);

    t = tm_btypeid(k->tm, "sally");
    check(t == 0, "id == %i (should be 0) @ %i", t, __LINE__);

    expected =  k->tm->next_btypeId;
    t = tm_nominal(k->tm, "fred");
    check(t == expected, "t == %i (should be %i)", t, expected);
    t = tm_nominal(k->tm, "joe");
    check(t == ++expected, "t == %i (should be %i)", t, expected);
    t = tm_btypeid(k->tm, "joe");
    check(t == expected, "t == %i (should be %i)", t, expected);

    BTYPEID_T *typelist = malloc(3 * sizeof(BTYPEID_T));
    typelist[0] = 2;
    typelist[1] = 1;
    typelist[2] = 2;
    t = tm_inter(k->tm, typelist);
    check(t == ++expected, "t == %i (should be %i)", t, expected);

    t = tm_inter(k->tm, typelist);
    check(t == expected, "t == %i (should be %i)", t, expected);

    K_trash(k);
}



void test_exclusions() {
    BTYPEID_T tCcy, u32, u64, t7, t8, GBP, EUR, *tl, actual, _GBP;
    struct MM *mm = MM_create();
    struct K *k = K_create(mm);
    struct TM *tm = k->tm;
    PP(debug, "kernel created");

    _GBP = tm_nominal(tm, "_GBP");
    u32 = tm_exclnominal(tm, "u32", btmemory);
    u64 = tm_exclnominal(tm, "u64", btmemory);
    tCcy = tm_exclnominal(tm, "ccy", btuser1);

    GBP = intersect(tm, tCcy, tm_nominal(tm, "_GBP"));
    check(GBP != 0, "t == %i (should not be %i)", GBP, 0);

    EUR = intersect(tm, tCcy, tm_nominal(tm, "_EUR"));
    check(EUR != 0, "t == %i (should not be %i)", EUR, 0);

    // name as
    check(tm_btypeid(tm, "EUR") != EUR, "t == %i (should not be %i) @ %i", tm_btypeid(tm, "EUR"), EUR, __LINE__);
    tm_name_as(tm, EUR, "EUR");
    check(tm_btypeid(tm, "EUR") == EUR, "t == %i (should be %i) @ %i", tm_btypeid(tm, "EUR"), EUR, __LINE__);
    check(strcmp(tm_pp(tm, EUR).cs, "EUR") == 0, "pp(EUR) != \"EUR\" but got \"%s\" @ %i", tm_pp(tm, EUR).cs, __LINE__);

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

    K_trash(k);
}


int main() {
    g_logging_level = info;
    PP(debug, "%i", countArgs(1));
    PP(debug, "%i", countArgs(1,2));
    test_tm();
    test_exclusions();
    PP(info, "passed");
    return 0;
}
