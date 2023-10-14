// ---------------------------------------------------------------------------------------------------------------------
// DESCRIPTION: Bones uses symbols (a q/kdb term for strings that have been interned) extensively. Symbols are not
//              intended for general strings usage, and it is probably performant to create less rather than more
//              symbols. Symbols are used as type names and in enums and are dictionary presorted for fast sorting.
//
//              struct SM is effectively a hash map that maps a char *name to an id, and vice versa. Symbols exist
//              for the duration of the kernel, so the memory holding the names is grow-only (we allocate a large
//              chunk of NO ACCESS VM, pages are made R/W on demand, and pages that no longer not need writing to
//              are made R/O).
//
//              We sort the symbols lazily, sorting symbols added since the last sort (typically a small set) and
//              merging that with the presorted set.
//
//              If necessary it may be possible to increase lookup performance
//                  better hashing for less probes
//                  access count ordered rehashing to less frequently used symbols occur later in the prob sequence
//                  could compare linear, quadratic, random and dual hashing
// ---------------------------------------------------------------------------------------------------------------------


#ifndef __BK_SM_C
#define __BK_SM_C "bk/sm.c"

#include "../../include/bk/mm.h"
#include "../../include/bk/sm.h"
#include "../../include/bk/os.h"
#include "ht_impl.h"
#include "pp.c"


pvt inline char const * const nameFromEntry(ht_struct(symIdByName) *h, SM_SYM_ID_T entry) {
    return h->sm->names + h->sm->nameRpById[entry];
}

pvt bool inline nameFound(ht_struct(symIdByName) const * const h, SM_SYM_ID_T entry, char const * const key) {
    return strcmp(h->sm->names + h->sm->nameRpById[entry], key) == 0;
}

// HT_IMPL(name, slot_t, key_t, __hash_fn, __found_fn, __key_from_entry_fn)
HT_IMPL(symIdByName, SM_SYM_ID_T, char const *, ht_str_hash, nameFound, nameFromEntry)


pub struct SM * SM_create(struct MM *mm) {
    struct SM *sm = (struct SM *) mm->malloc(sizeof(struct SM));
    sm->mm = mm;
    sm->names = os_vm_reserve(0, SM_MAX_NAME_STORAGE);

    sm->max_rp = os_page_size();
    os_mprotect(sm->names, sm->max_rp, PROT_READ | PROT_WRITE);     // make first page of name storage R/W
    os_madvise(sm->names, sm->max_rp, MADV_RANDOM);                 // and advise as randomly accessed

    sm->nameRpByIdSize = SM_ID_ARRAY_INC_SIZE;
    sm->next_sym_id = SM_NA_SYM + 1;
    sm->next_name_rp = 2;
    sm->nameRpById = mm->malloc(sm->nameRpByIdSize * sizeof(SM_SYM_ID_T));
    onOomDie(sm->nameRpById, "in %s malloc #1 failed", __FUNCTION__);
    sm->sortOrderById = mm->malloc(sm->nameRpByIdSize * sizeof(SM_SYM_ID_T));
    onOomDie(sm->sortOrderById, "in %s malloc #2 failed", __FUNCTION__);
    sm->symIdByName = ht_create(symIdByName);
    sm->symIdByName->sm = sm;
    return sm;
}

pub int SM_trash(struct SM *sm) {
    os_vm_unreserve(sm->names, SM_MAX_NAME_STORAGE);
    sm->mm->free(sm->nameRpById);
    sm->mm->free(sm->sortOrderById);
    ht_trash(symIdByName, sm->symIdByName);
    sm->mm->free(sm);
    return 0;
}

pub SM_SYM_ID_T sm_id(struct SM *sm, char const * const name) {
    int res, pageSize = 0;
    u32 idx = ht_put_idx(symIdByName, sm->symIdByName, name, &res);
    if (res == HT_EXISTS) return sm->symIdByName->slots[idx];

    // add the symbol
    int l = strlen(name);
    if (l >= SM_MAX_NAME_LEN || l == 0) return SM_NA_SYM;
    if (sm->next_name_rp + l > SM_MAX_NAME_STORAGE){
        return SM_NA_SYM;   // OPEN: we've run out of storage space, but really we should add an error reporting mechanism, e.g. SM_ERR_NAME_TOO_LONG, SM_ERR_OUT_OF_NAME_STORAGE etc
    }
    bool needsAnotherPage = (sm->next_name_rp + l + 2 >= sm->max_rp);
    if (needsAnotherPage) {
        // make next page r/w and mark as random access
        pageSize = os_page_size();
        os_mprotect(sm->names + sm->max_rp, pageSize, PROT_READ | PROT_WRITE);
        os_madvise(sm->names + sm->max_rp, pageSize, MADV_RANDOM);
    }
    if (sm->next_sym_id > sm->nameRpByIdSize) {
        sm->nameRpByIdSize += SM_ID_ARRAY_INC_SIZE;
        sm->nameRpById = sm->mm->realloc(sm->nameRpById, sm->nameRpByIdSize * sizeof(SM_SYM_ID_T));
        onOomDie(sm->sortOrderById, "in %s realloc #1 failed", __FUNCTION__);
        sm->sortOrderById = sm->mm->realloc(sm->sortOrderById, sm->nameRpByIdSize * sizeof(SM_SYM_ID_T));
        onOomDie(sm->sortOrderById, "in %s realloc #2 failed", __FUNCTION__);
    }
    SM_SYM_ID_T id = sm->next_sym_id;
    sm->nameRpById[id] = sm->next_name_rp;
    sm->sortOrderById[0] = SM_SYMS_NOT_SORTED;      // OPEN: check if the new syms makes the syms unsorted
    sm->next_sym_id++;
    // OPEN: prefix with length
    strcpy(sm->names + (sm->next_name_rp), name);
    sm->next_name_rp = sm->next_name_rp + 2 + l + 1;

    if (res == HT_EMPTY) ht_replace_empty(symIdByName, sm->symIdByName, idx, id);
    else if (res == HT_TOMBSTONE) ht_replace_tombstone(symIdByName, sm->symIdByName, idx, id);  // OPEN: can never be a HT_TOMBSTONE!
    if (needsAnotherPage) {
        os_mprotect(sm->names + sm->max_rp - pageSize, pageSize, PROT_READ);    // make the prior last page read only
        sm->max_rp += pageSize;
    }
    return id;
}

pub char * sm_name(struct SM *sm, SM_SYM_ID_T id) {
    return &sm->names[sm->nameRpById[id]];
}

pvt void _sm_sort_syms(struct SM *sm) {
    // OPEN: do the sort
    sm->sortOrderById[0] = SM_SYMS_SORTED;
}

pub bool sm_id_le(struct SM *sm, SM_SYM_ID_T a, SM_SYM_ID_T b) {
    if (sm->sortOrderById == SM_SYMS_NOT_SORTED) _sm_sort_syms(sm);
    return sm->sortOrderById[a] < sm->sortOrderById[b];
}


#endif // __BK_SM_C
