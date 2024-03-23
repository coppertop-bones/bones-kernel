#include "../bk/pp.c"
#include "../bk/k.c"
#include "../../src/bk/tp.c"

pvt void die_(char *preamble, char *msg, va_list args) {
    fprintf(stdout, "%s", preamble);
    vfprintf(stdout, msg, args);
    fprintf(stdout, "\n");
    exit(1);
}


#if defined _WIN64 || defined _WIN32
void test_FILE() {}
#elif defined _APPLE_ || defined __MACH__

void test_FILE() {
    BK_MM *mm;  TPN tpn;  S8 res;

    mm = MM_create();
    Buckets buckets;
    initBuckets(&buckets, 4096);
    BK_TP tp;
    TP_init(&tp, 0, &buckets);

    FILE *f = tp_open(&tp, "r+");
    fprintf(f, "hello %s", "world");
    fflush(f);
    res = tp_s8(&tp, tp_snap(&tp));
    check(strcmp(res.cs, "hello world") == 0, "oh dear");
    fclose(f);

    f = tp_open(&tp, "a+");
    fprintf(f, "hello %s", "world");
    fflush(f);
    f = tp_open(&tp, "a+");
    fprintf(f, "!");
    fflush(f);
    res = tp_s8(&tp, tp_snap(&tp));
    check(strcmp(res.cs, "hello world!") == 0, "oh dear got %s", res.cs);

    f = tp_open(&tp, "a+");
    for (int i=0; i < 100; i++)
        fprintf(f, "1234567890");
    fclose(f);
    res = tp_s8(&tp, tp_snap(&tp));
    check(s8_sz(res) == 100*10, "oh dear %i", s8_sz(res));
    TP_free(&tp);
    freeBuckets(buckets.first_bucket);
}

#elif defined __linux__
void test_FILE() {}
#endif


void test_tp() {
    Buckets buckets;  BK_TP tp;  S8 res;  TPN tpn;

    initBuckets(&buckets, 4096);
    TP_init(&tp, 0, &buckets);

    tp_pb_printf(&tp, "`");
    tp_pb_printf(&tp, "fred");
    tp_pb_printf(&tp, "`");
    tp_pb_printf(&tp, "joe");

    tpn = tp_snap(&tp);
    res = tp_s8(&tp, tpn);

    check(strcmp(res.cs, "`fred`joe") == 0, "oh dear");

    TP_free(&tp);
    freeBuckets(buckets.first_bucket);
}

void test_tpm() {
//    BK_MM *mm = MM_create();
//    BK_K *k = K_create(mm);
//
//    char *txt, *msg;
//    void *cp;
//    cp = tp_mark(k->tp);   // push, note, checkpoint
//    msg = "type error %s";
//
//    char *buf = tp_alloc(0, 1024);           // suggest, pencil, reserve, advise, regrowAt, pad
//    tp_stream(buf);
//    tp_pp_printf(...);
//
//    realloc();
//    txt = tm_pp(tm, btypeid);
//
//
//    tp_strcpy(tp, );
//    tp
//    tp_drop(tp, cp);        // drop, wipe, trash, clean, pop
}


int main() {
    test_tp();
    test_FILE();
    PP(info, "passed");
    return 0;
}
