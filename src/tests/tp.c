#include "../bk/pp.c"
#include "../bk/kernel.c"



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


int main() {
    test_tp();
    PP(info, "passed");
    return 0;
}
