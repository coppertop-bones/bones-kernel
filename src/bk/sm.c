#ifndef __BK_SM_C
#define __BK_SM_C "bk/sm.c"

#include "../../include/bk/sm.h"
#include "ht_impl.h"


//struct SM {
//    char *names;                        // 8 - VM buffer of u16 length prefixed, null terminated utf8 strings for type name and sym interning
//    RP *nameRpById;                     // 8 - array of name RP indexed by id
//    u32 *sortOrderById;                 // 8 - array of sort_order indexed by id - slot0 is 1 if sorted, 0 if not sorted
//    ht_struct(symIdByName) *symIdByName;   // 8
//    unsigned int nameRpByIdSize;        // 4
//    u32 next_sym_id;                    // 4
//    RP next_name_rp;                    // 4
//};


pvt inline char const * const keyFromId(ht_struct(symIdByName) *h, u32 id) {
    return h->sm->names + h->sm->nameRpById[id];
}

pvt bool symIdMatchesKey(ht_struct(symIdByName) const * const h, u32 id, char const * const key) {
    return strcmp(h->sm->names + h->sm->nameRpById[id], key) == 0;
}

// BK_H_IMPL(name, object_t, key_t, __hash_fn, __found_fn, __key_from_object_fn)
BK_H_IMPL(symIdByName, u32, char const *, ht_str_hash_func, symIdMatchesKey, keyFromId)


pub struct SM * sm_create() {
    struct SM *sm = (struct SM *) malloc(sizeof(struct SM));
    // TODO reserve a bunch (4GB) of virtual memory, protect, unprotect in pages
    // must be on a page boundary but could be chosen in an attempt to contend at cache set level
    sm->names = malloc(SM_MAX_SYM_STORAGE);
    sm->nameRpByIdSize = 1024;
    sm->next_sym_id = 1;
    sm->next_name_rp = 2;
    sm->nameRpById = malloc(sm->nameRpByIdSize * sizeof(u32));
    sm->sortOrderById = malloc(sm->nameRpByIdSize * sizeof(u32));
    sm->symIdByName = ht_create(symIdByName);
    return sm;
}

pub void sm_trash(struct SM *sm) {
    free(sm->syms);
    free(sm->rps);
    free(sm->positions);
    free(sm->hm);
    free(sm);
}


pvt u32 symById_found(char * s) {

}



pub u32 sm_id(struct SM *sm, char const * const buf) {
    int res;
    int hash = __ac_X31_hash_string(buf);
    int it = find_slot(fred, )

    kh_iter_t it = kh_put_it(fred, SM->hm, buf, res)
    // compute hash of buf
    // get index into hm
    // prob to find rp_id slot that matches buf
    // if found answer id
    // if not found
    //   compute length
    //   save buf in syms
    //   etc
    //   if new syms comes before old end - mark sorted as falsel
    //   answer id
    // count num chars - ensure <= MAX SYM SIZE, if not set last err and return 0
    // set not sorted flag
    // place count at start
    //
    return (u32) 1;
}

pub char * sm_buf(struct SM *sm, u32 id) {
    return &sm->syms[sm->rps[id]];
}

pvt void sm_sort(struct SM *sm) {
}

pub bool sm_id_le(struct SM *sm, u32 a, u32 b) {
    if (sm->positions == 0) sm_sort(sm);
    return sm->positions[a] < sm->positions[b];
}

pub inline RP sm_id_2_RP(struct SM *sm, u32 id) {
    return sm->rps[id];
}


#endif // __BK_SM_C
