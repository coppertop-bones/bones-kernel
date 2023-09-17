#include "../bk/pp.c"
#include "../bk/sm.c"

pvt void die_(char *preamble, char *msg, va_list args) {
    fprintf(stderr, "%s", preamble);
    vfprintf(stderr, msg, args);
    exit(1);
}

int main() {
    int id;
    struct SM *sm = sm_create();
    PP(info, "sm created");

    id = sm_id(sm, "fred");
    check(id == 1, "id == %i (should be 1)", id);

    id = sm_id(sm, "fred");
    check(id == 1, "id == %i (should be 1)", id);

    id = sm_id(sm, "joe");
    check(id == 2, "id == %i (should be 2)", id);

    sm_trash(sm);
    PP(info, "passed");
    return 0;
}
