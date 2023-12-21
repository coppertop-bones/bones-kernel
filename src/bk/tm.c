// ---------------------------------------------------------------------------------------------------------------------
//                                                    Type Manager
// ---------------------------------------------------------------------------------------------------------------------

#ifndef __BK_TM_C
#define __BK_TM_C "bk/tm.c"


#include "buckets.c"
#include "../../include/bk/mm.h"
#include "../../include/bk/tm.h"
#include "../../include/bk/tp.h"
#include "lib/ht_impl.h"
#include "lib/radix.h"


KRADIX_SORT_INIT(btypeid_t, btypeid_t, ,sizeof(btypeid_t))


// ---------------------------------------------------------------------------------------------------------------------
// symid_by_btypeid hash fns
// ---------------------------------------------------------------------------------------------------------------------

pvt inline symid_t symIdFromEntry(ht_struct(TM_BTYPEID_BY_SYMIDHASH) *ht, symid_t entry) {
    return ht->tm->symid_by_btypeid[entry];
}

pvt bool inline found(ht_struct(TM_BTYPEID_BY_SYMIDHASH) *ht, symid_t entry, btypeid_t key) {
    return ht->tm->symid_by_btypeid[entry] == key;
}

// HT_IMPL(name, slot_t, key_t, __hash_fn, __found_fn, __key_from_entry_fn)
HT_IMPL(TM_BTYPEID_BY_SYMIDHASH, btypeid_t, symid_t, ht_int32_hash, found, symIdFromEntry)


// ---------------------------------------------------------------------------------------------------------------------
// rp_by_tlid hash fns
// ---------------------------------------------------------------------------------------------------------------------

pvt inline btypeid_t * tlFromEntry(ht_struct(TM_TLID_BY_TLHASH) *ht, TM_TLID_T entry) {
    return ht->tm->typelist_buf + ht->tm->rp_by_tlid[entry];
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

pvt bool inline tlFound(ht_struct(TM_TLID_BY_TLHASH) *ht, TM_TLID_T entry, btypeid_t *key) {
    return tlCompare(tlFromEntry(ht, entry), key);
}

// HT_IMPL(name, slot_t, key_t, __hash_fn, __found_fn, __key_from_entry_fn)
HT_IMPL(TM_TLID_BY_TLHASH, TM_TLID_T, btypeid_t *, tl_hash, tlFound, tlFromEntry)


// ---------------------------------------------------------------------------------------------------------------------
// TM_XXXID_BY_TLIDHASH fns
// ---------------------------------------------------------------------------------------------------------------------

pvt inline TM_TLID_T tlidFromEntry(ht_struct(TM_XXXID_BY_TLIDHASH) *ht, TM_XXXID_T entry) {
    return ht->tlid_by_xxxid[entry];
}

pvt bool inline tlidFound(ht_struct(TM_XXXID_BY_TLIDHASH) *ht, TM_XXXID_T entry, TM_TLID_T key) {
    return ht->tlid_by_xxxid[entry] == key;
}

// HT_IMPL(name, slot_t, key_t, __hash_fn, __found_fn, __key_from_entry_fn)
HT_IMPL(TM_XXXID_BY_TLIDHASH, u32, u32, ht_int32_hash, tlidFound, tlidFromEntry)



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
        _growTo((void **)&tm->rp_by_tlid, tm->max_tlid * sizeof(RP), tm->mm, FN_NAME);
        tm->intid_by_tlidhash->tlid_by_xxxid = tm->tlid_by_intid;
    }
    tm->rp_by_tlid[tlid] = tm->next_rp;
    ht_replace_empty(TM_TLID_BY_TLHASH, tm->tlid_by_tlhash, idx, tlid);
    if (tm->next_rp + numTypes + 1 >= tm->max_rp) {
        size_t pageSize = os_page_size();
        os_mprotect(tm->typelist_buf + tm->max_rp - pageSize, pageSize, BK_M_READ);     // make the prior last page read only
        tm->max_rp += pageSize / sizeof(TM_TLID_T);
    }
    tm->next_rp += numTypes + 1;
    return tlid;
}

tdd btypeid_t _new_type_summary_at(BK_TM *tm, bmetatypeid_t bmtid, btexclusioncat_t excl, u32 idx, symid_t symid, btypeid_t id) {
    btypeid_t btypeid = tm->next_btypeId++;
    if (btypeid >= tm->max_btypeId) {
        tm->max_btypeId += TM_MAX_BTYPEID_INC_SIZE;
        _growTo((void **)&tm->summary_by_btypeid, tm->max_btypeId * sizeof(struct btsummary), tm->mm, FN_NAME);
        _growTo((void **)&tm->symid_by_btypeid, tm->max_intid * sizeof(symid_t), tm->mm, FN_NAME);
    }
    tm->symid_by_btypeid[btypeid] = symid;
    tm->summary_by_btypeid[btypeid].bmtid = bmtid;
    tm->summary_by_btypeid[btypeid].excl = excl;
    // OPEN switch on bmtid
    tm->summary_by_btypeid[btypeid].intId = id;
    ht_replace_empty(TM_BTYPEID_BY_SYMIDHASH, tm->btypeid_by_symidhash, idx, btypeid);
    return btypeid;
}


// ---------------------------------------------------------------------------------------------------------------------
// pretty printing
// ---------------------------------------------------------------------------------------------------------------------

pvt void tm_pb(BK_TM *tm, BK_TP *tp, btypeid_t btypeid) {
    struct btsummary *sum;
    symid_t symid;
    btypeid_t *tl;
    i32 i;
    char sep;
    if ((symid = tm->symid_by_btypeid[btypeid])) {
        tp_printfb(tp, "%s", sm_name(tm->sm, symid));
    } else {
        sum = tm->summary_by_btypeid + btypeid;
        switch (sum->bmtid) {
            case bmtnom:
                tp_printfb(tp, "%s", sm_name(tm->sm, symid));
                break;
            case bmtint:
                tl = tm->typelist_buf + tm->rp_by_tlid[tm->tlid_by_intid[sum->intId]];
                sep = 0;
                for (i = 1; i <= (i32) tl[0]; i++) {
                    if (sep) tp_printfb(tp, " & ");
                    sep = 1;
                    tm_pb(tm, tp, tl[i]);
                }
                break;
            case bmttup:
                tp_printfb(tp, "tup");
                break;
            case bmtuni:
                tp_printfb(tp, "uni");
                break;
            default:
                tp_printfb(tp, "NAT");
        }
    }
}
pvt inline TPN tm_tp(BK_TM *tm, BK_TP *tp, btypeid_t btypeid) {tm_pb(tm, tp, btypeid); return tp_flush(tp);}
pvt inline S8 tm_pp(BK_TM *tm, BK_TP *tp, btypeid_t btypeid) {tm_pb(tm, tp, btypeid); return tp_render(tp, tp_flush(tp));}


pvt void tm_pb_typelist(BK_TM *tm, BK_TP *tp, btypeid_t *typelist) {
    int firstTime = 1;
    for (u32 i = 1; i < typelist[0] + 1; i++) {
        if (firstTime) {
            firstTime = 0;
            tm_pb(tm, tp, typelist[i]);
        }
        else {
            tp_printfb(tp, ", ");
            tm_pb(tm, tp, typelist[i]);
        }
    }
}
pvt inline TPN tm_tp_typelist(BK_TM *tm, BK_TP *tp, btypeid_t *typelist) {tm_pb_typelist(tm, tp, typelist); return tp_flush(tp);}
pvt inline S8 tm_pp_typelist(BK_TM *tm, BK_TP *tp, btypeid_t *typelist) {tm_pb_typelist(tm, tp, typelist); return tp_render(tp, tp_flush(tp));}



// ---------------------------------------------------------------------------------------------------------------------
// type accessing / creation fns
// ---------------------------------------------------------------------------------------------------------------------

pub btypeid_t tm_btypeid(BK_TM *tm, char *name) {
    int res;  u32 idx;
    idx = ht_put_idx(TM_BTYPEID_BY_SYMIDHASH, tm->btypeid_by_symidhash, sm_id(tm->sm, name), &res);
    if (res == HT_EXISTS)
        return tm->btypeid_by_symidhash->slots[idx];
    else
        return 0;
}

pub btypeid_t tm_exclnominal(BK_TM *tm, char *name, btexclusioncat_t excl) {
    int res;  btypeid_t btypeid;  symid_t symid;  u32 idx;
    symid = sm_id(tm->sm, name);                                        // get symid of name
    idx = ht_put_idx(TM_BTYPEID_BY_SYMIDHASH, tm->btypeid_by_symidhash, symid, &res);      // get index of the btypeid corresponding to symid
    if (res == HT_EXISTS) {
        btypeid = ht_entry(tm->btypeid_by_symidhash, idx);
        struct btsummary summary = tm->summary_by_btypeid[btypeid];
        if (summary.excl != excl || summary.bmtid != bmtnom) return 0;      // check it's a nominal with the same exclusion
        return btypeid;
    }
    else {
        // create a new nominal in an exclusive category
        return _new_type_summary_at(tm, bmtnom, excl, idx, symid, 0);
    }
}

// set of values intersection ((1 2 3) + (4 5)) & ((1 2 3) + (6 7)) = (1 2 3 4 5) & (1 2 3 6 7) = (1 2 3)
// (int + str) & (int + bool) => (int+int) & (int+bool) & (str+int) & (str+bool)
// types only make sense in the context of fitsWithin a LHS might not behaviour as a RHS


pub btypeid_t tm_inter(BK_TM *tm, btypeid_t *typelist) {
    i32 i, j, res, numTypes, hasUnions;  btexclusioncat_t excl = 0;  TM_TLID_T tlid;
    btypeid_t btypeid, *interTl, *p1, *p2, *p3, *nextTypelist;
    TM_XXXID_T intid;  struct btsummary *sum;
    // (A&B) & (C&D)  = A & B & C & D
    // (A&B) & (B&C)  = A & B & C
    // (A+B) & (B+C)  = (A+B) & (B+C)  why not B? because we need to keep the detail when the program causes intersections
    //
    // (A&B) + (B&C)  = B & (A + C)    - not the same for unions of intersections

    // use tm->typelist_buf as scratch so don't have to allocate memory
    // OPEN: potentially though messy we could do intersections without child intersections in place in typelist to keep a little cache locality

    if (!(numTypes = typelist[0])) return 0;

    // check typeid is in range, and figure total possible length (including possible duplicate from child intersections)
    for (i = 1; i <= (i32)typelist[0]; i++) {
        if (!(0 < typelist[i] && typelist[i] < tm->next_btypeId)) return 0;
        sum = tm->summary_by_btypeid + typelist[i];
        if (sum->bmtid == bmtint) {
            tlid = tm->tlid_by_intid[sum->intId];
            numTypes += (tm->typelist_buf + tm->rp_by_tlid[tlid])[0] - 1;
        }
    }

    // grow tm->typelist_buf if necessary
    if (tm->next_rp + numTypes >= tm->max_rp) {
        if (tm->next_rp + numTypes >= TM_MAX_TL_STORAGE) die("%s: out of typelist storage", FN_NAME);  // OPEN: really we should add an error reporting mechanism, e.g. TM_ERR_OUT_OF_NAME_STORAGE, etc
        size_t pageSize = os_page_size();
        os_mprotect(tm->typelist_buf + tm->max_rp, pageSize, BK_M_READ | BK_M_WRITE);
        os_madvise(tm->typelist_buf + tm->max_rp, pageSize, BK_AD_RANDOM);
    }

    nextTypelist = tm->typelist_buf + tm->next_rp;

    // copy typelist into typelist_buf unpacking any intersections
    p1 = nextTypelist;
    *p1++ = numTypes;
    for (i = 1; i <= (i32)typelist[0]; i++) {
        sum = tm->summary_by_btypeid + typelist[i];
        if (sum->bmtid == bmtint) {
            // we have an intersection type - expand it
            tlid = tm->tlid_by_intid[sum->intId];
            interTl = (tm->typelist_buf + tm->rp_by_tlid[tlid]);
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
    if (hasUnions) {
        return 0;
    }

    // check for exclusion conflicts
    p1 = nextTypelist;
    for (i = 1; i <= numTypes; i++) {
        sum = tm->summary_by_btypeid + p1[i];
        if (excl & sum->excl) return 0;
        excl |= sum->excl;
    }

    // get the tlid for the typelist - adding if missing, returning 0 if invalid
    u32 idx = ht_put_idx(TM_TLID_BY_TLHASH, tm->tlid_by_tlhash, nextTypelist, &res);
    switch (res) {
        default:
            die("%s: HT_TOMBSTONE1!", FN_NAME);
        case HT_EXISTS:
            tlid = tm->tlid_by_tlhash->slots[idx];
            break;
        case HT_EMPTY:
            tlid = _commit_typelist_buf_at(tm, numTypes, idx);
            if (!tlid) return 0;       // an error occurred OPEN handle properly
    }

    // get the btypeid for the tlid
    idx = ht_put_idx(TM_XXXID_BY_TLIDHASH, tm->intid_by_tlidhash, tlid, &res);
    switch (res) {
        default:
            die("%s: HT_TOMBSTONE2!", FN_NAME);
        case HT_EXISTS:
            intid = tm->intid_by_tlidhash->slots[idx];
            return tm->btypid_by_intid[intid];
        case HT_EMPTY:
            // missing so commit the intersection type for tlid
            intid = tm->next_intid++;
            if (intid >= tm->max_intid) {
                tm->max_intid += TM_MAX_ID_INC_SIZE;
                _growTo((void **)&tm->tlid_by_intid, tm->max_intid * sizeof(TM_TLID_T), tm->mm, FN_NAME);
                _growTo((void **)&tm->btypid_by_intid, tm->max_intid * sizeof(btypeid_t), tm->mm, FN_NAME);
            }
            tm->tlid_by_intid[intid] = tlid;
            btypeid = _new_type_summary_at(tm, bmtint, excl, idx, 0, intid);
            tm->btypid_by_intid[intid] = btypeid;

            ht_replace_empty(TM_XXXID_BY_TLIDHASH, tm->intid_by_tlidhash, idx, intid);

            return btypeid;
    }
}

pub btypeid_t * tm_inter_tl(BK_TM *tm, btypeid_t btypeid) {
    struct btsummary *sum;
    sum = tm->summary_by_btypeid + btypeid;
    if (sum->bmtid == bmtint) {
        return tm->typelist_buf + tm->rp_by_tlid[tm->tlid_by_intid[sum->intId]];
    } else {
        return 0;
    }
}

pub char * tm_name(BK_TM *tm, btypeid_t btypeid) {
    // OPEN: do we return a null or "" if the type is unnamed? in Python we might want to return f't{btypeid}'
    symid_t symid;
    if (btypeid >= tm->next_btypeId) return "";
    symid = tm->symid_by_btypeid[btypeid];
    if (symid)
        return sm_name(tm->sm, symid);
    else
        return 0;
}

pub btypeid_t tm_name_as(BK_TM *tm, btypeid_t btypeid, char *name) {
    int res;  btypeid_t existingId;  symid_t symid;  u32 idx;
    symid = sm_id(tm->sm, name);
    idx = ht_put_idx(TM_BTYPEID_BY_SYMIDHASH, tm->btypeid_by_symidhash, symid, &res);
    if (res == HT_EXISTS) {
        existingId = ht_entry(tm->btypeid_by_symidhash, idx);
        return (existingId == btypeid) ? btypeid : 0;
    } else {
        ht_replace_empty(TM_BTYPEID_BY_SYMIDHASH, tm->btypeid_by_symidhash, idx, btypeid);
        tm->symid_by_btypeid[btypeid] = symid;
        return btypeid;
    }
}

pub btypeid_t tm_nominal(BK_TM *tm, char *name) {
    int res;  btypeid_t btypeid;  symid_t symid;  u32 idx;
    symid = sm_id(tm->sm, name);                                        // get symid of name
    idx = ht_put_idx(TM_BTYPEID_BY_SYMIDHASH, tm->btypeid_by_symidhash, symid, &res);      // get index of the btypeid corresponding to symid
    if (res == HT_EXISTS) {
        btypeid = ht_entry(tm->btypeid_by_symidhash, idx);
        return (tm->summary_by_btypeid[btypeid].bmtid == bmtnom) ? btypeid : 0;   // check it's a nominal
    }
    else {
        // create a new nominal
        return _new_type_summary_at(tm, bmtnom, 0, idx, symid, 0);
    }
}

tdd int tm_setNominalTo(BK_TM *tm, char *name, btypeid_t btypeid) {
     symid_t symid;  int res;  u32 idx;
    if (tm->summary_by_btypeid[btypeid].bmtid != bmterr) return 0;
    symid = sm_id(tm->sm, name);
    idx = ht_put_idx(TM_BTYPEID_BY_SYMIDHASH, tm->btypeid_by_symidhash, symid, &res);
    if (res == HT_EXISTS) return 0;

    tm->summary_by_btypeid[btypeid].bmtid = bmtnom;
    tm->symid_by_btypeid[btypeid] = symid;
    ht_replace_empty(TM_BTYPEID_BY_SYMIDHASH, tm->btypeid_by_symidhash, idx, btypeid);
    if (btypeid >= tm->next_btypeId) tm->next_btypeId = btypeid + 1;
    return btypeid;
}

pub size tm_size(BK_TM * tm, btypeid_t btypeid) {
    // OPEN: implement
    return 8;
}

pub btypeid_t tm_tuple(BK_TM *tm, btypeid_t *typelist) {
    // OPEN: implement
    return 0;
}

pub btypeid_t * tm_tuple_tl(BK_TM *tm, btypeid_t btypeid) {
    struct btsummary *sum;
    sum = tm->summary_by_btypeid + btypeid;
    if (sum->bmtid == bmttup) {
        return tm->typelist_buf + tm->rp_by_tlid[tm->tlid_by_tupid[sum->tupId]];
    } else {
        return 0;
    }
}

pub btypeid_t tm_union(BK_TM *tm, btypeid_t *typelist) {
    i32 i, j, res, numTypes;  struct btsummary *sum;  TM_XXXID_T uniid;  TM_TLID_T tlid;
    btypeid_t btypeid, *uniTl, *p1, *p2, *p3, *nextTypelist;
    if (!(numTypes = typelist[0])) return 0;

    // check typeid is in range, and figure total possible length (including possible duplicate from child unions)
    for (i = 1; i <= (i32)typelist[0]; i++) {
        if (!(0 < typelist[i] && typelist[i] < tm->next_btypeId)) return 0;
        sum = tm->summary_by_btypeid + typelist[i];
        if (sum->bmtid == bmtuni) {
            tlid = tm->tlid_by_uniid[sum->uniId];
            numTypes += (tm->typelist_buf + tm->rp_by_tlid[tlid])[0] - 1;
        }
    }

    // grow tm->typelist_buf if necessary
    if (tm->next_rp + numTypes >= tm->max_rp) {
        if (tm->next_rp + numTypes >= TM_MAX_TL_STORAGE) die("%s: out of typelist storage", FN_NAME);  // OPEN: really we should add an error reporting mechanism, e.g. TM_ERR_OUT_OF_NAME_STORAGE, etc
        size_t pageSize = os_page_size();
        os_mprotect(tm->typelist_buf + tm->max_rp, pageSize, BK_M_READ | BK_M_WRITE);
        os_madvise(tm->typelist_buf + tm->max_rp, pageSize, BK_AD_RANDOM);
    }

    nextTypelist = tm->typelist_buf + tm->next_rp;

    // copy typelist into typelist_buf unpacking any unions
    p1 = nextTypelist;
    *p1++ = numTypes;
    for (i = 1; i <= (i32)typelist[0]; i++) {
        sum = tm->summary_by_btypeid + typelist[i];
        if (sum->bmtid == bmtuni) {
            // we have a union type - expand it
            tlid = tm->tlid_by_uniid[sum->uniId];
            uniTl = (tm->typelist_buf + tm->rp_by_tlid[tlid]);
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
    u32 idx = ht_put_idx(TM_TLID_BY_TLHASH, tm->tlid_by_tlhash, nextTypelist, &res);
    switch (res) {
        default:
            die("%s: HT_TOMBSTONE1!", FN_NAME);
        case HT_EXISTS:
            tlid = tm->tlid_by_tlhash->slots[idx];
            break;
        case HT_EMPTY:
            tlid = _commit_typelist_buf_at(tm, numTypes, idx);
            if (!tlid) return 0;       // an error occurred OPEN handle properly
    }

    // get the btypeid for the tlid
    idx = ht_put_idx(TM_XXXID_BY_TLIDHASH, tm->uniid_by_tlidhash, tlid, &res);
    switch (res) {
        default:
            die("%s: HT_TOMBSTONE2!", FN_NAME);
        case HT_EXISTS:
            uniid = tm->uniid_by_tlidhash->slots[idx];
            return tm->btypid_by_uniid[uniid];
        case HT_EMPTY:
            // missing so commit the union type for tlid
            uniid = tm->next_uniid++;
            if (uniid >= tm->max_uniid) {
                tm->max_uniid += TM_MAX_ID_INC_SIZE;
                _growTo((void **)&tm->tlid_by_uniid, tm->max_uniid * sizeof(TM_TLID_T), tm->mm, FN_NAME);
                _growTo((void **)&tm->btypid_by_uniid, tm->max_uniid * sizeof(btypeid_t), tm->mm, FN_NAME);
            }
            tm->tlid_by_uniid[uniid] = tlid;
            btypeid = _new_type_summary_at(tm, bmtuni, 0, idx, 0, uniid);
            tm->btypid_by_uniid[uniid] = btypeid;

            ht_replace_empty(TM_XXXID_BY_TLIDHASH, tm->uniid_by_tlidhash, idx, uniid);

            return btypeid;
    }
}

pub btypeid_t * tm_union_tl(BK_TM *tm, btypeid_t btypeid) {
    struct btsummary *sum;
    sum = tm->summary_by_btypeid + btypeid;
    if (sum->bmtid == bmtuni) {
        return tm->typelist_buf + tm->rp_by_tlid[tm->tlid_by_uniid[sum->uniId]];
    } else {
        return 0;
    }
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
    tm->max_rp = os_page_size() / sizeof(TM_TLID_T);
    os_mprotect(tm->typelist_buf, tm->max_rp * sizeof(TM_TLID_T), BK_M_READ | BK_M_WRITE);      // make first page of typelist storage R/W
    os_madvise(tm->typelist_buf, tm->max_rp * sizeof(TM_TLID_T), BK_AD_RANDOM);                 // and advise as randomly accessed
    tm->next_rp = 0;

    tm->max_tlid = TM_MAX_TLID_INC_SIZE;
    tm->next_tlid = 1;
    tm->rp_by_tlid = (RP *) mm->malloc(tm->max_tlid * sizeof(RP));
    memset(tm->rp_by_tlid, 0, tm->max_tlid * sizeof(RP));
    tm->tlid_by_tlhash = ht_create(TM_TLID_BY_TLHASH);
    tm->tlid_by_tlhash->tm = tm;

    // type names
    tm->btypeid_by_symidhash = ht_create(TM_BTYPEID_BY_SYMIDHASH);
    tm->btypeid_by_symidhash->tm = tm;
    tm->max_btypeId = TM_MAX_BTYPEID_INC_SIZE;
    tm->next_btypeId = 1;
    tm->symid_by_btypeid = (btypeid_t *) mm->malloc(tm->max_btypeId * sizeof(btypeid_t));
    memset(tm->symid_by_btypeid, 0, tm->max_btypeId * sizeof(btypeid_t));

    // type summaries
    tm->summary_by_btypeid = (struct btsummary *) mm->malloc(tm->max_btypeId * sizeof(struct btsummary));
    memset(tm->summary_by_btypeid, 0, tm->max_btypeId * sizeof(struct btsummary));

    // intersections
    tm->max_intid = TM_MAX_ID_INC_SIZE;
    tm->next_intid = 1;
    tm->tlid_by_intid = (TM_TLID_T *) mm->malloc(tm->max_intid * sizeof(TM_TLID_T));
    memset(tm->tlid_by_intid, 0, tm->max_intid * sizeof(TM_TLID_T));
    tm->btypid_by_intid = (TM_TLID_T *) mm->malloc(tm->max_intid * sizeof(btypeid_t));
    memset(tm->summary_by_btypeid, 0, tm->max_intid * sizeof(btypeid_t));
    tm->intid_by_tlidhash = ht_create(TM_XXXID_BY_TLIDHASH);
    tm->intid_by_tlidhash->tlid_by_xxxid = tm->tlid_by_intid;

    // unions
    tm->max_uniid = TM_MAX_ID_INC_SIZE;
    tm->next_uniid = 1;
    tm->tlid_by_uniid = (TM_TLID_T *) mm->malloc(tm->max_uniid * sizeof(TM_TLID_T));
    memset(tm->tlid_by_uniid, 0, tm->max_uniid * sizeof(TM_TLID_T));
    tm->btypid_by_uniid = (TM_TLID_T *) mm->malloc(tm->max_uniid * sizeof(btypeid_t));
    memset(tm->summary_by_btypeid, 0, tm->max_uniid * sizeof(btypeid_t));
    tm->uniid_by_tlidhash = ht_create(TM_XXXID_BY_TLIDHASH);
    tm->uniid_by_tlidhash->tlid_by_xxxid = tm->tlid_by_uniid;

    // tuples
    tm->max_tupid = TM_MAX_ID_INC_SIZE;
    tm->next_tupid = 1;
    tm->tlid_by_tupid = (TM_TLID_T *) mm->malloc(tm->max_tupid * sizeof(TM_TLID_T));
    memset(tm->tlid_by_tupid, 0, tm->max_tupid * sizeof(TM_TLID_T));
    tm->btypid_by_tupid = (TM_TLID_T *) mm->malloc(tm->max_tupid * sizeof(btypeid_t));
    memset(tm->summary_by_btypeid, 0, tm->max_tupid * sizeof(btypeid_t));
    tm->tupid_by_tlidhash = ht_create(TM_XXXID_BY_TLIDHASH);
    tm->tupid_by_tlidhash->tlid_by_xxxid = tm->tlid_by_tupid;

    // structs
    tm->max_strid = TM_MAX_ID_INC_SIZE;
    tm->next_strid = 1;

    // records
    tm->max_recid = TM_MAX_ID_INC_SIZE;
    tm->next_recid = 1;

    // sequences
    tm->max_seqid = TM_MAX_ID_INC_SIZE;
    tm->next_seqid = 1;

    // maps
    tm->max_mapid = TM_MAX_ID_INC_SIZE;
    tm->next_mapid = 1;

    // functions
    tm->max_fncid = TM_MAX_ID_INC_SIZE;
    tm->next_fncid = 1;

    // schema variables
    tm->max_svrid = TM_MAX_ID_INC_SIZE;
    tm->next_svrid = 1;

    return tm;
}

pub int TM_trash(BK_TM *tm) {
    // typelists
    tm->mm->free(tm->rp_by_tlid);
    ht_trash(TM_TLID_BY_TLHASH, tm->tlid_by_tlhash);

    // type names
    ht_trash(TM_BTYPEID_BY_SYMIDHASH, tm->btypeid_by_symidhash);
    tm->mm->free(tm->symid_by_btypeid);

    // type summaries
    tm->mm->free(tm->summary_by_btypeid);

    // intersections
    tm->mm->free(tm->tlid_by_intid);
    tm->mm->free(tm->btypid_by_intid);
    ht_trash(TM_XXXID_BY_TLIDHASH, tm->intid_by_tlidhash);

    // unions
    tm->mm->free(tm->tlid_by_uniid);
    tm->mm->free(tm->btypid_by_uniid);
    ht_trash(TM_XXXID_BY_TLIDHASH, tm->uniid_by_tlidhash);

    // tuples
    tm->mm->free(tm->tlid_by_tupid);
    tm->mm->free(tm->btypid_by_tupid);
    ht_trash(TM_XXXID_BY_TLIDHASH, tm->tupid_by_tlidhash);

    // structs

    // records

    // sequences

    // maps

    // functions

    // schema variables

    // self
    tm->mm->free(tm);
    return 0;
}


#endif  // __BK_TM_C
