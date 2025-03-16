// ---------------------------------------------------------------------------------------------------------------------
// TDD TESTS - SYM MANAGER
// ---------------------------------------------------------------------------------------------------------------------

#include "../bk/pp.c"
#include "../bk/mm.c"
#include "../bk/tp.c"
#include "../bk/k.c"

pvt void die_(char *preamble, char *msg, va_list args) {
    fprintf(stderr, "%s", preamble);
    vfprintf(stderr, msg, args);
    exit(1);
}

int main() {
    int id;
    BK_MM *mm = MM_create();
    Buckets *buckets = mm->malloc(sizeof(Buckets));
    Buckets_init(buckets, 64);
    BK_K *k = K_create(mm, buckets);
    PP(info, "kernel created");

    int off = 6;

    id = sm_id(k->sm, "fred");
    check(id == 1 + off, "id == %i (should be %i)", id, 1 + off);

    id = sm_id(k->sm, "fred");
    check(id == 1 + off, "id == %i (should be %i)", id, 1 + off);

    id = sm_id(k->sm, "joe");
    check(id == 2 + off, "id == %i (should be %i)", id, 2 + off);

    id = tm_get(k->tm, "joe");
    check(id == 0, "id == %i (should be %i)", id, 0);

    id = tm_get(k->tm, "sally");
    check(id == 0, "id == %i (should be %i)", id, 0);


    K_trash(k);
    Buckets_finalise(buckets);
    free(buckets);
    PP(info, "passed");
    return 0;
}
