// ---------------------------------------------------------------------------------------------------------------------
// Copyright 2025 David Briant, https://github.com/coppertop-bones. Licensed under the Apache License, Version 2.0
//
// TDD TESTS - SYM MANAGER
// ---------------------------------------------------------------------------------------------------------------------

#include "../../src/bk/pp.c"
#include "../../src/bk/mm.c"
#include "../../src/bk/tp.c"
#include "../../src/bk/k.c"

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
    check(id == 1 + off, "id == %i (should be %i)", __FILE__, __LINE__, id, 1 + off);

    id = sm_id(k->sm, "fred");
    check(id == 1 + off, "id == %i (should be %i)", __FILE__, __LINE__, id, 1 + off);

    id = sm_id(k->sm, "joe");
    check(id == 2 + off, "id == %i (should be %i)", __FILE__, __LINE__, id, 2 + off);

    id = tm_lookup(k->tm, "joe");
    check(id == 0, "id == %i (should be %i)", __FILE__, __LINE__, id, 0);

    id = tm_lookup(k->tm, "sally");
    check(id == 0, "id == %i (should be %i)", __FILE__, __LINE__, id, 0);


    K_trash(k);
    Buckets_finalise(buckets);
    free(buckets);
    PP(info, "passed");
    return 0;
}
