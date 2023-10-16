#include "../bk/pp.c"
#include "../bk/kernel.c"

pvt void die_(char *preamble, char *msg, va_list args) {
    fprintf(stderr, "%s", preamble);
    vfprintf(stderr, msg, args);
    exit(1);
}

int main() {
    int id, refid;
    struct MM *mm = MM_create();
    struct K *k = K_create(mm);
    PP(info, "kernel created");

    id = tm_btypeid(k->tm, "joe");
    check(id == 0, "id == %i (should be 0)", id);

    id = tm_btypeid(k->tm, "sally");
    check(id == 0, "id == %i (should be 0)", id);

    refid =  k->tm->next_btypeId;
    id = tm_nominal(k->tm, "fred");
    check(id == refid, "id == %i (should be %i)", id, refid);
    id = tm_nominal(k->tm, "joe");
    check(id == ++refid, "id == %i (should be %i)", id, refid);
    id = tm_btypeid(k->tm, "joe");
    check(id == refid, "id == %i (should be %i)", id, refid);
    
    BTYPE_ID_T *typelist = malloc(3 * sizeof(BTYPE_ID_T));
    typelist[0] = 2;
    typelist[1] = 1;
    typelist[2] = 2;
    id = tm_inter(k->tm, typelist);
    check(id == ++refid, "id == %i (should be %i)", id, refid);
    
    id = tm_inter(k->tm, typelist);
    check(id == refid, "id == %i (should be %i)", id, refid);

    K_trash(k);
    PP(info, "passed");
    return 0;
}
