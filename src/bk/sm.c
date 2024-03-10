// ---------------------------------------------------------------------------------------------------------------------
// SM - SYM MANAGER
//
// DESCRIPTION:
// Bones uses symbols (a q/kdb term for strings that have been interned) extensively. Symbols are not intended for
// general strings usage, and it is probably performant to create less rather than more symbols. Symbols are used as
// type names and in enums and are dictionary presorted for faster sorting.
//
// BK_SM is effectively a hash map that maps a char *name to an id, and vice versa. Symbols exist for the duration of
// the kernel, so the memory holding the names is grow-only (we allocate a large chunk of NO ACCESS VM, pages are
// made R/W on demand, and pages that no longer not need writing to are made R/O).
//
// We sort the symbols lazily, sorting symbols added since the last sort (typically a small set) and merging that with
// the presorted set.
//
// If necessary it may be possible to increase lookup performance
// - better hashing for less probes
// - access count ordered rehashing to less frequently used symbols occur later in the prob sequence
// - could compare linear, quadratic, random and dual hashing
// ---------------------------------------------------------------------------------------------------------------------


#ifndef __BK_SM_C
#define __BK_SM_C "bk/sm.c"

#include "../../include/bk/mm.h"
#include "../../include/bk/sm.h"
#include "../../include/bk/lib/os.h"
#include "lib/hi_impl.tmplt"
#include "pp.c"


pvt inline char * nameFromEntry(hi_struct(SM_SYMID_BY_NAMEHASH) *h, symid_t entry) {
    return h->sm->symname_buf + h->sm->rp_by_symid[entry];
}

pvt bool inline nameFound(hi_struct(SM_SYMID_BY_NAMEHASH) *h, symid_t entry, char *key) {
    return strcmp(h->sm->symname_buf + h->sm->rp_by_symid[entry], key) == 0;
}

// HI_IMPL(name, token_t, hashable_t, __hash_fn, __found_fn, __hashable_from_token_fn)
HI_IMPL(SM_SYMID_BY_NAMEHASH, symid_t, char *, hi_chars_X31_hash, nameFound, nameFromEntry)


pub BK_SM * SM_create(BK_MM *mm) {
    BK_SM *sm = (BK_SM *) mm->malloc(sizeof(BK_SM));
    sm->mm = mm;
    sm->symname_buf = os_vm_reserve(0, SM_MAX_NAME_STORAGE);

    sm->max_rp = os_page_size();
    os_mprotect(sm->symname_buf, sm->max_rp, BK_PROT_READ | BK_PROT_WRITE);   // make first page of name storage R/W
    os_madvise(sm->symname_buf, sm->max_rp, BK_MADV_RANDOM);              // and advise as randomly accessed

    sm->max_symid = SM_MAX_SYM_ID_INC_SIZE;
    sm->next_symid = SM_NA_SYM + 1;
    sm->next_rp = 2;                                               // i.e. pointer to the char after the len prefix
    sm->rp_by_symid = mm->malloc(sm->max_symid * sizeof(RP));
    onOomDie(sm->rp_by_symid, s8("in %s malloc #1 failed"), FN_NAME);
    sm->sortorder_by_symid = mm->malloc(sm->max_symid * sizeof(symid_t));
    onOomDie(sm->sortorder_by_symid, s8("in %s malloc #2 failed"), FN_NAME);
    sm->symid_by_namehash = hi_create(SM_SYMID_BY_NAMEHASH);
    sm->symid_by_namehash->sm = sm;
    return sm;
}

pub int SM_trash(BK_SM *sm) {
    os_vm_unreserve(sm->symname_buf, SM_MAX_NAME_STORAGE);
    sm->mm->free(sm->rp_by_symid);
    sm->mm->free(sm->sortorder_by_symid);
    hi_trash(SM_SYMID_BY_NAMEHASH, sm->symid_by_namehash);
    sm->mm->free(sm);
    return 0;
}

pub symid_t sm_id(BK_SM *sm, char *name) {
    int res, pageSize = 0;
    u32 idx = hi_put_idx(SM_SYMID_BY_NAMEHASH, sm->symid_by_namehash, name, &res);

    if (res == HI_LIVE) return sm->symid_by_namehash->tokens[idx];
    if (res == HI_TOMBSTONE) die("TOMBSTONE");

    // add the symbol
    size l = strlen(name);
    if (l >= SM_MAX_NAME_LEN || l == 0) return SM_NA_SYM;
    if (sm->next_rp + l >= SM_MAX_NAME_STORAGE) die("%s: out of typelist storage", FN_NAME);   // OPEN: we've run out of storage space, but really we should add an error reporting mechanism, e.g. SM_ERR_NAME_TOO_LONG, SM_ERR_OUT_OF_NAME_STORAGE etc
    bool needsAnotherPage = (2 + sm->next_rp + l + 1 >= sm->max_rp);
    if (needsAnotherPage) {
        // make next page r/w and mark as random access
        pageSize = os_page_size();
        os_mprotect(sm->symname_buf + sm->max_rp, pageSize, BK_PROT_READ | BK_PROT_WRITE);
        os_madvise(sm->symname_buf + sm->max_rp, pageSize, BK_MADV_RANDOM);
    }
    if (sm->next_symid >= sm->max_symid) {
        // xxx_by_symid arrays need growing
        sm->max_symid += SM_MAX_SYM_ID_INC_SIZE;
        sm->rp_by_symid = sm->mm->realloc(sm->rp_by_symid, sm->max_symid * sizeof(symid_t));
        onOomDie(sm->sortorder_by_symid, s8("in %s realloc #1 failed"), FN_NAME);
        sm->sortorder_by_symid = sm->mm->realloc(sm->sortorder_by_symid, sm->max_symid * sizeof(symid_t));
        onOomDie(sm->sortorder_by_symid, s8("in %s realloc #2 failed"), FN_NAME);
    }
    symid_t id = sm->next_symid;
    sm->rp_by_symid[id] = sm->next_rp;
    sm->sortorder_by_symid[0] = SM_SYMS_NOT_SORTED;      // OPEN: check if the new syms makes the syms unsorted
    sm->next_symid++;
    // OPEN: prefix with length
    strcpy(sm->symname_buf + (sm->next_rp), name);
    sm->next_rp += 2 + (int)l + 1;

    hi_replace_empty(SM_SYMID_BY_NAMEHASH, sm->symid_by_namehash, idx, id);

    if (needsAnotherPage) {
        os_mprotect(sm->symname_buf + sm->max_rp - pageSize, pageSize, BK_PROT_READ);     // make the prior last page read only
        sm->max_rp += pageSize;
    }

    return id;
}

pub char * sm_name(BK_SM *sm, symid_t id) {
    return &sm->symname_buf[sm->rp_by_symid[id]];
}

pvt void _sm_sort_syms(BK_SM *sm) {
    // OPEN: do the sort
    sm->sortorder_by_symid[0] = SM_SYMS_SORTED;
}

pub bool sm_id_le(BK_SM *sm, symid_t a, symid_t b) {
    if (sm->sortorder_by_symid == SM_SYMS_NOT_SORTED) _sm_sort_syms(sm);
    return sm->sortorder_by_symid[a] < sm->sortorder_by_symid[b];
}


#endif // __BK_SM_C
