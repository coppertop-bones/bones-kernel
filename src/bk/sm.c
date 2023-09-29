#ifndef __BK_SM_C
#define __BK_SM_C "bk/sm.c"

#include "../../include/bk/sm.h"
#include "../../include/bk/os.h"
#include "ht_impl.h"


pvt inline char const * const nameFromEntry(ht_struct(symIdByName) *h, u32 entry) {
    return h->sm->names + h->sm->nameRpById[entry];
}

pvt bool inline nameFound(ht_struct(symIdByName) const * const h, u32 entry, char const * const key) {
    return strcmp(h->sm->names + h->sm->nameRpById[entry], key) == 0;
}

// HT_IMPL(name, slot_t, key_t, __hash_fn, __found_fn, __key_from_entry_fn)
HT_IMPL(symIdByName, u32, char const *, ht_str_hash, nameFound, nameFromEntry)


pub struct SM * sm_create() {
    struct SM *sm = (struct SM *) malloc(sizeof(struct SM));
    sm->names = os_vm_reserve(0, SM_MAX_NAME_STORAGE);
    // make first page read / write + advise as randomly accessed
    sm->max_rp = os_page_size();
    os_mprotect(sm->names, sm->max_rp, PROT_READ | PROT_WRITE);
    os_madvise(sm->names, sm->max_rp, MADV_RANDOM);

    sm->nameRpByIdSize = 0x4000;    // 16k syms for the mo
    sm->next_sym_id = 1;
    sm->next_name_rp = 2;
    sm->nameRpById = malloc(sm->nameRpByIdSize * sizeof(u32));
    sm->sortOrderById = malloc(sm->nameRpByIdSize * sizeof(u32));
    sm->symIdByName = ht_create(symIdByName);
    sm->symIdByName->sm = sm;
    return sm;
}

pub void sm_trash(struct SM *sm) {
    os_vm_unreserve(sm->names, SM_MAX_NAME_STORAGE);
    free(sm->nameRpById);
    free(sm->sortOrderById);
    ht_trash(symIdByName, sm->symIdByName);
    free(sm);
}

pub u32 sm_id(struct SM *sm, char const * const name) {
    int res, pageSize;
    u32 idx = ht_put_idx(symIdByName, sm->symIdByName, name, &res);
    if (res == HT_EXISTS) return sm->symIdByName->slots[idx];

    // add the symbol
    int l = strlen(name);
    if (l >= SM_MAX_NAME_LEN) return NA_SYM;
    if (sm->next_name_rp + l > SM_MAX_NAME_STORAGE){
        // we've run out of storage space
        return NA_SYM;
    }
    bool needsAnotherPage = (sm->next_name_rp + l + 2 >= sm->max_rp);
    if (needsAnotherPage) {
        // make next page r/w and mark as random access
        pageSize = os_page_size();
        os_mprotect(sm->names + sm->max_rp, pageSize, PROT_READ | PROT_WRITE);
        os_madvise(sm->names + sm->max_rp, pageSize, MADV_RANDOM);
    }
    if (sm->next_sym_id > sm->nameRpByIdSize) {
        // OPEN: grow the nameRpById and sortOrderById arrays
        return NA_SYM;
    }
    u32 id = sm->next_sym_id;
    sm->nameRpById[id] = sm->next_name_rp;
    sm->sortOrderById[0] = 0;       // mark as unsorted, OPEN: check if the new syms makes the syms unsorted
    sm->next_sym_id++;
    // OPEN: put count at start
    strcpy(sm->names + (sm->next_name_rp), name);
    sm->next_name_rp = sm->next_name_rp + 2 + l + 1;

    if (res == HT_EMPTY) ht_replace_empty(symIdByName, sm->symIdByName, idx, id);
    else if (res == HT_TOMBSTONE) ht_replace_tombstone(symIdByName, sm->symIdByName, idx, id);
    if (needsAnotherPage) {
        // make the prior last page read only
        os_mprotect(sm->names + sm->max_rp, pageSize, PROT_READ);
        sm->max_rp += pageSize;
    }
    return id;
}

pub char * sm_name(struct SM *sm, u32 id) {
    return &sm->names[sm->nameRpById[id]];
}

pvt void _sm_sort_syms(struct SM *sm) {
}

pub bool sm_id_le(struct SM *sm, u32 a, u32 b) {
    if (sm->sortOrderById == 0) _sm_sort_syms(sm);
    return sm->sortOrderById[a] < sm->sortOrderById[b];
}


#endif // __BK_SM_C
