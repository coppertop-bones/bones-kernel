#include "../bk/pp.c"
#include "../bk/kernel.c"

pvt void die_(char *preamble, char *msg, va_list args) {
    fprintf(stderr, "%s", preamble);
    vfprintf(stderr, msg, args);
    exit(1);
}

pub BTYPEID_T _intersect(struct TM *tm, size_t numTypes, ...) {
    va_list args;  BTYPEID_T *typelist;  int i;  BTYPEID_T btypeid;
    va_start(args, numTypes);
    typelist = malloc((1 + numTypes) * sizeof(BTYPEID_T));
    for (i = 1; i <= numTypes; i++) typelist[i] = va_arg(args, unsigned int);
    typelist[0] = numTypes;
    btypeid = tm_inter(tm, typelist);
    free(typelist);
    va_end(args);
    return btypeid;
}

pub BTYPEID_T _intersect2(struct TM *tm, size_t numTypes, BTYPEID_T const *args) {
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
    _intersect2((tm), sizeof(args) / sizeof(args[0]), args);                                                              \
})



void test_tm() {
    int id, refid;
    struct MM *mm = MM_create();
    struct K *k = K_create(mm);
    PP(info, "kernel created");

    id = tm_btypeid(k->tm, "joe");
    check(id == 0, "id == %i (should be 0) @ %i", id, __LINE__);

    id = tm_btypeid(k->tm, "sally");
    check(id == 0, "id == %i (should be 0) @ %i", id, __LINE__);

    refid =  k->tm->next_btypeId;
    id = tm_nominal(k->tm, "fred");
    check(id == refid, "id == %i (should be %i)", id, refid);
    id = tm_nominal(k->tm, "joe");
    check(id == ++refid, "id == %i (should be %i)", id, refid);
    id = tm_btypeid(k->tm, "joe");
    check(id == refid, "id == %i (should be %i)", id, refid);

    BTYPEID_T *typelist = malloc(3 * sizeof(BTYPEID_T));
    typelist[0] = 2;
    typelist[1] = 1;
    typelist[2] = 2;
    id = tm_inter(k->tm, typelist);
    check(id == ++refid, "id == %i (should be %i)", id, refid);

    id = tm_inter(k->tm, typelist);
    check(id == refid, "id == %i (should be %i)", id, refid);

    K_trash(k);
}



void test_exclusions() {
    int tCcy, u32, u64, t4, t5, t6, t7, _GBP, GBP, EUR;  BTYPEID_T *tl, res;
    struct MM *mm = MM_create();
    struct K *k = K_create(mm);
    PP(info, "kernel created");

    _GBP = tm_nominal(k->tm, "_GBP");
    u32 = tm_exclnominal(k->tm, "u32", btmemory);
    u64 = tm_exclnominal(k->tm, "u64", btmemory);
    tCcy = tm_exclnominal(k->tm, "ccy", btuser1);

    t4 = intersect(k->tm, u32, tCcy);
    t5 = intersect(k->tm, u64, tCcy);

    t6 = intersect(k->tm, t4, t5);
    check(t6 == 0, "id == %i (should be %i)", t6, 0);

    GBP = intersect(k->tm, tCcy, tm_nominal(k->tm, "_GBP"));
    check(GBP != 0, "id == %i (should not be %i)", GBP, 0);

    EUR = intersect(k->tm, tCcy, tm_nominal(k->tm, "_EUR"));
    check(EUR != 0, "id == %i (should not be %i)", EUR, 0);


    // name as
    check(tm_btypeid(k->tm, "EUR") != EUR, "id == %i (should not be %i) @ %i", tm_btypeid(k->tm, "EUR"), EUR, __LINE__);
    tm_name_as(k->tm, EUR, "EUR");
    check(tm_btypeid(k->tm, "EUR") == EUR, "id == %i (should be %i) @ %i", tm_btypeid(k->tm, "EUR"), EUR, __LINE__);


    tm_name_as(k->tm, GBP, "GBP");

    t7 = intersect(k->tm, GBP, u32);
    check(t7 != 0, "id == %i (should not be %i) @ %i", t7, 0, __LINE__);

    res = intersect(k->tm, t7, u64);
    check(res == 0, "id == %i (should be %i) @ %i", res, 0, __LINE__);

    tm_pp(k->tm, GBP);
    PP(info, "");
    tm_pp(k->tm, EUR);
    PP(info, "");

    check(intersect(k->tm, GBP, EUR) != 0, "id == %i (should not be %i) @ %i", intersect(k->tm, GBP, EUR), 0, __LINE__);

    res = intersect(k->tm, t7, EUR);
    check(res == 0, "id == %i (should be %i) @ %i", res, 0, __LINE__);

    K_trash(k);
}


int main() {
    test_tm();
    test_exclusions();
    PP(info, "passed");
    return 0;
}
