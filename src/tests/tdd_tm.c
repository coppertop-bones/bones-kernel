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


void * tp_mark(struct TP *);            // null is error
int tp_drop(struct TP *, void *);       //


void test_tp() {
    struct MM *mm = MM_create();
    struct K *k = K_create(mm);

    char *txt, *msg;
    void *cp;
    cp = tp_mark(k->tp);   // push, note, checkpoint
    msg = "type error %s";

    char *buf = tp_alloc(0, 1024);           // suggest, pencil, reserve, advise, regrowAt, pad
    tp_stream(buf);
    tp_printf(...);

    realloc();
    txt = tm_pp(tm, btypeid);


    tp_strcpy(tp, );
    tp
    tp_drop(tp, cp);        // drop, wipe, trash, clean, pop
}


void test_tm() {
    BTYPEID_T t, expected;
    struct MM *mm = MM_create();
    struct K *k = K_create(mm);
    PP(info, "kernel created");

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
    BTYPEID_T tCcy, u32, u64, t4, t5, t6, t7, _GBP, GBP, EUR, *tl, res;
    struct MM *mm = MM_create();
    struct K *k = K_create(mm);
    PP(info, "kernel created");

    _GBP = tm_nominal(k->tm, "_GBP");
    u32 = tm_exclnominal(k->tm, "u32", btmemory);
    u64 = tm_exclnominal(k->tm, "u64", btmemory);
    tCcy = tm_exclnominal(k->tm, "ccy", btuser1);


    t6 = intersect(k->tm, intersect(k->tm, u32, tCcy), intersect(k->tm, u64, tCcy));
    check(t6 == 0, "t == %i (should be %i)", t6, 0);

    GBP = intersect(k->tm, tCcy, tm_nominal(k->tm, "_GBP"));
    check(GBP != 0, "t == %i (should not be %i)", GBP, 0);

    EUR = intersect(k->tm, tCcy, tm_nominal(k->tm, "_EUR"));
    check(EUR != 0, "t == %i (should not be %i)", EUR, 0);


    // name as
    check(tm_btypeid(k->tm, "EUR") != EUR, "t == %i (should not be %i) @ %i", tm_btypeid(k->tm, "EUR"), EUR, __LINE__);
    tm_name_as(k->tm, EUR, "EUR");
    check(tm_btypeid(k->tm, "EUR") == EUR, "t == %i (should be %i) @ %i", tm_btypeid(k->tm, "EUR"), EUR, __LINE__);


    tm_name_as(k->tm, GBP, "GBP");

    t7 = intersect(k->tm, GBP, u32);
    check(t7 != 0, "t == %i (should not be %i) @ %i", t7, 0, __LINE__);

    res = intersect(k->tm, t7, u64);
    check(res == 0, "t == %i (should be %i) @ %i", res, 0, __LINE__);

    tm_pp(k->tm, GBP);
    PP(info, "");
    tm_pp(k->tm, EUR);
    PP(info, "");

    check(intersect(k->tm, GBP, EUR) != 0, "t == %i (should not be %i) @ %i", intersect(k->tm, GBP, EUR), 0, __LINE__);

    res = intersect(k->tm, t7, EUR);
    check(res == 0, "t == %i (should be %i) @ %i", res, 0, __LINE__);

    K_trash(k);
}


int main() {
    test_tm();
    test_exclusions();
    PP(info, "passed");
    return 0;
}
