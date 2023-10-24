#include "../bk/pp.c"
#include "../bk/kernel.c"
#include "../../src/bk/tp.c"

pvt void die_(char *preamble, char *msg, va_list args) {
    fprintf(stdout, "%s", preamble);
    vfprintf(stdout, msg, args);
    fprintf(stdout, "\n");
    exit(1);
}

void test_tp() {
    struct MM *mm = MM_create();
    Buckets buckets;
    initBuckets(&buckets, 4096);
    TP tp;
    tp_init(&tp, 0, &buckets);
    FILE *f = tp_open(&tp, "r+");
    fprintf(f, "hello %s", "world");
    fflush(f);
    s8 res = tp_getS8(&tp);
    check(strcmp(res.cs, "hello world") == 0, "oh dear");
    fclose(f);
    f = tp_open(&tp, "a+");
    fprintf(f, "!");
    fflush(f);
    check(strcmp(res.cs, "hello world!") == 0, "oh dear got %s", res.cs);

    f = tp_open(&tp, "a+");
    for (int i=0; i < 100; i++)
        fprintf(f, "1234567890");
    fclose(f);
    res = tp_getS8(&tp);
    check(res.szs > 100*10, "oh dear %i", res.szs);
    tp_free(&tp);
    freeBuckets(buckets.first_bucket);
}

void test_tpm() {
//    struct MM *mm = MM_create();
//    struct K *k = K_create(mm);
//
//    char *txt, *msg;
//    void *cp;
//    cp = tp_mark(k->tp);   // push, note, checkpoint
//    msg = "type error %s";
//
//    char *buf = tp_alloc(0, 1024);           // suggest, pencil, reserve, advise, regrowAt, pad
//    tp_stream(buf);
//    tp_printf(...);
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
    PP(info, "passed");
    return 0;
}
