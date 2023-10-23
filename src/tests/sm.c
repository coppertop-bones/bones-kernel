#include "../bk/pp.c"
#include "../bk/kernel.c"

pvt void die_(char *preamble, char *msg, va_list args) {
    fprintf(stderr, "%s", preamble);
    vfprintf(stderr, msg, args);
    exit(1);
}

int main() {
    int id;
    struct MM *mm = MM_create();
    struct K *k = K_create(mm);
    PP(info, "kernel created");

    id = sm_id(k->sm, "fred");
    check(id == 1, "id == %i (should be 1)", id);

    id = sm_id(k->sm, "fred");
    check(id == 1, "id == %i (should be 1)", id);

    id = sm_id(k->sm, "joe");
    check(id == 2, "id == %i (should be 2)", id);

    id = tm_btypeid(k->tm, "joe");
    check(id == 0, "id == %i (should be 0)", id);

    id = tm_btypeid(k->tm, "sally");
    check(id == 0, "id == %i (should be 0)", id);


    K_trash(k);
    PP(info, "passed");
    return 0;
}
