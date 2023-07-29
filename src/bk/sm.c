// ---------------------------------------------------------------------------------------------------------------------
// Copyright 2025 David Briant, https://github.com/coppertop-bones. Licensed under the Apache License, Version 2.0
//
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

#include "mm.c"
#include "../../include/bk/sm.h"
#include "../../include/bk/tp.h"
#include "../../include/bk/lib/os.h"
#include "lib/hi_impl.tmplt"
#include "pp.c"


// ---------------------------------------------------------------------------------------------------------------------
// SM_SYMID_BY_NAMEHASH fns
// ---------------------------------------------------------------------------------------------------------------------

pvt inline char * nameFromEntry(hi_struct(SM_SYMID_BY_NAMEHASH) *h, symid_t entry) {
    return h->sm->symname_buf + h->sm->rp_by_symid[entry];
}

pvt bool inline nameFound(hi_struct(SM_SYMID_BY_NAMEHASH) *h, symid_t entry, char const *key) {
    return strcmp(h->sm->symname_buf + h->sm->rp_by_symid[entry], key) == 0;
}

// HI_IMPL(name, token_t, hashable_t, __hash_fn, __found_fn, __hashable_from_token_fn)
HI_IMPL(SM_SYMID_BY_NAMEHASH, symid_t, char const *, hi_chars_X31_hash, nameFound, nameFromEntry)


// ---------------------------------------------------------------------------------------------------------------------
// SM_SLID_BY_SLHASH fns
// ---------------------------------------------------------------------------------------------------------------------

pvt inline btypeid_t * slFromSlid(hi_struct(SM_SLID_BY_SLHASH) *hi, SM_SLID_T slid) {
    return hi->sm->symlist_buf + hi->sm->slrp_by_slid[slid];
}

pvt inline bool slCompare(SM_SLID_T *a, SM_SLID_T *b) {
    SM_SLID_T size;
    if ((size=a[0]) != b[0]) return 0;
    for (SM_SLID_T i=1; i<=size; i++) if (a[i] != b[i]) return 0;     // beware <= :)
    return 1;
}

pvt u32 sl_hash(SM_SLID_T *sl) {
    u32 n = sl[0] * sizeof(SM_SLID_T);
    m8 *s = (mem) sl;
    m8 *e = s + n;
    u32 hash = *s++;
    for (; s < e; s++) if (*s) hash = (hash << 5) - hash + *s;  // OPEN: explain why ignoring zeros
    return hash;
}

pvt bool inline slHashableFound(hi_struct(SM_SLID_BY_SLHASH) *hi, SM_SLID_T token, SM_SLID_T *hashable) {
    return slCompare(slFromSlid(hi, token), hashable);
}

// HI_IMPL(name, token_t, hashable_t, __hash_fn, __found_fn, __hashable_from_token_fn)
HI_IMPL(SM_SLID_BY_SLHASH, SM_SLID_T, btypeid_t *, sl_hash, slHashableFound, slFromSlid)


// ---------------------------------------------------------------------------------------------------------------------
// pretty printing
// pb - print buckets - return void
// pp - print pad - answer text pad node
// s8 - print s8 - answers an s8
// ---------------------------------------------------------------------------------------------------------------------

pvt void sm_buf_symlist(BK_SM *sm, BK_TP *tp, symid_t *symlist) {
    for (u32 i = 1; i < symlist[0] + 1; i++) {
        tp_buf_printf(tp, "`");
        tp_buf_printf(tp, sm_name(sm, symlist[i]));
    }
}

pvt inline TPN sm_pp_symlist(BK_SM *sm, BK_TP *tp, symid_t *sl) {sm_buf_symlist(sm, tp, sl); return tp_flush(tp);}
pvt inline S8 sm_s8_symlist(BK_SM *sm, BK_TP *tp, symid_t *sl) {sm_buf_symlist(sm, tp, sl); return tp_s8(tp, tp_flush(tp));}


// ---------------------------------------------------------------------------------------------------------------------
// sym manager lifecycle fns
// ---------------------------------------------------------------------------------------------------------------------

pub BK_SM * SM_create(BK_MM *mm) {
    BK_SM *sm = (BK_SM *) mm->malloc(sizeof(BK_SM));
    sm->mm = mm;

    // syms
    sm->symname_buf = os_vm_reserve(0, SM_MAX_NAME_STORAGE);
    sm->max_rp = os_page_size();
    os_mprotect(sm->symname_buf, sm->max_rp, BK_PROT_READ | BK_PROT_WRITE);     // make first page of name storage R/W
    os_madvise(sm->symname_buf, sm->max_rp, BK_MADV_RANDOM);                    // and advise as randomly accessed
    sm->max_symid = SM_MAX_SYM_ID_INC_SIZE;
    sm->next_symid = SM_NA_SYM + 1;
    sm->next_rp = 2;                                               // i.e. pointer to the char after the len prefix
    sm->rp_by_symid = mm->malloc(sm->max_symid * sizeof(RP));
    onOomDie(sm->rp_by_symid, s8("in %s malloc #1 failed"), FN_NAME);
    sm->sortorder_by_symid = mm->malloc(sm->max_symid * sizeof(symid_t));
    onOomDie(sm->sortorder_by_symid, s8("in %s malloc #2 failed"), FN_NAME);
    sm->symid_by_namehash = hi_create(SM_SYMID_BY_NAMEHASH);
    sm->symid_by_namehash->sm = sm;

    // symlists
    sm->symlist_buf = os_vm_reserve(0, SM_MAX_SL_STORAGE);
    sm->max_slrp = os_page_size() / sizeof(SM_SLID_T);
    os_mprotect(sm->symlist_buf, sm->max_slrp * sizeof(SM_SLID_T), BK_PROT_READ | BK_PROT_WRITE);   // make first page of symlist storage R/W
    os_madvise(sm->symlist_buf, sm->max_slrp * sizeof(SM_SLID_T), BK_MADV_RANDOM);                  // and advise as randomly accessed
    sm->next_slrp = 0;
    sm->max_slid = SM_MAX_SLID_INC_SIZE;
    sm->next_slid = 1;
    sm->slrp_by_slid = (RP *) mm->malloc(sm->max_slid * sizeof(RP));
    memset(sm->slrp_by_slid, 0, sm->max_slid * sizeof(RP));
    sm->slid_by_slhash = hi_create(SM_SLID_BY_SLHASH);
    sm->slid_by_slhash->sm = sm;

    return sm;
}

pub int SM_trash(BK_SM *sm) {
    // symlists
    os_vm_unreserve(sm->symlist_buf, SM_MAX_SL_STORAGE);
    sm->mm->free(sm->slrp_by_slid);
    hi_trash(SM_SLID_BY_SLHASH, sm->slid_by_slhash);

    // syms
    os_vm_unreserve(sm->symname_buf, SM_MAX_NAME_STORAGE);
    sm->mm->free(sm->rp_by_symid);
    sm->mm->free(sm->sortorder_by_symid);
    hi_trash(SM_SYMID_BY_NAMEHASH, sm->symid_by_namehash);

    // self
    sm->mm->free(sm);
    return 0;
}


// ---------------------------------------------------------------------------------------------------------------------
// utils
// ---------------------------------------------------------------------------------------------------------------------

pvt void _sm_sort_syms(BK_SM *sm) {
    // OPEN: do the sort
    sm->sortorder_by_symid[0] = SM_SYMS_SORTED;
}

pub bool sm_id_le(BK_SM *sm, symid_t a, symid_t b) {
    if (sm->sortorder_by_symid == SM_SYMS_NOT_SORTED) _sm_sort_syms(sm);
    return sm->sortorder_by_symid[a] < sm->sortorder_by_symid[b];
}


// ---------------------------------------------------------------------------------------------------------------------
// type accessing / creation fns
// ---------------------------------------------------------------------------------------------------------------------

pub symid_t sm_id(BK_SM *sm, char const *name) {
    i32 res, pageSize = 0;
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
    sm->next_rp += 2 + (i32)l + 1;

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

pub symid_t * sm_sl(BK_SM *sm, SM_SLID_T slid) {
    if (slid < 0 | slid > sm->next_slid) return 0;
    return sm->symlist_buf + sm->slrp_by_slid[slid];
}

pub SM_SLID_T sm_slid(BK_SM *sm, symid_t *symlist) {
    i32 outcome, i, numSyms = symlist[0];  u32 idx;  symid_t *p1, *nextSymlist;  SM_SLID_T slid;

    // validate contents of symlist
    // PP(info, "%s:%i numSyms=%i", FN_NAME, __LINE__, numSyms);
    if (!(numSyms = symlist[0]))
        return 0;

    for (i = 1; i <= numSyms; i++) {
        if (!(0 < symlist[i] && symlist[i] < sm->next_symid)) {
            PP(info, "%s:%i sym%i=%i not in range", FN_NAME, __LINE__, i, symlist[i]);
            return 0;
        }
    }
    // make next page of sm->typelist_buf writable if necessary
    if (sm->next_slrp + numSyms >= sm->max_slrp) {
        if (sm->next_slrp + numSyms >= TM_MAX_TL_STORAGE) die("%s: out of symlist storage", FN_NAME);  // OPEN: really we should add an error reporting mechanism, e.g. TM_ERR_OUT_OF_NAME_STORAGE, etc
        size_t pageSize = os_page_size();
        os_mprotect(sm->symlist_buf + sm->max_slrp, pageSize, BK_PROT_READ | BK_PROT_WRITE);
        os_madvise(sm->symlist_buf + sm->max_slrp, pageSize, BK_MADV_RANDOM);
    }

    nextSymlist = sm->symlist_buf + sm->next_slrp;

    // copy symlist into symlist_buf, including the size
    p1 = nextSymlist;
    *p1++ = numSyms;

    for (i = 1; i <= numSyms; i++) *p1++ = symlist[i];

    // get the slid for the symlist - adding if missing, returning 0 if invalid
    idx = hi_put_idx(SM_SLID_BY_SLHASH, sm->slid_by_slhash, symlist, &outcome);
    switch (outcome) {
        default:
            die("%s:%i: HI_TOMBSTONE1!", FN_NAME, __LINE__);
        case HI_LIVE:
            return sm->slid_by_slhash->tokens[idx];
        case HI_EMPTY:
            // PP(info, "%s:%i", FN_NAME, __LINE__);
            if ((slid = sm->next_slid++) >= sm->max_slid) {
                sm->max_slid += SM_RP_BY_SLID_INC_SIZE;
                sm->slrp_by_slid = sm->mm->realloc(sm->slrp_by_slid, sm->max_slid * sizeof(RP));
                onOomDie(sm->slrp_by_slid, s8("%s: realloc #1 failed"), FN_NAME);
            }
            sm->slrp_by_slid[slid] = sm->next_slrp;
            hi_replace_empty(SM_SLID_BY_SLHASH, sm->slid_by_slhash, idx, slid);
            if (sm->next_slrp + numSyms + 1 >= sm->max_slrp) {
                size_t pageSize = os_page_size();
                os_mprotect(sm->symlist_buf + sm->max_slrp - pageSize, pageSize, BK_PROT_WRITE);     // make the prior last page read only
                sm->max_slrp += pageSize / sizeof(TM_TLID_T);
            }
            sm->next_slrp += numSyms + 1;
            if (!slid) {
                PP(info, "%s:%i", FN_NAME, __LINE__);
                return 0;       // an error occurred OPEN handle properly
            }
            // PP(info, "%s:%i slid=%i len=%i", FN_NAME, __LINE__, slid, nextSymlist[0]);
            return slid;
    }

}



#endif // __BK_SM_C
