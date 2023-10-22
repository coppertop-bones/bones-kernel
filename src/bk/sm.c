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
#include "../lib/ht_impl.c"
#include "pp.c"


pvt inline char * nameFromEntry(ht_struct(SM_SYMID_BY_NAMEHASH) *h, SYM_ID_T entry) {
    return h->sm->symname_buf + h->sm->rp_by_symid[entry];
}

pvt bool inline nameFound(ht_struct(SM_SYMID_BY_NAMEHASH) *h, SYM_ID_T entry, char *key) {
    return strcmp(h->sm->symname_buf + h->sm->rp_by_symid[entry], key) == 0;
}

// HT_IMPL(name, slot_t, key_t, __hash_fn, __found_fn, __key_from_entry_fn)
HT_IMPL(SM_SYMID_BY_NAMEHASH, SYM_ID_T, char *, ht_str_hash, nameFound, nameFromEntry)


pub struct SM * SM_create(struct MM *mm) {
    struct SM *sm = (struct SM *) mm->malloc(sizeof(struct SM));
    sm->mm = mm;
    sm->symname_buf = os_vm_reserve(0, SM_MAX_NAME_STORAGE);

    sm->max_rp = os_page_size();
    os_mprotect(sm->symname_buf, sm->max_rp, BK_M_READ | BK_M_WRITE);   // make first page of name storage R/W
    os_madvise(sm->symname_buf, sm->max_rp, BK_AD_RANDOM);              // and advise as randomly accessed

    sm->max_symid = SM_MAX_SYM_ID_INC_SIZE;
    sm->next_symid = SM_NA_SYM + 1;
    sm->next_rp = 2;                                               // i.e. pointer to the char after the len prefix
    sm->rp_by_symid = mm->malloc(sm->max_symid * sizeof(RP));
    onOomDie(sm->rp_by_symid, s8("in %s malloc #1 failed"), FN_NAME);
    sm->sortorder_by_symid = mm->malloc(sm->max_symid * sizeof(SYM_ID_T));
    onOomDie(sm->sortorder_by_symid, s8("in %s malloc #2 failed"), FN_NAME);
    sm->symid_by_namehash = ht_create(SM_SYMID_BY_NAMEHASH);
    sm->symid_by_namehash->sm = sm;
    return sm;
}

pub int SM_trash(struct SM *sm) {
    os_vm_unreserve(sm->symname_buf, SM_MAX_NAME_STORAGE);
    sm->mm->free(sm->rp_by_symid);
    sm->mm->free(sm->sortorder_by_symid);
    ht_trash(SM_SYMID_BY_NAMEHASH, sm->symid_by_namehash);
    sm->mm->free(sm);
    return 0;
}

pub SYM_ID_T sm_id(struct SM *sm, char *name) {
    int res, pageSize = 0;
    u32 idx = ht_put_idx(SM_SYMID_BY_NAMEHASH, sm->symid_by_namehash, name, &res);

    if (res == HT_EXISTS) return sm->symid_by_namehash->slots[idx];
    if (res == HT_TOMBSTONE) die("TOMBSTONE");

    // add the symbol
    int l = strlen(name);
    if (l >= SM_MAX_NAME_LEN || l == 0) return SM_NA_SYM;
    if (sm->next_rp + l >= SM_MAX_NAME_STORAGE) die("%s: out of typelist storage", FN_NAME);   // OPEN: we've run out of storage space, but really we should add an error reporting mechanism, e.g. SM_ERR_NAME_TOO_LONG, SM_ERR_OUT_OF_NAME_STORAGE etc
    bool needsAnotherPage = (2 + sm->next_rp + l + 1 >= sm->max_rp);
    if (needsAnotherPage) {
        // make next page r/w and mark as random access
        pageSize = os_page_size();
        os_mprotect(sm->symname_buf + sm->max_rp, pageSize, BK_M_READ | BK_M_WRITE);
        os_madvise(sm->symname_buf + sm->max_rp, pageSize, BK_AD_RANDOM);
    }
    if (sm->next_symid >= sm->max_symid) {
        // xxx_by_symid arrays need growing
        sm->max_symid += SM_MAX_SYM_ID_INC_SIZE;
        sm->rp_by_symid = sm->mm->realloc(sm->rp_by_symid, sm->max_symid * sizeof(SYM_ID_T));
        onOomDie(sm->sortorder_by_symid, s8("in %s realloc #1 failed"), FN_NAME);
        sm->sortorder_by_symid = sm->mm->realloc(sm->sortorder_by_symid, sm->max_symid * sizeof(SYM_ID_T));
        onOomDie(sm->sortorder_by_symid, s8("in %s realloc #2 failed"), FN_NAME);
    }
    SYM_ID_T id = sm->next_symid;
    sm->rp_by_symid[id] = sm->next_rp;
    sm->sortorder_by_symid[0] = SM_SYMS_NOT_SORTED;      // OPEN: check if the new syms makes the syms unsorted
    sm->next_symid++;
    // OPEN: prefix with length
    strcpy(sm->symname_buf + (sm->next_rp), name);
    sm->next_rp += 2 + l + 1;

    ht_replace_empty(SM_SYMID_BY_NAMEHASH, sm->symid_by_namehash, idx, id);

    if (needsAnotherPage) {
        os_mprotect(sm->symname_buf + sm->max_rp - pageSize, pageSize, BK_M_READ);     // make the prior last page read only
        sm->max_rp += pageSize;
    }

    return id;
}

pub char * sm_name(struct SM *sm, SYM_ID_T id) {
    return &sm->symname_buf[sm->rp_by_symid[id]];
}

pvt void _sm_sort_syms(struct SM *sm) {
    // OPEN: do the sort
    sm->sortorder_by_symid[0] = SM_SYMS_SORTED;
}

pub bool sm_id_le(struct SM *sm, SYM_ID_T a, SYM_ID_T b) {
    if (sm->sortorder_by_symid == SM_SYMS_NOT_SORTED) _sm_sort_syms(sm);
    return sm->sortorder_by_symid[a] < sm->sortorder_by_symid[b];
}


#endif // __BK_SM_C
