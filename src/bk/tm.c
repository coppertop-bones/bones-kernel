// ---------------------------------------------------------------------------------------------------------------------
// TM - TYPE MANAGER
// KEEPER REQUISITES: core
// ---------------------------------------------------------------------------------------------------------------------

#ifndef __BK_TM_C
#define __BK_TM_C "bk/tm.c"


#include "../../include/bk/mm.h"
#include "../../include/bk/tm.h"
#include "../../include/bk/tp.h"
#include "lib/hi_impl.tmplt"
#include "lib/radix.tmplt"

#include "pp.c"


KRADIX_SORT_INIT(btypeid_t, btypeid_t, ,sizeof(btypeid_t))


// forward declares to maintain nicer code order
pvt btypeid_t _inter_for_emplaced_tl(BK_TM *tm, btypeid_t btypeid);
pvt btypeid_t _union_for_emplaced_tl(BK_TM *tm, btypeid_t btypeid);


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
// TM_DETAILID_BY_SLIDTUPIDHASH fns - structs and records
// ---------------------------------------------------------------------------------------------------------------------

pvt inline TM_SLID_TUPID slidtupidFromXxxid(hi_struct(TM_DETAILID_BY_SLIDTUPIDHASH) *hi, TM_DETAILID_T detailid) {
    return (TM_SLID_TUPID) {.slid = hi->tm->slid_by_strid[detailid], .tupid = hi->tm->tupid_by_strid[detailid]};
}

pvt bool inline slidtupidHashableFound(hi_struct(TM_DETAILID_BY_SLIDTUPIDHASH) *hi, TM_DETAILID_T token, TM_SLID_TUPID hashable) {
    SM_SLID_T slid = hi->tm->slid_by_strid[token];
    TM_TLID_T tupid = hi->tm->tupid_by_strid[token];
    return slid == hashable.slid && tupid == hashable.tupid;
}

pvt u32 slidtupid_hash(TM_SLID_TUPID slid_tupid) {
    m8 *s = (mem) &slid_tupid;
    m8 *e = s + sizeof(TM_SLID_TUPID);
    u32 hash = *s++;
    for (; s < e; s++) if (*s) hash = (hash << 5) - hash + *s;  // OPEN: explain why ignoring zeros
    return hash;
}

// HI_IMPL(name, token_t, hashable_t, __hash_fn, __found_fn, __hashable_from_token_fn)
HI_IMPL(TM_DETAILID_BY_SLIDTUPIDHASH, TM_DETAILID_T, TM_SLID_TUPID, slidtupid_hash, slidtupidHashableFound, slidtupidFromXxxid)


// ---------------------------------------------------------------------------------------------------------------------
// utils
// ---------------------------------------------------------------------------------------------------------------------

pvt void _growTo(void **p, size_t size, BK_MM *mm, char *fnName) {
    void *t = *p;
    t = mm->realloc(t, size);
    onOomDie(t, s8("%s: realloc #1 failed"), fnName);
    *p = t;
}

tdd TM_TLID_T _commit_typelist_buf_at(BK_TM *tm, TM_TLID_T numTypes, u32 idx) {
    TM_TLID_T tlid;
    if ((tlid = tm->next_tlid++) >= tm->max_tlid) {
        tm->max_tlid += TM_RP_BY_TLID_INC_SIZE;
        _growTo((void **)&tm->tlrp_by_tlid, tm->max_tlid * sizeof(RP), tm->mm, FN_NAME);
        tm->intid_by_tlidhash->tlid_by_detailid = tm->tlid_by_intid;  // update the tm->intid_by_tlidhash with the new buffer
    }
    tm->tlrp_by_tlid[tlid] = tm->next_tlrp;
    hi_replace_empty(TM_TLID_BY_TLHASH, tm->tlid_by_tlhash, idx, tlid);
    if (tm->next_tlrp + numTypes + 1 >= tm->max_tlrp) {
        size_t pageSize = os_page_size();
        os_mprotect(tm->typelist_buf + tm->max_tlrp - pageSize, pageSize, BK_PROT_WRITE);     // make the prior last page read only
        tm->max_tlrp += pageSize / sizeof(TM_TLID_T);
    }
    tm->next_tlrp += numTypes + 1;
    return tlid;
}

tdd void _new_type_summary_at(BK_TM *tm, btypeid_t self, bmetatypeid_t bmtid, btypeid_t familyid, u32 details_id, u16 sz, bool hasT) {
    // OPEN: do we restrict the range of directly assigned btypeids?
    // OPEN: store family and size by details_id in the relevant slots (growing if necessary)
    while (self >= tm->max_btypeId) {
        tm->max_btypeId += TM_MAX_BTYPEID_INC_SIZE;
        _growTo((void **)&tm->btsummary_by_btypeid, tm->max_btypeId * sizeof(btsummary), tm->mm, FN_NAME);
        _growTo((void **)&tm->familyid_by_btypeid, tm->max_btypeId * sizeof(btypeid_t), tm->mm, FN_NAME);
        _growTo((void **)&tm->symid_by_btypeid, tm->max_btypeId * sizeof(symid_t), tm->mm, FN_NAME);
    }
    tm->btsummary_by_btypeid[self] =
        details_id |
        bmtid |
        (familyid ? TM_IN_FAMILY_MASK : 0) |
        (hasT ? TM_HAS_T_MASK : 0) |
        (sz ? TM_IS_MEM_MASK : 0);
    tm->familyid_by_btypeid[self] = familyid;
    if (self >= tm->next_btypeId) tm->next_btypeId = self + 1;
}

tdd inline void _set_recursive(BK_TM *tm, btypeid_t self) {
    tm->btsummary_by_btypeid[self] |= TM_IS_RECURSIVE_MASK;
}


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
            case bmtnom:
                tp_buf_printf(tp, "%s", sm_name(tm->sm, symid));
                break;
            case bmtint:
                tl = tm->typelist_buf + tm->tlrp_by_tlid[tm->tlid_by_intid[TM_DETAILS_ID(*sum)]];
                sep = 0;
                for (i = 1; i <= (i32) tl[0]; i++) {
                    if (sep) tp_buf_printf(tp, " & ");
                    sep = 1;
                    tm_pb(tm, tp, tl[i]);
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
// type accessing / creation fns
// ---------------------------------------------------------------------------------------------------------------------

pub bmetatypeid_t tm_bmetatypeid(BK_TM *tm, btypeid_t btypeid) {
    if (btypeid < 1 || btypeid >= tm->next_btypeId) return 0;
    return TM_BMT_ID(tm->btsummary_by_btypeid[btypeid]);
}

pub btypeid_t tm_btypeid(BK_TM *tm, char const *name) {
    int outcome;  u32 idx;
    idx = hi_put_idx(TM_BTYPEID_BY_SYMIDHASH, tm->btypeid_by_symidhash, sm_id(tm->sm, name), &outcome);
    if (outcome == HI_LIVE)
        return tm->btypeid_by_symidhash->tokens[idx];
    else
        return B_NAT;
}

pub btypeid_t tm_exp(BK_TM *tm, btypeid_t self) {
    // sets the type to be explicit and answers self
    tm->btsummary_by_btypeid[self] |= TM_IS_EXPLICIT_MASK;
    return self;
}

pub btypeid_t tm_fn(BK_TM *tm, btypeid_t tArgs, btypeid_t tRet, btypeid_t self) {
    i32 outcome;  TM_DETAILID_T fncid;  TM_T1T2 t1t2;  u32 idx;  bool hasT;

    // answers the validated function type corresponding to tArgs and tRet, creating if necessary

    // check each typeid is valid
    if (!(0 < tArgs && tArgs < tm->next_btypeId)) return B_NAT;
    if (TM_BMT_ID(tm->btsummary_by_btypeid[tArgs]) != bmttup) return B_NAT;
    if (!(0 < tRet && tRet < tm->next_btypeId)) return B_NAT;
    if (TM_BMT_ID(tm->btsummary_by_btypeid[tRet]) == bmterr) return B_NAT;

    t1t2.tArgs = tArgs;
    t1t2.tRet = tRet;

    // get the btypeid for the t1t2
    idx = hi_put_idx(TM_DETAILID_BY_T1T2HASH, tm->fncid_by_t1t2hash, t1t2, &outcome);
    switch (outcome) {
        default:
            die("%s:%i: HI_TOMBSTONE2!", FN_NAME, __LINE__);
        case HI_LIVE:
            fncid = tm->fncid_by_t1t2hash->tokens[idx];
            if (self == 0) return tm->btypid_by_fncid[fncid];
            else if (self == tm->btypid_by_fncid[fncid]) return self;
            else return B_NAT;
        case HI_EMPTY:
            // missing so commit the function type for t1t2
            if (self == 0)
                self = tm->next_btypeId;
            else if (self < tm->next_btypeId && TM_BMT_ID(tm->btsummary_by_btypeid[self]) != bmterr)
                // self is already in use so given the t1t2 lookup above we cannot be referring to the same btype
                return B_NAT;
            fncid = tm->next_fncid++;
            if (fncid >= tm->max_fncid) {
                tm->max_fncid += TM_MAX_ID_INC_SIZE;
                _growTo((void **)&tm->t1t2_by_fncid, tm->max_fncid * sizeof(TM_T1T2), tm->mm, FN_NAME);
                _growTo((void **)&tm->btypid_by_fncid, tm->max_fncid * sizeof(btypeid_t), tm->mm, FN_NAME);
            }
            tm->t1t2_by_fncid[fncid] = t1t2;
            hasT = TM_HAS_T(tm->btsummary_by_btypeid[tArgs]) || TM_HAS_T(tm->btsummary_by_btypeid[tRet]);
            _new_type_summary_at(tm, self, bmtfnc, B_NAT, fncid, 0, hasT);
            tm->btypid_by_fncid[fncid] = self;
            hi_replace_empty(TM_DETAILID_BY_T1T2HASH, tm->fncid_by_t1t2hash, idx, fncid);
            return self;
    }
}

pub TM_T1T2 tm_fn_targs_tret(BK_TM *tm, btypeid_t btypeid) {
    if (btypeid < 1 || btypeid >= tm->next_btypeId) return (TM_T1T2) {{0}, {0}};
    btsummary *sum = tm->btsummary_by_btypeid + btypeid;       // OPEN: in general use pointer to summary rather than copying the struct
    if (TM_BMT_ID(*sum) != bmtfnc) return (TM_T1T2) {{0}, {0}};
    return tm->t1t2_by_fncid[TM_DETAILS_ID(*sum)];
}

// set of values intersection ((1 2 3) + (4 5)) & ((1 2 3) + (6 7)) = (1 2 3 4 5) & (1 2 3 6 7) = (1 2 3)
// (int + str) & (int + bool) => (int+int) & (int+bool) & (str+int) & (str+bool)
// types only make sense in the context of fitsWithin a LHS might not behaviour as a RHS

pvt void _make_next_page_of_typelist_buf_writable_if_necessary(BK_TM *tm, int numTypes) {
    // make next page of tm->typelist_buf writable if necessary
    if (tm->next_tlrp + numTypes >= tm->max_tlrp) {
        if (tm->next_tlrp + numTypes >= TM_MAX_TL_STORAGE) die("%s: out of typelist storage", FN_NAME);  // OPEN: really we should add an error reporting mechanism, e.g. TM_ERR_OUT_OF_NAME_STORAGE, etc
        size_t pageSize = os_page_size();
        os_mprotect(tm->typelist_buf + tm->max_tlrp, pageSize, BK_PROT_READ | BK_PROT_WRITE);
        os_madvise(tm->typelist_buf + tm->max_tlrp, pageSize, BK_MADV_RANDOM);
    }
}

pub bool tm_hasT(BK_TM *tm, btypeid_t btypeid) {
    if (btypeid < 1 || btypeid >= tm->next_btypeId) return 0;
    return TM_HAS_T(tm->btsummary_by_btypeid[btypeid]);
}

// set of values intersection ((1 2 3) + (4 5)) & ((1 2 3) + (6 7)) = (1 2 3 4 5) & (1 2 3 6 7) = (1 2 3)
// (int + str) & (int + bool) => (int+int) & (int+bool) & (str+int) & (str+bool)
// types only make sense in the context of fitsWithin a LHS might not behaviour as a RHS

pub btypeid_t tm_inter(BK_TM *tm, btypeid_t self, btypeid_t *typelist) {
    i32 i, j, numTypes, hasUnions;  TM_TLID_T tlid;  btypeid_t *interTl, *p1, *p2, *p3, *nextTypelist;
    btsummary *sum;
    // (A&B) & (C&D)  = A & B & C & D
    // (A&B) & (B&C)  = A & B & C
    // (A+B) & (B+C)  = (A+B) & (B+C)  why not B? because we need to keep the detail when the program causes intersections
    //
    // (A&B) + (B&C)  = B & (A + C)    - not the same for unions of intersections

    // use tm->typelist_buf as scratch so don't have to allocate memory
    // OPEN: potentially though messy we could do intersections without child intersections in place in typelist to keep a little cache locality

    if (!(numTypes = typelist[0])) return B_NAT;

    // check btypeids in typelist are in range, and figure total possible length (including possible duplicate from child intersections)
    for (i = 1; i <= (i32) typelist[0]; i++) {
        if (!(0 < typelist[i] && typelist[i] < tm->next_btypeId)) return B_NAT;
        sum = tm->btsummary_by_btypeid + typelist[i];
        if (TM_BMT_ID(*sum) == bmtint) {
            tlid = tm->tlid_by_intid[TM_DETAILS_ID(*sum)];
            numTypes += (tm->typelist_buf + tm->tlrp_by_tlid[tlid])[0] - 1;
        }
    }

    _make_next_page_of_typelist_buf_writable_if_necessary(tm, numTypes);

    nextTypelist = tm->typelist_buf + tm->next_tlrp;

    // copy typelist into typelist_buf unpacking any intersections
    p1 = nextTypelist;
    *p1++ = numTypes;
    for (i = 1; i <= (i32) typelist[0]; i++) {
        sum = tm->btsummary_by_btypeid + typelist[i];
        if (TM_BMT_ID(*sum) == bmtint) {
            // we have an intersection type - expand it
            tlid = tm->tlid_by_intid[TM_DETAILS_ID(*sum)];
            interTl = (tm->typelist_buf + tm->tlrp_by_tlid[tlid]);
            for (j = 1; j <= (i32) interTl[0]; j++) *p1++ = interTl[j];
        } else
            *p1++ = typelist[i];
    }

    // sort types into btypeid order
    ks_radix_sort(btypeid_t, nextTypelist + 1, numTypes);

    // eliminate duplicates + check for unions
    p1 = nextTypelist + 1;
    p2 = p1 + 1;
    p3 = p1 + numTypes;
    hasUnions = TM_BMT_ID(tm->btsummary_by_btypeid[*p1]) == bmtuni;
    while (p2 < p3) {
        if (*p1 != *p2)
            *++p1 = *p2++;
        else
            while (*p1 == *p2 && p2 < p3) p2++;
        hasUnions |= TM_BMT_ID(tm->btsummary_by_btypeid[*p1]) == bmtuni;
    }
    numTypes = *nextTypelist = p1 - nextTypelist;

    // handle intersections of unions?
    if (hasUnions) return 0;

    return _inter_for_emplaced_tl(tm, self);
}
pvt btypeid_t _inter_for_emplaced_tl(BK_TM *tm, btypeid_t self) {
    u32 idx;  i32 i, outcome;  TM_TLID_T tlid;  TM_DETAILID_T intid;  int excl = 0;  btypeid_t *p1;  bool hasT;
    btsummary *sum;  btypeid_t *nextTypelist; i32 numTypes;

    nextTypelist = tm->typelist_buf + tm->next_tlrp;
    numTypes = *nextTypelist;

    // check for exclusion conflicts
    p1 = nextTypelist;
    hasT = false;
    for (i = 1; i <= numTypes; i++) {
        sum = tm->btsummary_by_btypeid + p1[i];
//        if (excl & sum->excl) return 0;    OPEN: fix here
        hasT = hasT || TM_HAS_T(*sum);
//        excl |= sum->excl;                OPEN: fix here
    }

    // get the tlid for the typelist - adding if missing, returning 0 if invalid
    idx = hi_put_idx(TM_TLID_BY_TLHASH, tm->tlid_by_tlhash, nextTypelist, &outcome);
    switch (outcome) {
        default:
            die("%s: HI_TOMBSTONE1!", FN_NAME);
        case HI_LIVE:
            tlid = tm->tlid_by_tlhash->tokens[idx];
            break;
        case HI_EMPTY:
            tlid = _commit_typelist_buf_at(tm, numTypes, idx);
            if (!tlid) return 0;       // an error occurred OPEN handle properly
    }

    // get the btypeid for the tlid
    idx = hi_put_idx(TM_DETAILID_BY_TLIDHASH, tm->intid_by_tlidhash, tlid, &outcome);
    switch (outcome) {
        default:
            die("%s: HI_TOMBSTONE2!", FN_NAME);
        case HI_LIVE:
            // typelist already exists
            intid = tm->intid_by_tlidhash->tokens[idx];
            if (self == 0) return tm->btypid_by_intid[intid];
            else if (self == tm->btypid_by_intid[intid]) return self;
            else return 0;
        case HI_EMPTY:
            // missing so commit the intersection type for tlid
            if (self == 0)
                self = tm->next_btypeId;
            else if (self < tm->next_btypeId && TM_BMT_ID(tm->btsummary_by_btypeid[self]) != bmterr)
                // self is already in use so given the type list lookup above we cannot be referring to the same btype
                return B_NAT;
            intid = tm->next_intid++;
            if (intid >= tm->max_intid) {
                tm->max_intid += TM_MAX_ID_INC_SIZE;
                _growTo((void **)&tm->tlid_by_intid, tm->max_intid * sizeof(TM_TLID_T), tm->mm, FN_NAME);
                _growTo((void **)&tm->btypid_by_intid, tm->max_intid * sizeof(btypeid_t), tm->mm, FN_NAME);
            }
            tm->tlid_by_intid[intid] = tlid;
            _new_type_summary_at(tm, self, bmtint, excl, intid, 0, hasT);
            tm->btypid_by_intid[intid] = self;
            hi_replace_empty(TM_DETAILID_BY_TLIDHASH, tm->intid_by_tlidhash, idx, intid);
            return self;
    }
}

pub btypeid_t * tm_inter_tl(BK_TM *tm, btypeid_t btypeid) {
    btsummary *sum;
    sum = tm->btsummary_by_btypeid + btypeid;
    if (TM_BMT_ID(*sum) == bmtint) {
        return tm->typelist_buf + tm->tlrp_by_tlid[tm->tlid_by_intid[TM_DETAILS_ID(*sum)]];
    } else {
        return 0;
    }
}

pub btypeid_t tm_inter_v(BK_TM *tm, btypeid_t self, u32 numTypes, ...) {
    va_list args;  btypeid_t *typelist;  int i;  btypeid_t btypeid;
    va_start(args, numTypes);
    typelist = malloc((1 + numTypes) * sizeof(btypeid_t));
    for (i = 1; i <= numTypes; i++) typelist[i] = va_arg(args, btypeid_t);
    typelist[0] = numTypes;
    btypeid = tm_inter(tm, self, typelist);
    free(typelist);
    va_end(args);
    return btypeid;
}

pvt btypeid_t _tm_inter_v2(BK_TM *tm, btypeid_t self, u32 numTypes, btypeid_t *args) {
    btypeid_t *typelist;  int i;  btypeid_t btypeid;
    typelist = malloc((1 + numTypes) * sizeof(btypeid_t));      // OPEN: use a typelist buffer of big enough size
    for (i = 1; i <= numTypes; i++) typelist[i] = args[i-1];
    typelist[0] = numTypes;
    btypeid = tm_inter(tm, self, typelist);
    free(typelist);
    return btypeid;
}

pub btypeid_t tm_map(BK_TM *tm, btypeid_t tK, btypeid_t tV, btypeid_t btypeid) {
    i32 outcome;  TM_DETAILID_T mapid;  TM_T1T2 t1t2;  u32 idx;  bool hasT;

    // answers the validated map type corresponding to tK and tV, creating if necessary

    // check each typeid is valid
    if (!(0 < tK && tK < tm->next_btypeId)) return B_NAT;
    if (TM_BMT_ID(*(tm->btsummary_by_btypeid + tK)) == bmterr) return B_NAT;
    if (!(0 < tV && tV < tm->next_btypeId)) return B_NAT;
    if (TM_BMT_ID(*(tm->btsummary_by_btypeid + tV)) == bmterr) return B_NAT;

    t1t2.tK = tK;
    t1t2.tV = tV;

    // get the btypeid for the t1t2
    idx = hi_put_idx(TM_DETAILID_BY_T1T2HASH, tm->mapid_by_t1t2hash, t1t2, &outcome);
    switch (outcome) {
        default:
            die("%s:%i: HI_TOMBSTONE2!", FN_NAME, __LINE__);
        case HI_LIVE:
            mapid = tm->mapid_by_t1t2hash->tokens[idx];
            if (btypeid == 0) return tm->btypid_by_mapid[mapid];
            else if (btypeid == tm->btypid_by_mapid[mapid]) return btypeid;
            else return 0;
        case HI_EMPTY:
            // missing so commit the function type for t1t2
            if (btypeid == 0)
                btypeid = tm->next_btypeId;
            else if (btypeid < tm->next_btypeId && TM_BMT_ID(tm->btsummary_by_btypeid[btypeid]) != bmterr)
                // btypeid is already in use so given the t1t2 lookup above we cannot be referring to the same btype
                return B_NAT;
            mapid = tm->next_mapid++;
            if (mapid >= tm->max_mapid) {
                tm->max_mapid += TM_MAX_ID_INC_SIZE;
                _growTo((void **)&tm->t1t2_by_mapid, tm->max_mapid * sizeof(TM_T1T2), tm->mm, FN_NAME);
                _growTo((void **)&tm->btypid_by_mapid, tm->max_mapid * sizeof(btypeid_t), tm->mm, FN_NAME);
            }
            tm->t1t2_by_mapid[mapid] = t1t2;
            hasT = TM_HAS_T(tm->btsummary_by_btypeid[tK]) || TM_HAS_T(tm->btsummary_by_btypeid[tV]);
            _new_type_summary_at(tm, btypeid, bmtmap, B_NAT, mapid, 0, hasT);
            tm->btypid_by_mapid[mapid] = btypeid;
            hi_replace_empty(TM_DETAILID_BY_T1T2HASH, tm->mapid_by_t1t2hash, idx, mapid);
            return btypeid;
    }
}

pub TM_T1T2 tm_map_tk_tv(BK_TM *tm, btypeid_t btypeid) {
    if (btypeid < 1 || btypeid >= tm->next_btypeId) return (TM_T1T2) {{0}, {0}};
    btsummary *sum = tm->btsummary_by_btypeid + btypeid;       // OPEN: in general use pointer to summary rather than copying the struct
    if (TM_BMT_ID(*sum) != bmtmap) return (TM_T1T2) {{0}, {0}};
    return tm->t1t2_by_mapid[TM_DETAILS_ID(*sum)];
}

pub btypeid_t tm_minus(BK_TM *tm, btypeid_t A, btypeid_t B, btypeid_t btypeid) {
    btsummary *sumA, *sumB; btypeid_t *tlA1, *tlB1, *tlA2, *tlB2, *tlDest1, *tlDest2, *p;  int nA, nB, nDest;

    if (!(0 < A && A < tm->next_btypeId)) return B_NAT;
    if (!(0 < B && B < tm->next_btypeId)) return B_NAT;
    sumA = (tm->btsummary_by_btypeid + A);
    sumB = (tm->btsummary_by_btypeid + B);
    if ((TM_BMT_ID(*sumA) != bmtint && TM_BMT_ID(*sumA) != bmtuni) || TM_BMT_ID(*sumB) == bmterr) return B_NAT;

    // A is either an intersection or a union - the minus operation is essentially the same
    if (TM_BMT_ID(*sumA) == bmtint)
        tlA1 = tm->typelist_buf + tm->tlrp_by_tlid[tm->tlid_by_intid[TM_DETAILS_ID(*sumA)]];     // points to size element in tlA
    else
        tlA1 = tm->typelist_buf + tm->tlrp_by_tlid[tm->tlid_by_uniid[TM_DETAILS_ID(*sumA)]];     // points to size element in tlA
    nA = *tlA1;
    tlA2 = tlA1 + nA;                                                                   // points to last element in tlA
    _make_next_page_of_typelist_buf_writable_if_necessary(tm, nA);
    tlDest2 = tlDest1 = tm->typelist_buf + tm->next_tlrp;                               // both point to size element in tlDest
    if (TM_BMT_ID(*sumB) == bmtint || TM_BMT_ID(*sumB) == bmtuni) {
        if (TM_BMT_ID(*sumB) == bmtint)
            tlB1 = tm->typelist_buf + tm->tlrp_by_tlid[tm->tlid_by_intid[TM_DETAILS_ID(*sumB)]]; // points to size element in tlB
        else
            tlB1 = tm->typelist_buf + tm->tlrp_by_tlid[tm->tlid_by_uniid[TM_DETAILS_ID(*sumB)]]; // points to size element in tlB
        nB = *tlB1;
        tlB2 = tlB1 + nB;                                                               // points to last element in tlB
        ++tlB1;
        // OPEN: this can be made a bit more efficient by using the fact that union and intersection typelists are sorted
        for (++tlA1; tlA1 <= tlA2; tlA1++) {
            int match = 0;
            for (p = tlB1; p <= tlB2; p++) {
                if (*tlA1 == *p) {
                    match = 1;
                    break;
                }
            }
            if (!match) *(++tlDest2) = *tlA1;    // if no match inc dest ptr and copy
        }
    }
    else {
        nB = 1;
        for (++tlA1; tlA1 <= tlA2; tlA1++) if (*tlA1 != B) *(++tlDest2) = *tlA1;    // if no match inc dest ptr and copy
    }
    nDest = tlDest2 - tlDest1;
    if (nDest + nB != nA) return B_NAT;
    if (nDest == 1) return *tlDest2;
    *tlDest1 = nDest;
    return (TM_BMT_ID(*sumA) == bmtint) ? _inter_for_emplaced_tl(tm, btypeid) : _union_for_emplaced_tl(tm, btypeid);
}

pub char const * tm_name(BK_TM *tm, btypeid_t btypeid) {
    // answers the name of the given type or a null pointer it has no name
    if (btypeid <= 0 || btypeid >= tm->next_btypeId) return 0;
    symid_t symid = tm->symid_by_btypeid[btypeid];
    return symid ? sm_name(tm->sm, symid) : 0;
}

pub btypeid_t tm_name_as(BK_TM *tm, btypeid_t btypeid, char const *name) {
    int outcome;  symid_t symid;  u32 idx;

    // assigns name to the unnamed btypedid, checking that name is not already used
    if (btypeid <= 0 || btypeid >= tm->next_btypeId)
        return B_NAT;
    else {
        if ((symid = tm->symid_by_btypeid[btypeid]) != 0)
            // already named - check the given name is the same as the existing name
            return strcmp(sm_name(tm->sm, symid), name) == 0 ? btypeid : B_NAT;
        else {
            // not named so check name is not already in use
            symid = sm_id(tm->sm, name);
            idx = hi_put_idx(TM_BTYPEID_BY_SYMIDHASH, tm->btypeid_by_symidhash, symid, &outcome);
            if (outcome == HI_LIVE)
                return B_NAT;
            else {
                hi_replace_empty(TM_BTYPEID_BY_SYMIDHASH, tm->btypeid_by_symidhash, idx, btypeid);
                tm->symid_by_btypeid[btypeid] = symid;
                return btypeid;
            }
        }
    }
}

pub btypeid_t tm_nominal(BK_TM *tm, btypeid_t self, btypeid_t familyid, char const *name) {
    int outcome;  symid_t symid;  u32 idx;  btsummary sum;

    // answers the validated nominal type corresponding to name and familyid, creating if necessary

    if (!familyid) {
        if (self && self < tm->next_btypeId && TM_BMT_ID(sum = tm->btsummary_by_btypeid[self]) != bmterr) {
            // there is already a type with id self so check we are referring to the same type
            if (TM_BMT_ID(sum) != bmtnom || TM_IN_FAMILY(sum) || strcmp(name, tm_name(tm, self)) != 0) return B_NAT;
            return self;
        } else {
            // if name is not already in use create a new nominal
            symid = sm_id(tm->sm, name);
            idx = hi_put_idx(TM_BTYPEID_BY_SYMIDHASH, tm->btypeid_by_symidhash, symid, &outcome);
            if (outcome == HI_LIVE) {
                // already exists so check it's a nominal not in a family
                self = hi_token(tm->btypeid_by_symidhash, idx);
                sum = tm->btsummary_by_btypeid[self];
                if (TM_BMT_ID(sum) != bmtnom || TM_IN_FAMILY(sum)) return B_NAT;
                return self;
            } else {
                if (self == 0) self = tm->next_btypeId;
                hi_replace_empty(TM_BTYPEID_BY_SYMIDHASH, tm->btypeid_by_symidhash, idx, self);
                tm->symid_by_btypeid[self] = symid;
                _new_type_summary_at(tm, self, bmtnom, B_NAT, 0, 0, false);
                return self;
            }
        }
    } else {
        if (self && self < tm->next_btypeId && TM_BMT_ID(sum = tm->btsummary_by_btypeid[self]) != bmterr) {
            // there is already a type with id self so check we are referring to the same type
            if (TM_BMT_ID(sum) != bmtnom || !TM_IN_FAMILY(sum) || tm->familyid_by_btypeid[self] != familyid || strcmp(name, tm_name(tm, self)) != 0) return B_NAT;
            return self;
        } else {
            // if name is not already in use create a new nominal
            symid = sm_id(tm->sm, name);
            idx = hi_put_idx(TM_BTYPEID_BY_SYMIDHASH, tm->btypeid_by_symidhash, symid, &outcome);
            if (outcome == HI_LIVE) {
                // already exists so check it's a nominal with the same family
                self = hi_token(tm->btypeid_by_symidhash, idx);
                sum = tm->btsummary_by_btypeid[self];
                if (TM_BMT_ID(sum) != bmtnom || !TM_IN_FAMILY(sum) || (tm->familyid_by_btypeid[self] != familyid)) return B_NAT;
                return self;
            } else {
                if (self == 0) self = tm->next_btypeId;
                hi_replace_empty(TM_BTYPEID_BY_SYMIDHASH, tm->btypeid_by_symidhash, idx, self);
                tm->symid_by_btypeid[self] = symid;
                _new_type_summary_at(tm, self, bmtnom, familyid, 0, 0, false);
                return self;
            }
        }
    }
}

pub btypeid_t tm_nominal_impin(BK_TM *tm, btypeid_t self, btypeid_t familyid, char const *name) {
    return B_NAT;
}

pub btypeid_t tm_recursive(BK_TM *tm, btypeid_t self) {
    return B_NAT;
}

pub btypeid_t tm_recursive_in(BK_TM *tm, btypeid_t self, btypeid_t familyid) {
    return B_NAT;
}

pub btypeid_t tm_schemavar(BK_TM *tm, btypeid_t self, char const *name) {
    int outcome;  symid_t symid;  u32 idx;  btsummary sum;

    // answers the validated schema variable corresponding to name, creating if necessary
    if (self && self < tm->next_btypeId && TM_BMT_ID(sum = tm->btsummary_by_btypeid[self]) != bmterr) {
        // there is already a type with id self so check we are referring to the same type
//        if (TM_BMT_ID(sum) != bmtsvr || sum.excl != B_NAT || strcmp(name, tm_name(tm, self)) != 0) return B_NAT;  // OPEN: fix here
        return self;
    } else {
        // if name is not already in use create a new schema variable
        symid = sm_id(tm->sm, name);
        idx = hi_put_idx(TM_BTYPEID_BY_SYMIDHASH, tm->btypeid_by_symidhash, symid, &outcome);
        if (outcome == HI_LIVE) {
            // already exists so check it's a schema variable
            self = hi_token(tm->btypeid_by_symidhash, idx);
            sum = tm->btsummary_by_btypeid[self];
//            if (TM_BMT_ID(sum) != bmtsvr || sum.excl != B_NAT) return B_NAT;  OPEN: fix here
            return self;
        } else {
            if (self == 0) self = tm->next_btypeId;
            hi_replace_empty(TM_BTYPEID_BY_SYMIDHASH, tm->btypeid_by_symidhash, idx, self);
            tm->symid_by_btypeid[self] = symid;
            _new_type_summary_at(tm, self, bmtsvr, B_NAT, 0, 0, true);
            return self;
        }
    }
}

pub btypeid_t tm_seq(BK_TM *tm, btypeid_t self, btypeid_t containedid) {
    i32 outcome;  btsummary *sum;  btypeid_t containerid;  u32 idx;

    // answers the validated sequence type corresponding to tContained, creating if necessary

    // check that containedid is valid
    if (!(0 < containedid && containedid < tm->next_btypeId)) return B_NAT;
    sum = tm->btsummary_by_btypeid + containedid;
    if (TM_BMT_ID(*sum) == bmterr) return B_NAT;

    // get the btypeid for the tContained
    idx = hi_put_idx(TM_BTYPID_BY_SEQIDHASH, tm->containerid_by_containedidhash, containedid, &outcome);
    switch (outcome) {
        default:
            die("%s:%i: HI_TOMBSTONE2!", FN_NAME, __LINE__);
        case HI_LIVE:
            containerid = tm->containerid_by_containedidhash->tokens[idx];
            if (self == 0) return containerid;
            else if (self == containerid) return self;
            else return B_NAT;
        case HI_EMPTY:
            // missing so commit the tuple type for tlid
            if (self == 0)
                self = tm->next_btypeId;
            else if (self < tm->next_btypeId && TM_BMT_ID(tm->btsummary_by_btypeid[self]) != bmterr)
                // self is already in use so given the type list lookup above we cannot be referring to the same btype
                return B_NAT;
            _new_type_summary_at(tm, self, bmtseq, B_NAT, containedid, 0, TM_HAS_T(*sum));
            hi_replace_empty(TM_BTYPID_BY_SEQIDHASH, tm->containerid_by_containedidhash, idx, self);
            return self;
    }
}

pub btypeid_t tm_seq_t(BK_TM *tm, btypeid_t self) {
    btsummary *sum;
    if (!(0 < self && self < tm->next_btypeId)) return B_NAT;
    sum = tm->btsummary_by_btypeid + self;
    return TM_BMT_ID(*sum) == bmtseq ? TM_DETAILS_ID(*sum) : B_NAT;
}

pub size tm_size(BK_TM *tm, btypeid_t self) {
    // OPEN: implement (requires packing decisions which should be put in the client? except the mm needs to be able to navigate)
    return B_NAT;
}

pub btypeid_t tm_size_as(BK_TM *tm, btypeid_t self, size sz) {
    // OPEN: implement
    return B_NAT;
}

pub btypeid_t tm_struct(BK_TM *tm, btypeid_t self, SM_SLID_T slid, btypeid_t tupid) {
    i32 outcome;  TM_DETAILID_T strid;  TM_SLID_TUPID slid_tupid;  u32 idx;

    // answers the validated function type corresponding to tArgs and tRet, creating if necessary

    // check each typeid is valid
    if (!(0 < tupid && tupid < tm->next_btypeId)) return B_NAT;
    if (TM_BMT_ID(*(tm->btsummary_by_btypeid + tupid)) != bmttup) return B_NAT;

    slid_tupid.slid = slid;
    slid_tupid.tupid = tupid;

    // get the btypeid for the t1t2
    idx = hi_put_idx(TM_DETAILID_BY_SLIDTUPIDHASH, tm->strid_by_slidtupidhash, slid_tupid, &outcome);
    switch (outcome) {
        default:
            die("%s:%i: HI_TOMBSTONE!", FN_NAME, __LINE__);
        case HI_LIVE:
            strid = tm->strid_by_slidtupidhash->tokens[idx];
            if (self == 0) return tm->btypid_by_strid[strid];
            else if (self == tm->btypid_by_strid[strid]) return self;
            else return B_NAT;
        case HI_EMPTY:
            // missing so commit the function type for t1t2
            if (self == 0)
                self = tm->next_btypeId;
            else if (self < tm->next_btypeId && TM_BMT_ID(tm->btsummary_by_btypeid[self]) != bmterr)
                // self is already in use so given the t1t2 lookup above we cannot be referring to the same btype
                return B_NAT;
            strid = tm->next_strid++;
            if (strid >= tm->max_strid) {
                tm->max_strid += TM_MAX_ID_INC_SIZE;
                _growTo((void **)&tm->tupid_by_strid, tm->max_strid * sizeof(TM_T1T2), tm->mm, FN_NAME);
                _growTo((void **)&tm->slid_by_strid, tm->max_strid * sizeof(TM_T1T2), tm->mm, FN_NAME);
                _growTo((void **)&tm->btypid_by_strid, tm->max_strid * sizeof(btypeid_t), tm->mm, FN_NAME);
            }
            tm->slid_by_strid[strid] = slid_tupid.slid;
            tm->tupid_by_strid[strid] = slid_tupid.tupid;
            _new_type_summary_at(tm, self, bmtstr, B_NAT, strid, 0, TM_HAS_T(tm->btsummary_by_btypeid[tupid]));
            tm->btypid_by_strid[strid] = self;
            hi_replace_empty(TM_DETAILID_BY_SLIDTUPIDHASH, tm->strid_by_slidtupidhash, idx, strid);
            return self;
    }
}

pub btypeid_t tm_structv_sts(BK_TM *tm, u32 numTypes, ...) {
    // OPEN: implement
    return B_NAT;
}

pub btypeid_t tm_structv_ssts(BK_TM *tm, u32 numTypes, ...) {
    // OPEN: implement
    return B_NAT;
}

pub symid_t * tm_struct_sl(BK_TM *tm, btypeid_t self) {
    // OPEN: implement
    return 0;
}

pub btypeid_t * tm_struct_tl(BK_TM *tm, btypeid_t self) {
    // OPEN: implement
    return 0;
}

pub btypeid_t tm_tuple(BK_TM *tm, btypeid_t self, btypeid_t *typelist) {
    i32 i, outcome, numTypes;  btsummary *sum;  TM_DETAILID_T tupid;  TM_TLID_T tlid;  btypeid_t *p1, *nextTypelist;
    u32 idx;  bool hasT;

    // answers the validated tuple type corresponding to typelist, creating if necessary
    numTypes = typelist[0];

    // check each typeid in the list is valid
    // OPEN: can this loop be merged with the copying loop?
    hasT = false;
    for (i = 1; i <= numTypes; i++) {
        if (!(0 < typelist[i] && typelist[i] < tm->next_btypeId)) return B_NAT;
        sum = tm->btsummary_by_btypeid + typelist[i];
        if (TM_BMT_ID(*sum) == bmterr) return B_NAT;
        hasT = hasT | TM_HAS_T(*sum);
    }

    // make next page of tm->typelist_buf writable if necessary
    if (tm->next_tlrp + numTypes >= tm->max_tlrp) {
        if (tm->next_tlrp + numTypes >= TM_MAX_TL_STORAGE) die("%s: out of typelist storage", FN_NAME);  // OPEN: really we should add an error reporting mechanism, e.g. TM_ERR_OUT_OF_NAME_STORAGE, etc
        size_t pageSize = os_page_size();
        os_mprotect(tm->typelist_buf + tm->max_tlrp, pageSize, BK_PROT_READ | BK_PROT_WRITE);
        os_madvise(tm->typelist_buf + tm->max_tlrp, pageSize, BK_MADV_RANDOM);
    }

    nextTypelist = tm->typelist_buf + tm->next_tlrp;

    // copy typelist into typelist_buf
    p1 = nextTypelist;
    *p1++ = numTypes;
    for (i = 1; i <= numTypes; i++) {
        *p1++ = typelist[i];
    }

    // get the tlid for the typelist - adding if missing, returning 0 if invalid
    idx = hi_put_idx(TM_TLID_BY_TLHASH, tm->tlid_by_tlhash, nextTypelist, &outcome);
    switch (outcome) {
        default:
            die("%s:%i: HI_TOMBSTONE1!", FN_NAME, __LINE__);
        case HI_LIVE:
            tlid = tm->tlid_by_tlhash->tokens[idx];
            break;
        case HI_EMPTY:
            tlid = _commit_typelist_buf_at(tm, numTypes, idx);
            if (!tlid) return B_NAT;       // an error occurred OPEN handle properly
    }

    // get the btypeid for the tlid
    idx = hi_put_idx(TM_DETAILID_BY_TLIDHASH, tm->tupid_by_tlidhash, tlid, &outcome);
    switch (outcome) {
        default:
            die("%s:%i: HI_TOMBSTONE2!", FN_NAME, __LINE__);
        case HI_LIVE:
            tupid = tm->tupid_by_tlidhash->tokens[idx];
            if (self == 0) return tm->btypid_by_tupid[tupid];
            else if (self == tm->btypid_by_tupid[tupid]) return self;
            else return B_NAT;
        case HI_EMPTY:
            // missing so commit the tuple type for tlid
            if (self == 0)
                self = tm->next_btypeId;
            else if (self < tm->next_btypeId && TM_BMT_ID(tm->btsummary_by_btypeid[self]) != bmterr)
                // self is already in use so given the type list lookup above we cannot be referring to the same btype
                return B_NAT;
            tupid = tm->next_tupid++;
            if (tupid >= tm->max_tupid) {
                tm->max_tupid += TM_MAX_ID_INC_SIZE;
                _growTo((void **)&tm->tlid_by_tupid, tm->max_tupid * sizeof(TM_TLID_T), tm->mm, FN_NAME);
                _growTo((void **)&tm->btypid_by_tupid, tm->max_tupid * sizeof(btypeid_t), tm->mm, FN_NAME);
            }
            tm->tlid_by_tupid[tupid] = tlid;
            _new_type_summary_at(tm, self, bmttup, B_NAT, tupid, 0, hasT);
            tm->btypid_by_tupid[tupid] = self;
            hi_replace_empty(TM_DETAILID_BY_TLIDHASH, tm->tupid_by_tlidhash, idx, tupid);
            return self;
    }
}

pub btypeid_t tm_tuplev(BK_TM *tm, u32 numTypes, ...) {
    // OPEN: implement
    return B_NAT;
}

pub btypeid_t * tm_tuple_tl(BK_TM *tm, btypeid_t btypeid) {
    btsummary *sum;
    // OPEN: do we bounds check btypeid here?
    sum = tm->btsummary_by_btypeid + btypeid;
    if (TM_BMT_ID(*sum) == bmttup) {
        return tm->typelist_buf + tm->tlrp_by_tlid[tm->tlid_by_tupid[TM_DETAILS_ID(*sum)]];
    } else {
        return 0;
    }
}

pub btypeid_t tm_union(BK_TM *tm, btypeid_t btypeid, btypeid_t *typelist) {
    i32 i, j, numTypes;  btsummary *sum;  TM_TLID_T tlid;  btypeid_t *uniTl, *p1, *p2, *p3, *nextTypelist;

    if (!(numTypes = typelist[0])) return B_NAT;

    // check typeid is in range, and figure total possible length (including possible duplicate from child unions)
    for (i = 1; i <= numTypes; i++) {
        if (!(0 < typelist[i] && typelist[i] < tm->next_btypeId)) return B_NAT;
        sum = tm->btsummary_by_btypeid + typelist[i];
        if (TM_BMT_ID(*sum) == bmtuni) {
            tlid = tm->tlid_by_uniid[TM_DETAILS_ID(*sum)];
            numTypes += (tm->typelist_buf + tm->tlrp_by_tlid[tlid])[0] - 1;
        }
    }

    // make next page of tm->typelist_buf writable if necessary
    if (tm->next_tlrp + numTypes >= tm->max_tlrp) {
        if (tm->next_tlrp + numTypes >= TM_MAX_TL_STORAGE) die("%s: out of typelist storage", FN_NAME);  // OPEN: really we should add an error reporting mechanism, e.g. TM_ERR_OUT_OF_NAME_STORAGE, etc
        size_t pageSize = os_page_size();
        os_mprotect(tm->typelist_buf + tm->max_tlrp, pageSize, BK_PROT_READ | BK_PROT_WRITE);
        os_madvise(tm->typelist_buf + tm->max_tlrp, pageSize, BK_MADV_RANDOM);
    }

    nextTypelist = tm->typelist_buf + tm->next_tlrp;

    // copy typelist into typelist_buf unpacking any unions
    p1 = nextTypelist;
    *p1++ = numTypes;
    for (i = 1; i <= numTypes; i++) {
        sum = tm->btsummary_by_btypeid + typelist[i];
        if (TM_BMT_ID(*sum) == bmtuni) {
            // we have a union type - expand it
            tlid = tm->tlid_by_uniid[TM_DETAILS_ID(*sum)];
            uniTl = (tm->typelist_buf + tm->tlrp_by_tlid[tlid]);
            for (j = 1; j <= (i32)uniTl[0]; j++) *p1++ = uniTl[j];
        }
        else
            *p1++ = typelist[i];
    }

    // sort types into btypeid order
    ks_radix_sort(btypeid_t, nextTypelist + 1, numTypes);

    // eliminate duplicates
    p1 = nextTypelist + 1;
    p2 = p1 + 1;
    p3 = p1 + numTypes;
    while (p2 < p3) {
        if (*p1 != *p2)
            *++p1 = *p2++;
        else
            while (*p1 == *p2 && p2 < p3) p2++;
    }
    *nextTypelist = p1 - nextTypelist;

    return _union_for_emplaced_tl(tm, btypeid);
}
pvt btypeid_t _union_for_emplaced_tl(BK_TM *tm, btypeid_t btypeid) {
    u32 idx;  i32 outcome, i;  TM_TLID_T tlid;  TM_DETAILID_T uniid;  btypeid_t *nextTypelist; i32 numTypes;  bool hasT;

    nextTypelist = tm->typelist_buf + tm->next_tlrp;
    numTypes = *nextTypelist;

    // get the tlid for the typelist - adding if missing, returning 0 if invalid
    idx = hi_put_idx(TM_TLID_BY_TLHASH, tm->tlid_by_tlhash, nextTypelist, &outcome);
    switch (outcome) {
        default:
            die("%s: HI_TOMBSTONE1!", FN_NAME);
        case HI_LIVE:
            tlid = tm->tlid_by_tlhash->tokens[idx];
            break;
        case HI_EMPTY:
            tlid = _commit_typelist_buf_at(tm, numTypes, idx);
            if (!tlid) return 0;       // an error occurred OPEN handle properly
    }

    // get the btypeid for the tlid
    idx = hi_put_idx(TM_DETAILID_BY_TLIDHASH, tm->uniid_by_tlidhash, tlid, &outcome);
    switch (outcome) {
        default:
            die("%s: HI_TOMBSTONE2!", FN_NAME);
        case HI_LIVE:
            uniid = tm->uniid_by_tlidhash->tokens[idx];
            if (btypeid == 0) return tm->btypid_by_uniid[uniid];
            else if (btypeid == tm->btypid_by_uniid[uniid]) return btypeid;
            else return 0;
        case HI_EMPTY:
            // missing so commit the union type for tlid
            if (btypeid == 0)
                btypeid = tm->next_btypeId;
            else if (btypeid < tm->next_btypeId && TM_BMT_ID(tm->btsummary_by_btypeid[btypeid]) != bmterr)
                // btypeid is already in use so given the type list lookup above we cannot be referring to the same btype
                return B_NAT;
            uniid = tm->next_uniid++;
            if (uniid >= tm->max_uniid) {
                tm->max_uniid += TM_MAX_ID_INC_SIZE;
                _growTo((void **)&tm->tlid_by_uniid, tm->max_uniid * sizeof(TM_TLID_T), tm->mm, FN_NAME);
                _growTo((void **)&tm->btypid_by_uniid, tm->max_uniid * sizeof(btypeid_t), tm->mm, FN_NAME);
            }
            tm->tlid_by_uniid[uniid] = tlid;
            hasT = false;
            for (i = 1; i <= numTypes; i++) hasT = hasT || TM_HAS_T(tm->btsummary_by_btypeid[nextTypelist[i]]);
            _new_type_summary_at(tm, btypeid, bmtuni, B_NAT, uniid, 0, hasT);
            tm->btypid_by_uniid[uniid] = btypeid;
            hi_replace_empty(TM_DETAILID_BY_TLIDHASH, tm->uniid_by_tlidhash, idx, uniid);
            return btypeid;
    }
}

pub btypeid_t * tm_union_tl(BK_TM *tm, btypeid_t btypeid) {
    // answer pointer to the typelist of the union btypeid or 0 for error
    btsummary *sum;
    sum = tm->btsummary_by_btypeid + btypeid;
    if (TM_BMT_ID(*sum) == bmtuni) {
        return tm->typelist_buf + tm->tlrp_by_tlid[tm->tlid_by_uniid[TM_DETAILS_ID(*sum)]];
    } else {
        return 0;
    }
}

pub btypeid_t tm_union_v(BK_TM *tm, btypeid_t self, u32 numTypes, ...) {
    va_list args;  btypeid_t *typelist;  int i;  btypeid_t btypeid;
    va_start(args, numTypes);
    typelist = malloc((1 + numTypes) * sizeof(btypeid_t));
    for (i = 1; i <= numTypes; i++) typelist[i] = va_arg(args, btypeid_t);
    typelist[0] = numTypes;
    btypeid = tm_union(tm, self, typelist);
    free(typelist);
    va_end(args);
    return btypeid;
}

pub btypeid_t _tm_union_v2(BK_TM *tm, btypeid_t self, u32 numTypes, btypeid_t *args) {
    btypeid_t *typelist;  int i;  btypeid_t btypeid;
    typelist = malloc((1 + numTypes) * sizeof(btypeid_t));      // OPEN: use a typelist buffer of big enough size
    for (i = 1; i <= numTypes; i++) typelist[i] = args[i-1];
    typelist[0] = numTypes;
    btypeid = tm_union(tm, self, typelist);
    free(typelist);
    return btypeid;
}


// ---------------------------------------------------------------------------------------------------------------------
// sub-type checking
// ---------------------------------------------------------------------------------------------------------------------

int tm_fitsWithin(BK_TM *tm, btypeid_t a, btypeid_t b) {
    // should answer a tuple {cacheID, doesFit, tByT, distance}
    // tByT can just be a T sorted list (not worth doing a hash)
    if (a == b) return 1;
    if ((b == B_EXTERN) && (a & B_EXTERN)) return 1;
    if ((b == B_FN) && ((a & 0x7f) == B_FN)) return 1;
    if ((b == B_EXTERN_FN_PTR) && ((a & 0xffff) == B_EXTERN_FN_PTR)) return 1;
    if ((b == B_FN_PTR) && ((a & 0xff7f) == B_FN_PTR)) return 1;
    if ((b == B_CHAR_STAR) && ((a & 0xff7f) == B_CHAR_STAR)) return 1;
    if ((b == B_VOID_STAR) && ((a & 0xff7f) == B_VOID_STAR)) return 1;
    if ((a & 0x000000FF) == b) return 1;
    return 0;
}


// ---------------------------------------------------------------------------------------------------------------------
// type manager lifecycle fns
// ---------------------------------------------------------------------------------------------------------------------

pub BK_TM * TM_create(BK_MM *mm, Buckets *buckets, BK_SM *sm, BK_TP *tp) {
    // OPEN: should we use calloc instead of memset to init arrays to zero?
    BK_TM *tm = (BK_TM *) mm->malloc(sizeof(BK_TM));
    tm->mm = mm;
    tm->buckets = buckets;
    tm->sm = sm;
    tm->tp = tp;

    // typelists
    tm->typelist_buf = os_vm_reserve(0, TM_MAX_TL_STORAGE);
    tm->max_tlrp = os_page_size() / sizeof(TM_TLID_T);
    os_mprotect(tm->typelist_buf, tm->max_tlrp * sizeof(TM_TLID_T), BK_PROT_READ | BK_PROT_WRITE);  // make first page of typelist storage R/W
    os_madvise(tm->typelist_buf, tm->max_tlrp * sizeof(TM_TLID_T), BK_MADV_RANDOM);                 // and advise as randomly accessed
    tm->next_tlrp = 0;
    tm->max_tlid = TM_MAX_SLID_INC_SIZE;
    tm->next_tlid = 1;
    tm->tlrp_by_tlid = (RP *) mm->malloc(tm->max_tlid * sizeof(RP));
    memset(tm->tlrp_by_tlid, 0, tm->max_tlid * sizeof(RP));
    tm->tlid_by_tlhash = hi_create(TM_TLID_BY_TLHASH);
    tm->tlid_by_tlhash->tm = tm;

    // type names
    tm->btypeid_by_symidhash = hi_create(TM_BTYPEID_BY_SYMIDHASH);
    tm->btypeid_by_symidhash->tm = tm;
    tm->max_btypeId = TM_MAX_BTYPEID_INC_SIZE;
    tm->next_btypeId = 1;
    tm->symid_by_btypeid = (TM_DETAILID_T *) mm->malloc(tm->max_btypeId * sizeof(TM_DETAILID_T));
    memset(tm->symid_by_btypeid, 0, tm->max_btypeId * sizeof(btypeid_t));

    // type summaries
    tm->btsummary_by_btypeid = (btsummary *) mm->malloc(tm->max_btypeId * sizeof(btsummary));
    memset(tm->btsummary_by_btypeid, 0, tm->max_btypeId * sizeof(btsummary));

    // families
    tm->familyid_by_btypeid = (btypeid_t *) mm->malloc(tm->max_btypeId * sizeof(btypeid_t));
    memset(tm->familyid_by_btypeid, 0, tm->max_btypeId * sizeof(btypeid_t));

    // intersections
    tm->max_intid = TM_MAX_ID_INC_SIZE;
    tm->next_intid = 1;
    tm->tlid_by_intid = (TM_TLID_T *) mm->malloc(tm->max_intid * sizeof(TM_TLID_T));
    memset(tm->tlid_by_intid, 0, tm->max_intid * sizeof(TM_TLID_T));
    tm->btypid_by_intid = (TM_DETAILID_T *) mm->malloc(tm->max_intid * sizeof(TM_DETAILID_T));
    memset(tm->btypid_by_intid, 0, tm->max_intid * sizeof(TM_DETAILID_T));
    tm->intid_by_tlidhash = hi_create(TM_DETAILID_BY_TLIDHASH);
    tm->intid_by_tlidhash->tlid_by_detailid = tm->tlid_by_intid;

    // unions
    tm->max_uniid = TM_MAX_ID_INC_SIZE;
    tm->next_uniid = 1;
    tm->tlid_by_uniid = (TM_TLID_T *) mm->malloc(tm->max_uniid * sizeof(TM_TLID_T));
    memset(tm->tlid_by_uniid, 0, tm->max_uniid * sizeof(TM_TLID_T));
    tm->btypid_by_uniid = (TM_DETAILID_T *) mm->malloc(tm->max_uniid * sizeof(TM_DETAILID_T));
    memset(tm->btypid_by_uniid, 0, tm->max_uniid * sizeof(TM_DETAILID_T));
    tm->uniid_by_tlidhash = hi_create(TM_DETAILID_BY_TLIDHASH);
    tm->uniid_by_tlidhash->tlid_by_detailid = tm->tlid_by_uniid;

    // tuples
    tm->max_tupid = TM_MAX_ID_INC_SIZE;
    tm->next_tupid = 1;
    tm->tlid_by_tupid = (TM_TLID_T *) mm->malloc(tm->max_tupid * sizeof(TM_TLID_T));
    memset(tm->tlid_by_tupid, 0, tm->max_tupid * sizeof(TM_TLID_T));
    tm->btypid_by_tupid = (btypeid_t *) mm->malloc(tm->max_tupid * sizeof(btypeid_t));
    memset(tm->btypid_by_tupid, 0, tm->max_tupid * sizeof(btypeid_t));
    tm->tupid_by_tlidhash = hi_create(TM_DETAILID_BY_TLIDHASH);
    tm->tupid_by_tlidhash->tlid_by_detailid = tm->tlid_by_tupid;

    // structs
    tm->max_strid = TM_MAX_ID_INC_SIZE;
    tm->next_strid = 1;
    tm->tupid_by_strid = (TM_TLID_T *) mm->malloc(tm->max_strid * sizeof(TM_TLID_T));
    memset(tm->tupid_by_strid, 0, tm->max_strid * sizeof(TM_TLID_T));
    tm->slid_by_strid = (SM_SLID_T *) mm->malloc(tm->max_strid * sizeof(SM_SLID_T));
    memset(tm->slid_by_strid, 0, tm->max_strid * sizeof(SM_SLID_T));
    tm->btypid_by_strid = (btypeid_t *) mm->malloc(tm->max_strid * sizeof(btypeid_t));
    memset(tm->btypid_by_strid, 0, tm->max_strid * sizeof(btypeid_t));
    tm->strid_by_slidtupidhash = hi_create(TM_DETAILID_BY_SLIDTUPIDHASH);
    tm->strid_by_slidtupidhash->tm = tm;

    // records
    tm->max_recid = TM_MAX_ID_INC_SIZE;
    tm->next_recid = 1;
    tm->tupid_by_recid = (TM_TLID_T *) mm->malloc(tm->max_recid * sizeof(TM_TLID_T));
    memset(tm->tupid_by_recid, 0, tm->max_recid * sizeof(TM_TLID_T));
    tm->btypid_by_recid = (btypeid_t *) mm->malloc(tm->max_recid * sizeof(btypeid_t));
    memset(tm->btypid_by_recid, 0, tm->max_recid * sizeof(btypeid_t));
    tm->recid_by_slidtlidhash = hi_create(TM_DETAILID_BY_SLIDTUPIDHASH);
    tm->recid_by_slidtlidhash->tm = tm;

    // sequences
    tm->containerid_by_containedidhash = hi_create(TM_BTYPID_BY_SEQIDHASH);
    tm->containerid_by_containedidhash->tm = tm;

    // maps
    tm->max_mapid = TM_MAX_ID_INC_SIZE;
    tm->next_mapid = 1;
    tm->t1t2_by_mapid = (TM_T1T2 *) mm->malloc(tm->max_mapid * sizeof(TM_T1T2));
    memset(tm->t1t2_by_mapid, 0, tm->max_mapid * sizeof(TM_T1T2));
    tm->btypid_by_mapid = (btypeid_t *) mm->malloc(tm->max_mapid * sizeof(btypeid_t));
    memset(tm->btypid_by_mapid, 0, tm->max_mapid * sizeof(btypeid_t));
    tm->mapid_by_t1t2hash = hi_create(TM_DETAILID_BY_T1T2HASH);
    tm->mapid_by_t1t2hash->t1t2_by_detailid = tm->t1t2_by_mapid;

    // functions
    tm->max_fncid = TM_MAX_ID_INC_SIZE;
    tm->next_fncid = 1;
    tm->t1t2_by_fncid = (TM_T1T2 *) mm->malloc(tm->max_fncid * sizeof(TM_T1T2));
    memset(tm->t1t2_by_fncid, 0, tm->max_fncid * sizeof(TM_T1T2));
    tm->btypid_by_fncid = (btypeid_t *) mm->malloc(tm->max_fncid * sizeof(btypeid_t));
    memset(tm->btypid_by_fncid, 0, tm->max_fncid * sizeof(btypeid_t));
    tm->fncid_by_t1t2hash = hi_create(TM_DETAILID_BY_T1T2HASH);
    tm->fncid_by_t1t2hash->t1t2_by_detailid = tm->t1t2_by_fncid;

    // schema variables

    return tm;
}

pub int TM_trash(BK_TM *tm) {
    // typelists
    tm->mm->free(tm->tlrp_by_tlid);
    hi_trash(TM_TLID_BY_TLHASH, tm->tlid_by_tlhash);
    os_vm_unreserve(tm->typelist_buf, TM_MAX_TL_STORAGE);

    // type names
    hi_trash(TM_BTYPEID_BY_SYMIDHASH, tm->btypeid_by_symidhash);
    tm->mm->free(tm->symid_by_btypeid);

    // type summaries
    tm->mm->free(tm->btsummary_by_btypeid);

    // families
    tm->mm->free(tm->familyid_by_btypeid);

    // intersections
    tm->mm->free(tm->tlid_by_intid);
    tm->mm->free(tm->btypid_by_intid);
    hi_trash(TM_DETAILID_BY_TLIDHASH, tm->intid_by_tlidhash);

    // unions
    tm->mm->free(tm->tlid_by_uniid);
    tm->mm->free(tm->btypid_by_uniid);
    hi_trash(TM_DETAILID_BY_TLIDHASH, tm->uniid_by_tlidhash);

    // tuples
    tm->mm->free(tm->tlid_by_tupid);
    tm->mm->free(tm->btypid_by_tupid);
    hi_trash(TM_DETAILID_BY_TLIDHASH, tm->tupid_by_tlidhash);

    // structs
    tm->mm->free(tm->tupid_by_strid);
    tm->mm->free(tm->slid_by_strid);
    tm->mm->free(tm->btypid_by_strid);
    hi_trash(TM_DETAILID_BY_SLIDTUPIDHASH, tm->strid_by_slidtupidhash);

    // records
    tm->mm->free(tm->tupid_by_recid);
    tm->mm->free(tm->slid_by_recid);
    tm->mm->free(tm->btypid_by_recid);
    hi_trash(TM_DETAILID_BY_SLIDTUPIDHASH, tm->recid_by_slidtlidhash);

    // sequences
    hi_trash(TM_BTYPID_BY_SEQIDHASH, tm->containerid_by_containedidhash);

    // maps
    tm->mm->free(tm->t1t2_by_mapid);
    tm->mm->free(tm->btypid_by_mapid);
    hi_trash(TM_DETAILID_BY_T1T2HASH, tm->mapid_by_t1t2hash);

    // functions
    tm->mm->free(tm->t1t2_by_fncid);
    tm->mm->free(tm->btypid_by_fncid);
    hi_trash(TM_DETAILID_BY_T1T2HASH, tm->fncid_by_t1t2hash);

    // schema variables

    // self
    tm->mm->free(tm);
    return 0;
}



#define BK_INTERSECTION(tm, ...) ({                                                                                     \
    btypeid_t args[] = { __VA_ARGS__ };                                                                                 \
    tm_inter_v((tm), 0, sizeof(args) / sizeof(args[0]), args);                                                          \
})

#define BK_UNION(tm, ...) ({                                                                                            \
    btypeid_t args[] = { __VA_ARGS__ };                                                                                 \
    _tm_union_v2((tm), 0, sizeof(args) / sizeof(args[0]), args);                                                        \
})



#endif  // __BK_TM_C
