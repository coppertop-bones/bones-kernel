// ---------------------------------------------------------------------------------------------------------------------
// TM - TYPE MANAGER
// ---------------------------------------------------------------------------------------------------------------------

#ifndef __BK_TM_C
#define __BK_TM_C "bk/tm.c"


#include "../../include/bk/mm.h"
#include "../../include/bk/tm.h"
#include "../../include/bk/tp.h"
#include "lib/hi_impl.h"
#include "lib/radix.h"


KRADIX_SORT_INIT(btypeid_t, btypeid_t, ,sizeof(btypeid_t))


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
    u8 *s = (unsigned char *) tl;
    u8 *e = s + n;
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
// TM_XXXID_BY_TLIDHASH fns
// ---------------------------------------------------------------------------------------------------------------------

pvt inline TM_TLID_T tlidFromXxxid(hi_struct(TM_XXXID_BY_TLIDHASH) *hi, TM_XXXID_T xxxid) {
    return hi->tlid_by_xxxid[xxxid];
}

pvt bool inline tlidHashableFound(hi_struct(TM_XXXID_BY_TLIDHASH) *hi, TM_XXXID_T token, TM_TLID_T hashable) {
    return hi->tlid_by_xxxid[token] == hashable;
}

// HI_IMPL(name, token_t, hashable_t, __hash_fn, __found_fn, __hashable_from_token_fn)
HI_IMPL(TM_XXXID_BY_TLIDHASH, TM_XXXID_T, TM_TLID_T, hi_int32_hash, tlidHashableFound, tlidFromXxxid)


// ---------------------------------------------------------------------------------------------------------------------
// TM_BTYPID_BY_SEQIDHASH fns - find the container btypeid from the contained btypeid (seqid)
// ---------------------------------------------------------------------------------------------------------------------

pvt inline btypeid_t seqidFromBtypeid(hi_struct(TM_BTYPID_BY_SEQIDHASH) *hi, btypeid_t containerbtypeid) {
    return hi->tm->summary_by_btypeid[containerbtypeid].seqId;
}

pvt bool inline seqidHashableFound(hi_struct(TM_BTYPID_BY_SEQIDHASH) *hi, btypeid_t containerbtypeid, btypeid_t hashable) {
    // having hi->tm keeps summary hotter - good idea? slightly less memory and no need to maintain btypeid_by_seqid array
    struct btsummary sum = hi->tm->summary_by_btypeid[containerbtypeid];
    return sum.bmtid == bmtseq && sum.seqId == hashable;
}

// HI_IMPL(name, token_t, hashable_t, __hash_fn, __found_fn, __hashable_from_token_fn)
HI_IMPL(TM_BTYPID_BY_SEQIDHASH, TM_XXXID_T, btypeid_t, hi_int32_hash, seqidHashableFound, seqidFromBtypeid)


// ---------------------------------------------------------------------------------------------------------------------
// TM_XXXID_BY_T1T2HASH fns
// ---------------------------------------------------------------------------------------------------------------------

pvt inline TM_T1T2 t1t2idFromXxxid(hi_struct(TM_XXXID_BY_T1T2HASH) *hi, TM_XXXID_T xxxid) {
    return hi->t1t2_by_xxxid[xxxid];
}

pvt bool inline t1t2HashableFound(hi_struct(TM_XXXID_BY_T1T2HASH) *hi, TM_XXXID_T token, TM_T1T2 hashable) {
    TM_T1T2 t1t2 = hi->t1t2_by_xxxid[token];
    return t1t2.t1 == hashable.t1 && t1t2.t2 == hashable.t2;
}

pvt u32 t1t2_hash(TM_T1T2 t1t2) {
    u8 *s = (unsigned char *) &t1t2;
    u8 *e = s + 2 * sizeof(btypeid_t);
    u32 hash = *s++;
    for (; s < e; s++) if (*s) hash = (hash << 5) - hash + *s;  // OPEN: explain why ignoring zeros
    return hash;
}

// HI_IMPL(name, token_t, hashable_t, __hash_fn, __found_fn, __hashable_from_token_fn)
HI_IMPL(TM_XXXID_BY_T1T2HASH, TM_XXXID_T, TM_T1T2, t1t2_hash, t1t2HashableFound, t1t2idFromXxxid)


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
        tm->intid_by_tlidhash->tlid_by_xxxid = tm->tlid_by_intid;
    }
    tm->tlrp_by_tlid[tlid] = tm->next_tlrp;
    hi_replace_empty(TM_TLID_BY_TLHASH, tm->tlid_by_tlhash, idx, tlid);
    if (tm->next_tlrp + numTypes + 1 >= tm->max_tlrp) {
        size_t pageSize = os_page_size();
        os_mprotect(tm->typelist_buf + tm->max_tlrp - pageSize, pageSize, BK_M_READ);     // make the prior last page read only
        tm->max_tlrp += pageSize / sizeof(TM_TLID_T);
    }
    tm->next_tlrp += numTypes + 1;
    return tlid;
}

tdd void _new_type_summary_at(BK_TM *tm, bmetatypeid_t bmtid, btexclusioncat_t excl, btypeid_t btypeid, u32 _id) {
    // OPEN: add size
    // OPEN: do we restrict the range of directly assigned btypeids?
    while (btypeid >= tm->max_btypeId) {
        tm->max_btypeId += TM_MAX_BTYPEID_INC_SIZE;
        _growTo((void **)&tm->summary_by_btypeid, tm->max_btypeId * sizeof(struct btsummary), tm->mm, FN_NAME);
        _growTo((void **)&tm->symid_by_btypeid, tm->max_btypeId * sizeof(symid_t), tm->mm, FN_NAME);
    }
    tm->summary_by_btypeid[btypeid].bmtid = bmtid;
    tm->summary_by_btypeid[btypeid].excl = excl;
    tm->summary_by_btypeid[btypeid]._id = _id;
    if (btypeid >= tm->next_btypeId) tm->next_btypeId = btypeid + 1;
}


// ---------------------------------------------------------------------------------------------------------------------
// pretty printing
// ---------------------------------------------------------------------------------------------------------------------

pvt void tm_pb(BK_TM *tm, BK_TP *tp, btypeid_t btypeid) {
    struct btsummary *sum;  symid_t symid;  btypeid_t *tl;  i32 i;  char sep;
    if ((symid = tm->symid_by_btypeid[btypeid])) {
        tp_buf_printf(tp, "%s", sm_name(tm->sm, symid));
    } else {
        sum = tm->summary_by_btypeid + btypeid;
        switch (sum->bmtid) {
            case bmtnom:
                tp_buf_printf(tp, "%s", sm_name(tm->sm, symid));
                break;
            case bmtint:
                tl = tm->typelist_buf + tm->tlrp_by_tlid[tm->tlid_by_intid[sum->intId]];
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
pvt inline TPN tm_pp(BK_TM *tm, BK_TP *tp, btypeid_t btypeid) {tm_pb(tm, tp, btypeid); return tp_buf_flush(tp);}
pvt inline S8 tm_s8(BK_TM *tm, BK_TP *tp, btypeid_t btypeid) {tm_pb(tm, tp, btypeid); return tp_s8(tp, tp_buf_flush(tp));}


pvt void tm_pb_typelist(BK_TM *tm, BK_TP *tp, btypeid_t *typelist) {
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

pvt inline TPN tm_pp_typelist(BK_TM *tm, BK_TP *tp, btypeid_t *typelist) {tm_pb_typelist(tm, tp, typelist); return tp_buf_flush(tp);}
pvt inline S8 tm_s8_typelist(BK_TM *tm, BK_TP *tp, btypeid_t *typelist) {tm_pb_typelist(tm, tp, typelist); return tp_s8(tp, tp_buf_flush(tp));}



// ---------------------------------------------------------------------------------------------------------------------
// type accessing / creation fns
// ---------------------------------------------------------------------------------------------------------------------

pub bmetatypeid_t tm_bmetatypeid(BK_TM *tm, btypeid_t btypeid) {
    if (btypeid < 1 || btypeid >= tm->next_btypeId) return 0;
    return tm->summary_by_btypeid[btypeid].bmtid;
}

pub btypeid_t tm_btypeid(BK_TM *tm, char *name) {
    int outcome;  u32 idx;
    idx = hi_put_idx(TM_BTYPEID_BY_SYMIDHASH, tm->btypeid_by_symidhash, sm_id(tm->sm, name), &outcome);
    if (outcome == HI_LIVE)
        return tm->btypeid_by_symidhash->tokens[idx];
    else
        return 0;
}

pub btypeid_t tm_exclnominal(BK_TM *tm, char *name, btexclusioncat_t excl, btypesize_t sz, btypeid_t btypeid) {
    // answers the btypeid. if btypedid already exists check name and that it is a nominal. if not create it.
    int outcome;  symid_t symid;  u32 idx;  struct btsummary sum;
    if (btypeid && btypeid < tm->next_btypeId && (sum = tm->summary_by_btypeid[btypeid]).bmtid != bmterr) {
        // already exists so check we are referring to the same type
        if (sum.bmtid != bmtnom || sum.excl != excl || strcmp(name, tm_name(tm, btypeid)) != 0) return B_NAT;          // OPEN: check size too
        return btypeid;
    } else {
        // if name is not already in use create a new nominal
        symid = sm_id(tm->sm, name);
        idx = hi_put_idx(TM_BTYPEID_BY_SYMIDHASH, tm->btypeid_by_symidhash, symid, &outcome);
        if (outcome == HI_LIVE) {
            // already exists so check it's a nominal with the same exclusion
            btypeid = hi_token(tm->btypeid_by_symidhash, idx);
            struct btsummary s = tm->summary_by_btypeid[btypeid];
            if (s.bmtid != bmtnom || s.excl != excl) return B_NAT;          // OPEN: check size
            return btypeid;
        } else {
            if (btypeid == 0) btypeid = tm->next_btypeId;
            hi_replace_empty(TM_BTYPEID_BY_SYMIDHASH, tm->btypeid_by_symidhash, idx, btypeid);
            tm->symid_by_btypeid[btypeid] = symid;
            _new_type_summary_at(tm, bmtnom, excl, btypeid, 0);
            return btypeid;
        }
    }
}

pub btypeid_t tm_fn(BK_TM *tm, btypeid_t tArgs, btypeid_t tRet, btypeid_t btypeid) {
    i32 outcome;  TM_XXXID_T fncid;  TM_T1T2 t1t2;  u32 idx;

    // answers the validated function type corresponding to tArgs and tRet, creating if necessary

    // check each typeid is valid
    if (!(0 < tArgs && tArgs < tm->next_btypeId)) return 0;
    if ((tm->summary_by_btypeid + tArgs)->bmtid != bmttup) return 0;
    if (!(0 < tRet && tRet < tm->next_btypeId)) return 0;
    if ((tm->summary_by_btypeid + tRet)->bmtid == bmterr) return 0;

    t1t2.tArgs = tArgs;
    t1t2.tRet = tRet;

    // get the btypeid for the t1t2
    idx = hi_put_idx(TM_XXXID_BY_T1T2HASH, tm->fncid_by_t1t2hash, t1t2, &outcome);
    switch (outcome) {
        default:
            die("%s:%i: HI_TOMBSTONE2!", FN_NAME, __LINE__);
        case HI_LIVE:
            fncid = tm->fncid_by_t1t2hash->tokens[idx];
            if (btypeid == 0) return tm->btypid_by_fncid[fncid];
            else if (btypeid == tm->btypid_by_fncid[fncid]) return btypeid;
            else return 0;
        case HI_EMPTY:
            // missing so commit the function type for t1t2
            if (btypeid == 0)
                btypeid = tm->next_btypeId;
            else if (btypeid < tm->next_btypeId && tm->summary_by_btypeid[btypeid].bmtid != bmterr)
                // btypeid is already in use so given the t1t2 lookup above we cannot be referring to the same btype
                return B_NAT;
            fncid = tm->next_fncid++;
            if (fncid >= tm->max_fncid) {
                tm->max_fncid += TM_MAX_ID_INC_SIZE;
                _growTo((void **)&tm->t1t2_by_fncid, tm->max_fncid * sizeof(TM_T1T2), tm->mm, FN_NAME);
                _growTo((void **)&tm->btypid_by_fncid, tm->max_fncid * sizeof(btypeid_t), tm->mm, FN_NAME);
            }
            tm->t1t2_by_fncid[fncid] = t1t2;
            _new_type_summary_at(tm, bmtfnc, btenone, btypeid, fncid);
            tm->btypid_by_fncid[fncid] = btypeid;
            hi_replace_empty(TM_XXXID_BY_T1T2HASH, tm->fncid_by_t1t2hash, idx, fncid);
            return btypeid;
    }
}

pub TM_T1T2 tm_Fn(BK_TM *tm, btypeid_t btypeid) {
    if (btypeid < 1 || btypeid >= tm->next_btypeId) return (TM_T1T2) {{0}, {0}};
    struct btsummary *sum = tm->summary_by_btypeid + btypeid;       // OPEN: in general use pointer to summary rather than copying the struct
    if (sum->bmtid != bmtfnc) return (TM_T1T2) {{0}, {0}};
    return tm->t1t2_by_fncid[sum->fncId];
}

pub btexclusioncat_t tm_exclusion_cat(BK_TM *tm, char *name, btexclusioncat_t excl) {
    // answers the exclusion category for the name creating if necessary. if excl is given checks for consistency
    if (excl == 0) {
        if (strcmp(name, "mem") == 0) return btememory;
        if (strcmp(name, "ptr") == 0) return bteptr;
        if (strcmp(name, "ccy") == 0) return bteccy;
    }
    else {
        if (strcmp(name, "mem") == 0 && excl == btememory) return btememory;
        if (strcmp(name, "ptr") == 0 && excl == bteptr) return bteptr;
        if (strcmp(name, "ccy") == 0 && excl == bteccy) return bteccy;
    }
    return btenone;
}

// set of values intersection ((1 2 3) + (4 5)) & ((1 2 3) + (6 7)) = (1 2 3 4 5) & (1 2 3 6 7) = (1 2 3)
// (int + str) & (int + bool) => (int+int) & (int+bool) & (str+int) & (str+bool)
// types only make sense in the context of fitsWithin a LHS might not behaviour as a RHS


pub btypeid_t tm_inter(BK_TM *tm, btypeid_t *typelist, btypeid_t btypeid) {
    i32 i, j, outcome, numTypes, hasUnions;  btexclusioncat_t excl = 0;  TM_TLID_T tlid;
    btypeid_t *interTl, *p1, *p2, *p3, *nextTypelist;
    TM_XXXID_T intid;  struct btsummary *sum;  u32 idx;
    // (A&B) & (C&D)  = A & B & C & D
    // (A&B) & (B&C)  = A & B & C
    // (A+B) & (B+C)  = (A+B) & (B+C)  why not B? because we need to keep the detail when the program causes intersections
    //
    // (A&B) + (B&C)  = B & (A + C)    - not the same for unions of intersections

    // use tm->typelist_buf as scratch so don't have to allocate memory
    // OPEN: potentially though messy we could do intersections without child intersections in place in typelist to keep a little cache locality

    if (!(numTypes = typelist[0])) return 0;

    // check btypeids in typelist are in range, and figure total possible length (including possible duplicate from child intersections)
    for (i = 1; i <= (i32)typelist[0]; i++) {
        if (!(0 < typelist[i] && typelist[i] < tm->next_btypeId)) return 0;
        sum = tm->summary_by_btypeid + typelist[i];
        if (sum->bmtid == bmtint) {
            tlid = tm->tlid_by_intid[sum->intId];
            numTypes += (tm->typelist_buf + tm->tlrp_by_tlid[tlid])[0] - 1;
        }
    }

    // make next page of tm->typelist_buf writable if necessary
    if (tm->next_tlrp + numTypes >= tm->max_tlrp) {
        if (tm->next_tlrp + numTypes >= TM_MAX_TL_STORAGE) die("%s: out of typelist storage", FN_NAME);  // OPEN: really we should add an error reporting mechanism, e.g. TM_ERR_OUT_OF_NAME_STORAGE, etc
        size_t pageSize = os_page_size();
        os_mprotect(tm->typelist_buf + tm->max_tlrp, pageSize, BK_M_READ | BK_M_WRITE);
        os_madvise(tm->typelist_buf + tm->max_tlrp, pageSize, BK_AD_RANDOM);
    }

    nextTypelist = tm->typelist_buf + tm->next_tlrp;

    // copy typelist into typelist_buf unpacking any intersections
    p1 = nextTypelist;
    *p1++ = numTypes;
    for (i = 1; i <= (i32)typelist[0]; i++) {
        sum = tm->summary_by_btypeid + typelist[i];
        if (sum->bmtid == bmtint) {
            // we have an intersection type - expand it
            tlid = tm->tlid_by_intid[sum->intId];
            interTl = (tm->typelist_buf + tm->tlrp_by_tlid[tlid]);
            for (j = 1; j <= (i32)interTl[0]; j++) *p1++ = interTl[j];
        }
        else
            *p1++ = typelist[i];
    }

    // sort types into btypeid order
    ks_radix_sort(btypeid_t, nextTypelist + 1, numTypes);

    // eliminate duplicates + check for unions
    p1 = nextTypelist + 1;
    p2 = p1 + 1;
    p3 = p1 + numTypes;
    hasUnions = (tm->summary_by_btypeid[*p1]).bmtid == bmtuni;
    while (p2 < p3) {
        if (*p1 != *p2)
            *++p1 = *p2++;
        else
            while (*p1 == *p2 && p2 < p3) p2++;
        hasUnions |= (tm->summary_by_btypeid[*p1]).bmtid == bmtuni;
    }
    numTypes = *nextTypelist = p1 - nextTypelist;

    // handle intersections of unions?
    if (hasUnions) return 0;

    // check for exclusion conflicts
    p1 = nextTypelist;
    for (i = 1; i <= numTypes; i++) {
        sum = tm->summary_by_btypeid + p1[i];
        if (excl & sum->excl) return 0;
        excl |= sum->excl;
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
    idx = hi_put_idx(TM_XXXID_BY_TLIDHASH, tm->intid_by_tlidhash, tlid, &outcome);
    switch (outcome) {
        default:
            die("%s: HI_TOMBSTONE2!", FN_NAME);
        case HI_LIVE:
            // typelist already exists
            intid = tm->intid_by_tlidhash->tokens[idx];
            if (btypeid == 0) return tm->btypid_by_intid[intid];
            else if (btypeid == tm->btypid_by_intid[intid]) return btypeid;
            else return 0;
        case HI_EMPTY:
            // missing so commit the intersection type for tlid
            if (btypeid == 0)
                btypeid = tm->next_btypeId;
            else if (btypeid < tm->next_btypeId && tm->summary_by_btypeid[btypeid].bmtid != bmterr)
                // btypeid is already in use so given the type list lookup above we cannot be referring to the same btype
                return B_NAT;
            intid = tm->next_intid++;
            if (intid >= tm->max_intid) {
                tm->max_intid += TM_MAX_ID_INC_SIZE;
                _growTo((void **)&tm->tlid_by_intid, tm->max_intid * sizeof(TM_TLID_T), tm->mm, FN_NAME);
                _growTo((void **)&tm->btypid_by_intid, tm->max_intid * sizeof(btypeid_t), tm->mm, FN_NAME);
            }
            tm->tlid_by_intid[intid] = tlid;
            _new_type_summary_at(tm, bmtint, excl, btypeid, intid);
            tm->btypid_by_intid[intid] = btypeid;
            hi_replace_empty(TM_XXXID_BY_TLIDHASH, tm->intid_by_tlidhash, idx, intid);
            return btypeid;
    }
}

pub btypeid_t * tm_inter_tl(BK_TM *tm, btypeid_t btypeid) {
    struct btsummary *sum;
    sum = tm->summary_by_btypeid + btypeid;
    if (sum->bmtid == bmtint) {
        return tm->typelist_buf + tm->tlrp_by_tlid[tm->tlid_by_intid[sum->intId]];
    } else {
        return 0;
    }
}

pub btypeid_t tm_map(BK_TM *tm, btypeid_t tK, btypeid_t tV, btypeid_t btypeid) {
    i32 outcome;  TM_XXXID_T mapid;  TM_T1T2 t1t2;  u32 idx;

    // answers the validated function type corresponding to tK and tV, creating if necessary

    // check each typeid is valid
    if (!(0 < tK && tK < tm->next_btypeId)) return 0;
    if ((tm->summary_by_btypeid + tK)->bmtid == bmterr) return 0;
    if (!(0 < tV && tV < tm->next_btypeId)) return 0;
    if ((tm->summary_by_btypeid + tV)->bmtid == bmterr) return 0;

    t1t2.tK = tK;
    t1t2.tV = tV;

    // get the btypeid for the t1t2
    idx = hi_put_idx(TM_XXXID_BY_T1T2HASH, tm->mapid_by_t1t2hash, t1t2, &outcome);
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
            else if (btypeid < tm->next_btypeId && tm->summary_by_btypeid[btypeid].bmtid != bmterr)
                // btypeid is already in use so given the t1t2 lookup above we cannot be referring to the same btype
                return B_NAT;
            mapid = tm->next_mapid++;
            if (mapid >= tm->max_mapid) {
                tm->max_mapid += TM_MAX_ID_INC_SIZE;
                _growTo((void **)&tm->t1t2_by_mapid, tm->max_mapid * sizeof(TM_T1T2), tm->mm, FN_NAME);
                _growTo((void **)&tm->btypid_by_mapid, tm->max_mapid * sizeof(btypeid_t), tm->mm, FN_NAME);
            }
            tm->t1t2_by_mapid[mapid] = t1t2;
            _new_type_summary_at(tm, bmtmap, btenone, btypeid, mapid);
            tm->btypid_by_mapid[mapid] = btypeid;
            hi_replace_empty(TM_XXXID_BY_T1T2HASH, tm->mapid_by_t1t2hash, idx, mapid);
            return btypeid;
    }
}

pub TM_T1T2 tm_Map(BK_TM *tm, btypeid_t btypeid) {
    if (btypeid < 1 || btypeid >= tm->next_btypeId) return (TM_T1T2) {{0}, {0}};
    struct btsummary *sum = tm->summary_by_btypeid + btypeid;       // OPEN: in general use pointer to summary rather than copying the struct
    if (sum->bmtid != bmtmap) return (TM_T1T2) {{0}, {0}};
    return tm->t1t2_by_mapid[sum->mapId];
}

pub char * tm_name(BK_TM *tm, btypeid_t btypeid) {

    // answers the name of the given type or a null pointer it has no name
    if (btypeid <= 0 || btypeid >= tm->next_btypeId) return 0;
    symid_t symid = tm->symid_by_btypeid[btypeid];
    return symid ? sm_name(tm->sm, symid) : 0;
}

pub btypeid_t tm_name_as(BK_TM *tm, btypeid_t btypeid, char *name) {
    int outcome;  symid_t symid;  u32 idx;

    // assigns name to the unnamed btypedid, checking that name is not already used
    if (btypeid <= 0 || btypeid >= tm->next_btypeId)
        return B_NAT;
    else {
        if ((symid = tm->symid_by_btypeid[btypeid]) != 0)
            // already named - check the given name is the same as the existing name
            return strcmp(sm_name(tm->sm, sm_id_2_RP(tm->sm, symid)), name) == 0 ? btypeid : B_NAT;
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

pub btypeid_t tm_nominal(BK_TM *tm, char *name, btypeid_t btypeid) {
    int outcome;  symid_t symid;  u32 idx;  struct btsummary sum;

    // answers the validated nominal type corresponding to name, creating if necessary
    if (btypeid && btypeid < tm->next_btypeId && (sum = tm->summary_by_btypeid[btypeid]).bmtid != bmterr) {
        // there is already a type with id btypeid so check we are referring to the same type
        if (sum.bmtid != bmtnom || sum.excl != btenone || strcmp(name, tm_name(tm, btypeid)) != 0) return B_NAT;
        return btypeid;
    } else {
        // if name is not already in use create a new nominal
        symid = sm_id(tm->sm, name);
        idx = hi_put_idx(TM_BTYPEID_BY_SYMIDHASH, tm->btypeid_by_symidhash, symid, &outcome);
        if (outcome == HI_LIVE) {
            // already exists so check it's a nominal with no exclusion
            btypeid = hi_token(tm->btypeid_by_symidhash, idx);
            sum = tm->summary_by_btypeid[btypeid];
            if (sum.bmtid != bmtnom || sum.excl != btenone) return B_NAT;
            return btypeid;
        } else {
            if (btypeid == 0) btypeid = tm->next_btypeId;
            hi_replace_empty(TM_BTYPEID_BY_SYMIDHASH, tm->btypeid_by_symidhash, idx, btypeid);
            tm->symid_by_btypeid[btypeid] = symid;
            _new_type_summary_at(tm, bmtnom, btenone, btypeid, 0);
            return btypeid;
        }
    }
}

pub btypeid_t tm_schemavar(BK_TM *tm, char *name, btypeid_t btypeid) {
    int outcome;  symid_t symid;  u32 idx;  struct btsummary sum;

    // answers the validated schema variable corresponding to name, creating if necessary
    if (btypeid && btypeid < tm->next_btypeId && (sum = tm->summary_by_btypeid[btypeid]).bmtid != bmterr) {
        // there is already a type with id btypeid so check we are referring to the same type
        if (sum.bmtid != bmtsvr || sum.excl != btenone || strcmp(name, tm_name(tm, btypeid)) != 0) return B_NAT;
        return btypeid;
    } else {
        // if name is not already in use create a new schema variable
        symid = sm_id(tm->sm, name);
        idx = hi_put_idx(TM_BTYPEID_BY_SYMIDHASH, tm->btypeid_by_symidhash, symid, &outcome);
        if (outcome == HI_LIVE) {
            // already exists so check it's a schema variable
            btypeid = hi_token(tm->btypeid_by_symidhash, idx);
            sum = tm->summary_by_btypeid[btypeid];
            if (sum.bmtid != bmtsvr || sum.excl != btenone) return B_NAT;
            return btypeid;
        } else {
            if (btypeid == 0) btypeid = tm->next_btypeId;
            hi_replace_empty(TM_BTYPEID_BY_SYMIDHASH, tm->btypeid_by_symidhash, idx, btypeid);
            tm->symid_by_btypeid[btypeid] = symid;
            _new_type_summary_at(tm, bmtsvr, btenone, btypeid, 0);
            return btypeid;
        }
    }
}

pub btypeid_t tm_seq(BK_TM *tm, btypeid_t containedid, btypeid_t btypeid) {
    i32 outcome;  struct btsummary *sum;  btypeid_t containerid;  u32 idx;

    // answers the validated sequence type corresponding to tContained, creating if necessary

    // check that containedid is valid
    if (!(0 < containedid && containedid < tm->next_btypeId)) return 0;
    sum = tm->summary_by_btypeid + containedid;
    if (sum->bmtid == bmterr) return 0;

    // get the btypeid for the tContained
    idx = hi_put_idx(TM_BTYPID_BY_SEQIDHASH, tm->containerid_by_containedidhash, containedid, &outcome);
    switch (outcome) {
        default:
            die("%s:%i: HI_TOMBSTONE2!", FN_NAME, __LINE__);
        case HI_LIVE:
            containerid = tm->containerid_by_containedidhash->tokens[idx];
            if (btypeid == 0) return containerid;
            else if (btypeid == containerid) return btypeid;
            else return 0;
        case HI_EMPTY:
            // missing so commit the tuple type for tlid
            if (btypeid == 0)
                btypeid = tm->next_btypeId;
            else if (btypeid < tm->next_btypeId && tm->summary_by_btypeid[btypeid].bmtid != bmterr)
                // btypeid is already in use so given the type list lookup above we cannot be referring to the same btype
                return 0;
            _new_type_summary_at(tm, bmtseq, btenone, btypeid, containedid);
            hi_replace_empty(TM_BTYPID_BY_SEQIDHASH, tm->containerid_by_containedidhash, idx, btypeid);
            return btypeid;
    }
}

pub btypeid_t tm_seq_t(BK_TM *tm, btypeid_t btypeid) {
    struct btsummary *sum;
    // OPEN: do we bounds check btypeid here?
    sum = tm->summary_by_btypeid + btypeid;
    return sum->bmtid == bmtseq ? sum->seqId : 0;
}

pub size tm_size(BK_TM *tm, btypeid_t btypeid) {
    // OPEN: implement (requires packing decisions which should be put in the client? except the mm needs to be able to navigate)
    return 8;
}

pub btypeid_t tm_size_as(BK_TM *tm, btypeid_t btypeid, size sz) {
    return 0;
}

pub btypeid_t tm_tuple(BK_TM *tm, btypeid_t *typelist, btypeid_t btypeid) {
    i32 i, outcome, numTypes;  struct btsummary *sum;  TM_XXXID_T tupid;  TM_TLID_T tlid;  btypeid_t *p1, *nextTypelist;
    u32 idx;

    // answers the validated tuple type corresponding to typelist, creating if necessary
    if (!(numTypes = typelist[0])) return 0;

    // check each typeid in the list is valid
    // OPEN: can this loop be merged with the copying loop?
    for (i = 1; i <= (i32)typelist[0]; i++) {
        if (!(0 < typelist[i] && typelist[i] < tm->next_btypeId)) return 0;
        sum = tm->summary_by_btypeid + typelist[i];
        if (sum->bmtid == bmterr) return 0;
    }

    // make next page of tm->typelist_buf writable if necessary
    if (tm->next_tlrp + numTypes >= tm->max_tlrp) {
        if (tm->next_tlrp + numTypes >= TM_MAX_TL_STORAGE) die("%s: out of typelist storage", FN_NAME);  // OPEN: really we should add an error reporting mechanism, e.g. TM_ERR_OUT_OF_NAME_STORAGE, etc
        size_t pageSize = os_page_size();
        os_mprotect(tm->typelist_buf + tm->max_tlrp, pageSize, BK_M_READ | BK_M_WRITE);
        os_madvise(tm->typelist_buf + tm->max_tlrp, pageSize, BK_AD_RANDOM);
    }

    nextTypelist = tm->typelist_buf + tm->next_tlrp;

    // copy typelist into typelist_buf
    p1 = nextTypelist;
    *p1++ = numTypes;
    for (i = 1; i <= (i32)typelist[0]; i++) {
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
            if (!tlid) return 0;       // an error occurred OPEN handle properly
    }

    // get the btypeid for the tlid
    idx = hi_put_idx(TM_XXXID_BY_TLIDHASH, tm->tupid_by_tlidhash, tlid, &outcome);
    switch (outcome) {
        default:
            die("%s:%i: HI_TOMBSTONE2!", FN_NAME, __LINE__);
        case HI_LIVE:
            tupid = tm->tupid_by_tlidhash->tokens[idx];
            if (btypeid == 0) return tm->btypid_by_tupid[tupid];
            else if (btypeid == tm->btypid_by_tupid[tupid]) return btypeid;
            else return 0;
        case HI_EMPTY:
            // missing so commit the tuple type for tlid
            if (btypeid == 0)
                btypeid = tm->next_btypeId;
            else if (btypeid < tm->next_btypeId && tm->summary_by_btypeid[btypeid].bmtid != bmterr)
                // btypeid is already in use so given the type list lookup above we cannot be referring to the same btype
                return B_NAT;
            tupid = tm->next_tupid++;
            if (tupid >= tm->max_tupid) {
                tm->max_tupid += TM_MAX_ID_INC_SIZE;
                _growTo((void **)&tm->tlid_by_tupid, tm->max_tupid * sizeof(TM_TLID_T), tm->mm, FN_NAME);
                _growTo((void **)&tm->btypid_by_tupid, tm->max_tupid * sizeof(btypeid_t), tm->mm, FN_NAME);
            }
            tm->tlid_by_tupid[tupid] = tlid;
            _new_type_summary_at(tm, bmttup, btenone, btypeid, tupid);
            tm->btypid_by_tupid[tupid] = btypeid;
            hi_replace_empty(TM_XXXID_BY_TLIDHASH, tm->tupid_by_tlidhash, idx, tupid);
            return btypeid;
    }
}

pub btypeid_t * tm_tuple_tl(BK_TM *tm, btypeid_t btypeid) {
    struct btsummary *sum;
    // OPEN: do we bounds check btypeid here?
    sum = tm->summary_by_btypeid + btypeid;
    if (sum->bmtid == bmttup) {
        return tm->typelist_buf + tm->tlrp_by_tlid[tm->tlid_by_tupid[sum->tupId]];
    } else {
        return 0;
    }
}

pub btypeid_t tm_union(BK_TM *tm, btypeid_t *typelist, btypeid_t btypeid) {
    i32 i, j, outcome, numTypes;  struct btsummary *sum;  TM_XXXID_T uniid;  TM_TLID_T tlid;
    btypeid_t *uniTl, *p1, *p2, *p3, *nextTypelist;  u32 idx;

    if (!(numTypes = typelist[0])) return 0;

    // check typeid is in range, and figure total possible length (including possible duplicate from child unions)
    for (i = 1; i <= (i32)typelist[0]; i++) {
        if (!(0 < typelist[i] && typelist[i] < tm->next_btypeId)) return 0;
        sum = tm->summary_by_btypeid + typelist[i];
        if (sum->bmtid == bmtuni) {
            tlid = tm->tlid_by_uniid[sum->uniId];
            numTypes += (tm->typelist_buf + tm->tlrp_by_tlid[tlid])[0] - 1;
        }
    }

    // make next page of tm->typelist_buf writable if necessary
    if (tm->next_tlrp + numTypes >= tm->max_tlrp) {
        if (tm->next_tlrp + numTypes >= TM_MAX_TL_STORAGE) die("%s: out of typelist storage", FN_NAME);  // OPEN: really we should add an error reporting mechanism, e.g. TM_ERR_OUT_OF_NAME_STORAGE, etc
        size_t pageSize = os_page_size();
        os_mprotect(tm->typelist_buf + tm->max_tlrp, pageSize, BK_M_READ | BK_M_WRITE);
        os_madvise(tm->typelist_buf + tm->max_tlrp, pageSize, BK_AD_RANDOM);
    }

    nextTypelist = tm->typelist_buf + tm->next_tlrp;

    // copy typelist into typelist_buf unpacking any unions
    p1 = nextTypelist;
    *p1++ = numTypes;
    for (i = 1; i <= (i32)typelist[0]; i++) {
        sum = tm->summary_by_btypeid + typelist[i];
        if (sum->bmtid == bmtuni) {
            // we have a union type - expand it
            tlid = tm->tlid_by_uniid[sum->uniId];
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
    numTypes = *nextTypelist = p1 - nextTypelist;

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
    idx = hi_put_idx(TM_XXXID_BY_TLIDHASH, tm->uniid_by_tlidhash, tlid, &outcome);
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
            else if (btypeid < tm->next_btypeId && tm->summary_by_btypeid[btypeid].bmtid != bmterr)
                // btypeid is already in use so given the type list lookup above we cannot be referring to the same btype
                return B_NAT;
            uniid = tm->next_uniid++;
            if (uniid >= tm->max_uniid) {
                tm->max_uniid += TM_MAX_ID_INC_SIZE;
                _growTo((void **)&tm->tlid_by_uniid, tm->max_uniid * sizeof(TM_TLID_T), tm->mm, FN_NAME);
                _growTo((void **)&tm->btypid_by_uniid, tm->max_uniid * sizeof(btypeid_t), tm->mm, FN_NAME);
            }
            tm->tlid_by_uniid[uniid] = tlid;
            _new_type_summary_at(tm, bmtuni, btenone, btypeid, uniid);
            tm->btypid_by_uniid[uniid] = btypeid;
            hi_replace_empty(TM_XXXID_BY_TLIDHASH, tm->uniid_by_tlidhash, idx, uniid);
            return btypeid;
    }
}

pub btypeid_t * tm_union_tl(BK_TM *tm, btypeid_t btypeid) {
    // answer pointer to the typelist of the union btypeid or 0 for error
    struct btsummary *sum;
    sum = tm->summary_by_btypeid + btypeid;
    if (sum->bmtid == bmtuni) {
        return tm->typelist_buf + tm->tlrp_by_tlid[tm->tlid_by_uniid[sum->uniId]];
    } else {
        return 0;
    }
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

pub BK_TM * TM_create(BK_MM *mm, Buckets *buckets, BK_SM *sm, struct TPM *tp) {
    // OPEN: should we use calloc instead of memset to init arrays to zero?
    BK_TM *tm = (BK_TM *) mm->malloc(sizeof(BK_TM));
    tm->mm = mm;
    tm->buckets = buckets;
    tm->sm = sm;
    tm->tp = tp;
    tm->typelist_buf = os_vm_reserve(0, TM_MAX_TL_STORAGE);

    // typelists
    tm->max_tlrp = os_page_size() / sizeof(TM_TLID_T);
    os_mprotect(tm->typelist_buf, tm->max_tlrp * sizeof(TM_TLID_T), BK_M_READ | BK_M_WRITE);      // make first page of typelist storage R/W
    os_madvise(tm->typelist_buf, tm->max_tlrp * sizeof(TM_TLID_T), BK_AD_RANDOM);                 // and advise as randomly accessed
    tm->next_tlrp = 0;

    tm->max_tlid = TM_MAX_TLID_INC_SIZE;
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
    tm->symid_by_btypeid = (TM_XXXID_T *) mm->malloc(tm->max_btypeId * sizeof(TM_XXXID_T));
    memset(tm->symid_by_btypeid, 0, tm->max_btypeId * sizeof(btypeid_t));

    // type summaries
    tm->summary_by_btypeid = (struct btsummary *) mm->malloc(tm->max_btypeId * sizeof(struct btsummary));
    memset(tm->summary_by_btypeid, 0, tm->max_btypeId * sizeof(struct btsummary));

    // intersections
    tm->max_intid = TM_MAX_ID_INC_SIZE;
    tm->next_intid = 1;
    tm->tlid_by_intid = (TM_TLID_T *) mm->malloc(tm->max_intid * sizeof(TM_TLID_T));
    memset(tm->tlid_by_intid, 0, tm->max_intid * sizeof(TM_TLID_T));
    tm->btypid_by_intid = (TM_XXXID_T *) mm->malloc(tm->max_intid * sizeof(TM_XXXID_T));
    memset(tm->btypid_by_intid, 0, tm->max_intid * sizeof(TM_XXXID_T));
    tm->intid_by_tlidhash = hi_create(TM_XXXID_BY_TLIDHASH);
    tm->intid_by_tlidhash->tlid_by_xxxid = tm->tlid_by_intid;

    // unions
    tm->max_uniid = TM_MAX_ID_INC_SIZE;
    tm->next_uniid = 1;
    tm->tlid_by_uniid = (TM_TLID_T *) mm->malloc(tm->max_uniid * sizeof(TM_TLID_T));
    memset(tm->tlid_by_uniid, 0, tm->max_uniid * sizeof(TM_TLID_T));
    tm->btypid_by_uniid = (TM_XXXID_T *) mm->malloc(tm->max_uniid * sizeof(TM_XXXID_T));
    memset(tm->btypid_by_uniid, 0, tm->max_uniid * sizeof(TM_XXXID_T));
    tm->uniid_by_tlidhash = hi_create(TM_XXXID_BY_TLIDHASH);
    tm->uniid_by_tlidhash->tlid_by_xxxid = tm->tlid_by_uniid;

    // tuples
    tm->max_tupid = TM_MAX_ID_INC_SIZE;
    tm->next_tupid = 1;
    tm->tlid_by_tupid = (TM_TLID_T *) mm->malloc(tm->max_tupid * sizeof(TM_TLID_T));
    memset(tm->tlid_by_tupid, 0, tm->max_tupid * sizeof(TM_TLID_T));
    tm->btypid_by_tupid = (btypeid_t *) mm->malloc(tm->max_tupid * sizeof(btypeid_t));
    memset(tm->btypid_by_tupid, 0, tm->max_tupid * sizeof(btypeid_t));
    tm->tupid_by_tlidhash = hi_create(TM_XXXID_BY_TLIDHASH);
    tm->tupid_by_tlidhash->tlid_by_xxxid = tm->tlid_by_tupid;

    // structs
    tm->max_strid = TM_MAX_ID_INC_SIZE;
    tm->next_strid = 1;

    // records
//    tm->max_recid = TM_MAX_ID_INC_SIZE;
//    tm->next_recid = 1;

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
    tm->mapid_by_t1t2hash = hi_create(TM_XXXID_BY_T1T2HASH);
    tm->mapid_by_t1t2hash->t1t2_by_xxxid = tm->t1t2_by_mapid;

    // functions
    tm->max_fncid = TM_MAX_ID_INC_SIZE;
    tm->next_fncid = 1;
    tm->t1t2_by_fncid = (TM_T1T2 *) mm->malloc(tm->max_fncid * sizeof(TM_T1T2));
    memset(tm->t1t2_by_fncid, 0, tm->max_fncid * sizeof(TM_T1T2));
    tm->btypid_by_fncid = (btypeid_t *) mm->malloc(tm->max_fncid * sizeof(btypeid_t));
    memset(tm->btypid_by_fncid, 0, tm->max_fncid * sizeof(btypeid_t));
    tm->fncid_by_t1t2hash = hi_create(TM_XXXID_BY_T1T2HASH);
    tm->fncid_by_t1t2hash->t1t2_by_xxxid = tm->t1t2_by_fncid;

    // schema variables
    tm->max_svrid = TM_MAX_ID_INC_SIZE;
    tm->next_svrid = 1;

    return tm;
}

pub int TM_trash(BK_TM *tm) {
    // typelists
    tm->mm->free(tm->tlrp_by_tlid);
    hi_trash(TM_TLID_BY_TLHASH, tm->tlid_by_tlhash);

    // type names
    hi_trash(TM_BTYPEID_BY_SYMIDHASH, tm->btypeid_by_symidhash);
    tm->mm->free(tm->symid_by_btypeid);

    // type summaries
    tm->mm->free(tm->summary_by_btypeid);

    // intersections
    tm->mm->free(tm->tlid_by_intid);
    tm->mm->free(tm->btypid_by_intid);
    hi_trash(TM_XXXID_BY_TLIDHASH, tm->intid_by_tlidhash);

    // unions
    tm->mm->free(tm->tlid_by_uniid);
    tm->mm->free(tm->btypid_by_uniid);
    hi_trash(TM_XXXID_BY_TLIDHASH, tm->uniid_by_tlidhash);

    // tuples
    tm->mm->free(tm->tlid_by_tupid);
    tm->mm->free(tm->btypid_by_tupid);
    hi_trash(TM_XXXID_BY_TLIDHASH, tm->tupid_by_tlidhash);

    // structs

    // records

    // sequences
    hi_trash(TM_BTYPID_BY_SEQIDHASH, tm->containerid_by_containedidhash);

    // maps
    tm->mm->free(tm->t1t2_by_mapid);
    tm->mm->free(tm->btypid_by_mapid);
    hi_trash(TM_XXXID_BY_T1T2HASH, tm->mapid_by_t1t2hash);

    // functions
    tm->mm->free(tm->t1t2_by_fncid);
    tm->mm->free(tm->btypid_by_fncid);
    hi_trash(TM_XXXID_BY_T1T2HASH, tm->fncid_by_t1t2hash);

    // schema variables

    // self
    tm->mm->free(tm);
    return 0;
}




pub btypeid_t tm_interv(BK_TM *tm, u32 numTypes, ...) {
    va_list args;  btypeid_t *typelist;  int i;  btypeid_t btypeid;
    va_start(args, numTypes);
    typelist = malloc((1 + numTypes) * sizeof(btypeid_t));
    for (i = 1; i <= numTypes; i++) typelist[i] = va_arg(args, btypeid_t);
    typelist[0] = numTypes;
    btypeid = tm_inter(tm, typelist, 0);
    free(typelist);
    va_end(args);
    return btypeid;
}

pub btypeid_t _intersect2(BK_TM *tm, u32 numTypes, btypeid_t *args) {
    btypeid_t *typelist;  int i;  btypeid_t btypeid;
    typelist = malloc((1 + numTypes) * sizeof(btypeid_t));      // OPEN: use a typelist buffer of big enough size
    for (i = 1; i <= numTypes; i++) typelist[i] = args[i-1];
    typelist[0] = numTypes;
    btypeid = tm_inter(tm, typelist, 0);
    free(typelist);
    return btypeid;
}

#define BK_INTERSECTION(tm, ...) ({                                                                                     \
    btypeid_t args[] = { __VA_ARGS__ };                                                                                 \
    _intersect2((tm), sizeof(args) / sizeof(args[0]), args);                                                            \
})

pub btypeid_t _union2(BK_TM *tm, u32 numTypes, btypeid_t *args) {
    btypeid_t *typelist;  int i;  btypeid_t btypeid;
    typelist = malloc((1 + numTypes) * sizeof(btypeid_t));
    for (i = 1; i <= numTypes; i++) typelist[i] = args[i-1];
    typelist[0] = numTypes;
    btypeid = tm_union(tm, typelist, 0);
    free(typelist);
    return btypeid;
}

#define BK_UNION(tm, ...) ({                                                                                            \
    btypeid_t args[] = { __VA_ARGS__ };                                                                                 \
    _union2((tm), sizeof(args) / sizeof(args[0]), args);                                                                \
})



#endif  // __BK_TM_C
