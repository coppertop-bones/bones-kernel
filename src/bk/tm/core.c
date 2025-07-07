// ---------------------------------------------------------------------------------------------------------------------
// Copyright 2025 David Briant, https://github.com/coppertop-bones. Licensed under the Apache License, Version 2.0
//
// CORE IMPLEMENTATION
// KEEPER REQUISITES: core
// ---------------------------------------------------------------------------------------------------------------------

#ifndef SRC_BK_TM_CORE_C
#define SRC_BK_TM_CORE_C "bk/tm/core.c"


#include "../../../include/bk/mm.h"
#include "../../../include/bk/tm.h"

#include "../lib/hi_impl.tmplt"
#include "../lib/radix.tmplt"
#include "../pp.c"


KRADIX_SORT_INIT(btypeid_t, btypeid_t, ,sizeof(btypeid_t))



// ---------------------------------------------------------------------------------------------------------------------
// TM_BTYPID_BY_SEQIDHASH fns - find the container btypeid from the contained btypeid (seqid)
// ---------------------------------------------------------------------------------------------------------------------

pvt inline TM_DETAILID_T seqidFromBtypeid(hi_struct(TM_BTYPID_BY_SEQIDHASH) *hi, btypeid_t containerbtypeid) {
    return TM_DETAILS_ID(hi->tm->btsummary_by_btypeid[containerbtypeid]);
}

pvt bool inline seqidHashableFound(hi_struct(TM_BTYPID_BY_SEQIDHASH) *hi, btypeid_t containerbtypeid, btypeid_t hashable) {
    // having hi->tm keeps summary hotter - good idea? slightly less memory and no need to maintain btypeid_by_seqid array
    btsummary sum = hi->tm->btsummary_by_btypeid[containerbtypeid];
    return TM_BMT_ID(sum) == bmtseq && TM_DETAILS_ID(sum) == hashable;
}

// HI_IMPL(name, token_t, hashable_t, __hash_fn, __found_fn, __hashable_from_token_fn)
HI_IMPL(TM_BTYPID_BY_SEQIDHASH, TM_DETAILID_T, btypeid_t, hi_int32_hash, seqidHashableFound, seqidFromBtypeid)


// ---------------------------------------------------------------------------------------------------------------------
// TM_DETAILID_BY_T1T2HASH fns - functions and maps
// ---------------------------------------------------------------------------------------------------------------------

pvt inline TM_T1T2 t1t2idFromXxxid(hi_struct(TM_DETAILID_BY_T1T2HASH) *hi, TM_DETAILID_T detailid) {
    return hi->t1t2_by_detailid[detailid];
}

pvt bool inline t1t2HashableFound(hi_struct(TM_DETAILID_BY_T1T2HASH) *hi, TM_DETAILID_T token, TM_T1T2 hashable) {
    TM_T1T2 t1t2 = hi->t1t2_by_detailid[token];
    return t1t2.t1 == hashable.t1 && t1t2.t2 == hashable.t2;
}

pvt u32 t1t2_hash(TM_T1T2 t1t2) {
    m8 *s = (mem) &t1t2;
    m8 *e = s + sizeof(TM_T1T2);
    u32 hash = *s++;
    for (; s < e; s++) if (*s) hash = (hash << 5) - hash + *s;  // OPEN: explain why ignoring zeros
    return hash;
}

// HI_IMPL(name, token_t, hashable_t, __hash_fn, __found_fn, __hashable_from_token_fn)
HI_IMPL(TM_DETAILID_BY_T1T2HASH, TM_DETAILID_T, TM_T1T2, t1t2_hash, t1t2HashableFound, t1t2idFromXxxid)


// ---------------------------------------------------------------------------------------------------------------------
// TM_DETAILID_BY_SLIDTLIDHASH fns - structs and records
// ---------------------------------------------------------------------------------------------------------------------

pvt inline TM_SLID_TLID slidtupidFromXxxid(hi_struct(TM_DETAILID_BY_SLIDTLIDHASH) *hi, TM_DETAILID_T detailid) {
    return (TM_SLID_TLID) {.slid = hi->tm->slid_by_strid[detailid], .tlid = hi->tm->tlid_by_strid[detailid]};
}

pvt bool inline slidtupidHashableFound(hi_struct(TM_DETAILID_BY_SLIDTLIDHASH) *hi, TM_DETAILID_T token, TM_SLID_TLID hashable) {
    SM_SLID_T slid = hi->tm->slid_by_strid[token];
    TM_TLID_T tlid = hi->tm->tlid_by_strid[token];
    return slid == hashable.slid && tlid == hashable.tlid;
}

pvt u32 slidtupid_hash(TM_SLID_TLID slid_tupid) {
    m8 *s = (mem) &slid_tupid;
    m8 *e = s + sizeof(TM_SLID_TLID);
    u32 hash = *s++;
    for (; s < e; s++) if (*s) hash = (hash << 5) - hash + *s;  // OPEN: explain why ignoring zeros
    return hash;
}

// HI_IMPL(name, token_t, hashable_t, __hash_fn, __found_fn, __hashable_from_token_fn)
HI_IMPL(TM_DETAILID_BY_SLIDTLIDHASH, TM_DETAILID_T, TM_SLID_TLID, slidtupid_hash, slidtupidHashableFound, slidtupidFromXxxid)


// ---------------------------------------------------------------------------------------------------------------------
// pretty printing
// pb - print buckets - return void
// pp - print pad - answer text pad node
// s8 - print s8 - answers an s8
// ---------------------------------------------------------------------------------------------------------------------

pvt void tm_pb(BK_TM *tm, BK_TP *tp, btypeid_t btypeid) {
    // print buckets the btype
    btsummary *sum;  symid_t symid;  btypeid_t *tl;  i32 i;  char sep;
    if ((symid = tm->symid_by_btypeid[btypeid])) {
        tp_buf_printf(tp, "%s", sm_name(tm->sm, symid));
    } else {
        sum = tm->btsummary_by_btypeid + btypeid;
        switch (TM_BMT_ID(*sum)) {
            case bmtatm:
                tp_buf_printf(tp, "%s", sm_name(tm->sm, symid));
                break;
            case bmtint:
                if (TM_IS_RECURSIVE(*sum)) {
                    tp_buf_printf(tp, "rec%i", btypeid);
                } else {
                    tl = tm->typelist_buf + tm->tlrp_by_tlid[tm->tlid_by_intid[TM_DETAILS_ID(*sum)]];
                    sep = 0;
                    for (i = 1; i <= (i32) tl[0]; i++) {
                        if (sep) tp_buf_printf(tp, " & ");
                        sep = 1;
                        tm_pb(tm, tp, tl[i]);
                    }
                }
                break;
            case bmttup:
                tp_buf_printf(tp, "tup");
                break;
            case bmtuni:
                tp_buf_printf(tp, "uni");
                break;
            default:
                tp_buf_printf(tp, "NAT");
        }
    }
}
pvt inline TPN tm_pp(BK_TM *tm, BK_TP *tp, btypeid_t btypeid) {tm_pb(tm, tp, btypeid); return tp_flush(tp);}
pvt inline S8 tm_s8(BK_TM *tm, BK_TP *tp, btypeid_t btypeid) {tm_pb(tm, tp, btypeid); return tp_s8(tp, tp_flush(tp));}

pvt void tm_buf_typelist(BK_TM *tm, BK_TP *tp, btypeid_t *typelist) {
    int firstTime = 1;
    for (u32 i = 1; i < typelist[0] + 1; i++) {
        if (firstTime) {
            firstTime = 0;
            tm_pb(tm, tp, typelist[i]);
        }
        else {
            tp_buf_printf(tp, ", ");
            tm_pb(tm, tp, typelist[i]);
        }
    }
}

pvt inline TPN tm_pp_typelist(BK_TM *tm, BK_TP *tp, btypeid_t *tl) {tm_buf_typelist(tm, tp, tl); return tp_flush(tp);}
pvt inline S8 tm_s8_typelist(BK_TM *tm, BK_TP *tp, btypeid_t *tl) {tm_buf_typelist(tm, tp, tl); return tp_s8(tp, tp_flush(tp));}


// ---------------------------------------------------------------------------------------------------------------------
// TM_BTYPEID_BY_SYMIDHASH fns
// ---------------------------------------------------------------------------------------------------------------------

pvt inline symid_t symidFromBtypeid(hi_struct(TM_BTYPEID_BY_SYMIDHASH) *hi, btypeid_t btypeid) {
    return hi->tm->symid_by_btypeid[btypeid];
}

pvt bool inline symidHashableFound(hi_struct(TM_BTYPEID_BY_SYMIDHASH) *hi, btypeid_t token, symid_t hashable) {
    return hi->tm->symid_by_btypeid[token] == hashable;
}

// HI_IMPL(name, token_t, hashable_t, __hash_fn, __found_fn, __hashable_from_token_fn)
HI_IMPL(TM_BTYPEID_BY_SYMIDHASH, btypeid_t, symid_t, hi_int32_hash, symidHashableFound, symidFromBtypeid)


// ---------------------------------------------------------------------------------------------------------------------
// TM_TLID_BY_TLHASH fns
// ---------------------------------------------------------------------------------------------------------------------

pvt inline btypeid_t * tlFromTlid(hi_struct(TM_TLID_BY_TLHASH) *hi, TM_TLID_T tlid) {
    return hi->tm->typelist_buf + hi->tm->tlrp_by_tlid[tlid];
}

pvt inline bool tlCompare(btypeid_t *a, btypeid_t *b) {
    btypeid_t size;
    if ((size=a[0]) != b[0]) return 0;
    for (btypeid_t i=1; i<=size; i++) if (a[i] != b[i]) return 0;     // beware <= :)
    return 1;
}

pvt u32 tl_hash(btypeid_t *tl) {
    u32 n = tl[0] * sizeof(btypeid_t);
    m8 *s = (mem) tl;
    m8 *e = s + n;
    u32 hash = *s++;
    for (; s < e; s++) if (*s) hash = (hash << 5) - hash + *s;  // OPEN: explain why ignoring zeros
    return hash;
}

pvt bool inline tlHashableFound(hi_struct(TM_TLID_BY_TLHASH) *hi, TM_TLID_T token, btypeid_t *hashable) {
    return tlCompare(tlFromTlid(hi, token), hashable);
}

// HI_IMPL(name, token_t, hashable_t, __hash_fn, __found_fn, __hashable_from_token_fn)
HI_IMPL(TM_TLID_BY_TLHASH, TM_TLID_T, btypeid_t *, tl_hash, tlHashableFound, tlFromTlid)


// ---------------------------------------------------------------------------------------------------------------------
// TM_DETAILID_BY_TLIDHASH fns
// ---------------------------------------------------------------------------------------------------------------------

pvt inline TM_TLID_T tlidFromXxxid(hi_struct(TM_DETAILID_BY_TLIDHASH) *hi, TM_DETAILID_T detailid) {
    return hi->tlid_by_detailid[detailid];
}

pvt bool inline tlidHashableFound(hi_struct(TM_DETAILID_BY_TLIDHASH) *hi, TM_DETAILID_T token, TM_TLID_T hashable) {
    return hi->tlid_by_detailid[token] == hashable;
}

// HI_IMPL(name, token_t, hashable_t, __hash_fn, __found_fn, __hashable_from_token_fn)
HI_IMPL(TM_DETAILID_BY_TLIDHASH, TM_DETAILID_T, TM_TLID_T, hi_int32_hash, tlidHashableFound, tlidFromXxxid)


// ---------------------------------------------------------------------------------------------------------------------
// utils
// ---------------------------------------------------------------------------------------------------------------------

pvt void _growTo(void **p, size_t size, BK_MM *mm, char *fnName) {
    void *t = *p;
    t = mm->realloc(t, size);
    onOomDie(t, s8("%s: realloc #1 failed"), fnName);
    *p = t;
}

tdd TM_TLID_T _commit_typelist_buf_at(BK_TM *tm, TM_TLID_T *typelist, u32 idx) {
    TM_TLID_T tlid, numTypes;
    if ((tlid = tm->next_tlid++) >= tm->max_tlid) {
        tm->max_tlid += TM_RP_BY_TLID_INC_SIZE;
        _growTo((void **)&tm->tlrp_by_tlid, tm->max_tlid * sizeof(RP), tm->mm, FN_NAME);
        tm->intid_by_tlidhash->tlid_by_detailid = tm->tlid_by_intid;  // update the tm->intid_by_tlidhash with the new buffer
    }
    tm->tlrp_by_tlid[tlid] = tm->next_tlrp;
    hi_replace_empty(TM_TLID_BY_TLHASH, tm->tlid_by_tlhash, idx, tlid);
    numTypes = typelist[0];
    if (tm->next_tlrp + numTypes + 1 >= tm->max_tlrp) {
        // make the prior last page read only if we've gone over a page boundary (to protect type list from accidental mutation)
        size_t pageSize = os_page_size();
        os_mprotect(tm->typelist_buf + tm->max_tlrp - pageSize, pageSize, BK_PROT_READ);
        tm->max_tlrp += pageSize / sizeof(TM_TLID_T);
    }
    tm->next_tlrp += numTypes + 1;
    return tlid;
}

tdd btypeid_t _update_type_summary(BK_TM *tm, btypeid_t btypeid, u32 detailsid, u16 sz, bool hasT) {
    // OPEN: store space and size by details_id in the relevant slots (growing if necessary)
    if (btypeid >= tm->max_btypeId) tm_reserve_block(tm, btypeid);  // grow if necessary
    tm->btsummary_by_btypeid[btypeid] |=
            detailsid |
            (sz ? TM_IS_MEM_MASK : 0) |
            (hasT ? TM_HAS_T_MASK : 0);
    if (btypeid >= tm->next_btypeId) tm->next_btypeId = btypeid + 1;
    return btypeid;
}


pvt void _make_next_page_of_typelist_buf_writable_if_necessary(BK_TM *tm, int numSlots) {
    // make next page of tm->typelist_buf writable if necessary
    // OPEN: handle the unlikely requirement of needing more than os_page_size() of extra memory
    if (tm->next_tlrp + numSlots >= tm->max_tlrp) {
        if (tm->next_tlrp + numSlots >= TM_MAX_TL_STORAGE) die("%s: out of typelist storage", FN_NAME);  // OPEN: really we should add an error reporting mechanism, e.g. TM_ERR_OUT_OF_NAME_STORAGE, etc
        size_t pageSize = os_page_size();
        os_mprotect(tm->typelist_buf + tm->max_tlrp, pageSize, BK_PROT_READ | BK_PROT_WRITE);
        os_madvise(tm->typelist_buf + tm->max_tlrp, pageSize, BK_MADV_RANDOM);
    }
}


// ---------------------------------------------------------------------------------------------------------------------
// error reporting
// ---------------------------------------------------------------------------------------------------------------------

pvt btypeid_t setErrAndDesc(btypeid_t err, char const *msg, char const *filename, long lineno, ...) {
    va_list args;
    va_start(args, lineno);
    fprintf(stderr, "  File \"%s\", line %li, \"", filename, lineno);
    vfprintf(stderr, msg, args);
    fprintf(stderr, "\"\n");
    va_end(args);
    return err;
}

pvt btypeid_t _err_emptyTypelist(btypeid_t ret, char const *filename, char const * fnname, long lineno) {
    return setErrAndDesc(ret, "typelist is empty", filename, lineno, fnname);
}

pvt btypeid_t _err_itemInTLOutOfRange(btypeid_t ret, char const *filename, char const * fnname, long lineno, btypeid_t t, int offset) {
    return setErrAndDesc(ret, "%s type %i (t%i) is out of btypeId range", filename, lineno, fnname, offset, t);
}

pvt btypeid_t _err_invalid_btype_B_NAT(btypeid_t ret, char const *filename, char const * fnname, long lineno) {
    return setErrAndDesc(ret, "btypeid is B_NAT", filename, lineno, fnname);
}

pvt btypeid_t _btypeInTypeListNotInitialised(btypeid_t ret, char const *filename, long lineno, btypeid_t t, int offset) {
    return setErrAndDesc(ret, "type %i (t%i) is out initialised", filename, lineno, offset, t);
}

pvt btypeid_t _seriousErrorCommitingTypelistBufHandleProperly(btypeid_t ret, char const *filename, long lineno) {
    return setErrAndDesc(ret, "Serious error committing typelist buf - TODO pp list here", filename, lineno);
}

pvt btypeid_t _err_otherAlreadyRepresentsTL(btypeid_t ret, char const *filename, long lineno, btypeid_t self, btypeid_t other) {
    // OPEN: add pp of tl and name of other
    return setErrAndDesc(B_NAT, "Self (t%i) - typelist already represented by t%i", filename, lineno, self, other);
}

pvt btypeid_t _err_btypeAlreadyInitialised(btypeid_t ret, char const *filename, long lineno, btypeid_t self) {
    // OPEN: add pp name of self
    return setErrAndDesc(B_NAT, "Self (t%i) - already initialised", __FILE__, __LINE__, self);
}
pvt btypeid_t _err_btypeidOutOfRange(btypeid_t ret, char const *filename, char const * fnname, long lineno, btypeid_t self) {
    return setErrAndDesc(ret, "%s self t%i is out of btypeId range", filename, lineno, fnname, self);
}

#endif  // SRC_BK_TM_CORE_C