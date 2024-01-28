// ---------------------------------------------------------------------------------------------------------------------
//                                                    Type Manager
// ---------------------------------------------------------------------------------------------------------------------

#ifndef __BK_TM_C
#define __BK_TM_C "bk/tm.c"


#include "../../include/bk/mm.h"
#include "../../include/bk/tm.h"
#include "../../include/bk/tp.h"
#include "lib/ht_impl.h"
#include "lib/radix.h"


KRADIX_SORT_INIT(btypeid_t, btypeid_t, ,sizeof(btypeid_t))


// ---------------------------------------------------------------------------------------------------------------------
// TM_BTYPEID_BY_SYMIDHASH fns
// ---------------------------------------------------------------------------------------------------------------------

pvt inline symid_t symidFromBtypeid(ht_struct(TM_BTYPEID_BY_SYMIDHASH) *ht, symid_t slot) {
    return ht->tm->symid_by_btypeid[slot];
}

pvt bool inline symidHashableFound(ht_struct(TM_BTYPEID_BY_SYMIDHASH) *ht, btypeid_t entry, symid_t hashable) {
    return ht->tm->symid_by_btypeid[entry] == hashable;
}

// HT_IMPL(name, entry_t, hashable_t, __hash_fn, __found_fn, __hashable_from_slot_fn)
HT_IMPL(TM_BTYPEID_BY_SYMIDHASH, btypeid_t, symid_t, ht_int32_hash, symidHashableFound, symidFromBtypeid)


// ---------------------------------------------------------------------------------------------------------------------
// TM_TLID_BY_TLHASH fns
// ---------------------------------------------------------------------------------------------------------------------

pvt inline btypeid_t * tlFromTlid(ht_struct(TM_TLID_BY_TLHASH) *ht, TM_TLID_T tlid) {
    return ht->tm->typelist_buf + ht->tm->tlrp_by_tlid[tlid];
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

pvt bool inline tlFound(ht_struct(TM_TLID_BY_TLHASH) *ht, TM_TLID_T value, btypeid_t *key) {
    return tlCompare(tlFromTlid(ht, value), key);
}

// HT_IMPL(name, entry_t, hashable_t, __hash_fn, __found_fn, __hashable_from_slot_fn)
HT_IMPL(TM_TLID_BY_TLHASH, TM_TLID_T, btypeid_t *, tl_hash, tlFound, tlFromTlid)


// ---------------------------------------------------------------------------------------------------------------------
// TM_XXXID_BY_TLIDHASH fns
// ---------------------------------------------------------------------------------------------------------------------

pvt inline TM_TLID_T tlidFromSlot(ht_struct(TM_XXXID_BY_TLIDHASH) *ht, TM_XXXID_T slot) {
    return ht->tlid_by_xxxid[slot];
}

pvt bool inline tlidFound(ht_struct(TM_XXXID_BY_TLIDHASH) *ht, TM_XXXID_T slot, TM_TLID_T key) {
    return ht->tlid_by_xxxid[slot] == key;
}

// HT_IMPL(name, entry_t, hashable_t, __hash_fn, __found_fn, __hashable_from_slot_fn)
HT_IMPL(TM_XXXID_BY_TLIDHASH, u32, u32, ht_int32_hash, tlidFound, tlidFromSlot)



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
    ht_replace_empty(TM_TLID_BY_TLHASH, tm->tlid_by_tlhash, idx, tlid);
    if (tm->next_tlrp + numTypes + 1 >= tm->max_tlrp) {
        size_t pageSize = os_page_size();
        os_mprotect(tm->typelist_buf + tm->max_tlrp - pageSize, pageSize, BK_M_READ);     // make the prior last page read only
        tm->max_tlrp += pageSize / sizeof(TM_TLID_T);
    }
    tm->next_tlrp += numTypes + 1;
    return tlid;
}

tdd void _new_type_summary_at(BK_TM *tm, bmetatypeid_t bmtid, btexclusioncat_t excl, u32 idx, symid_t symid, btypeid_t btypeid, u32 _id) {
    // OPEN: add size
    // OPEN: do we restrict the range of directly assigned btypeids?
    while (btypeid >= tm->max_btypeId) {
        tm->max_btypeId += TM_MAX_BTYPEID_INC_SIZE;
        _growTo((void **)&tm->summary_by_btypeid, tm->max_btypeId * sizeof(struct btsummary), tm->mm, FN_NAME);
        _growTo((void **)&tm->symid_by_btypeid, tm->max_btypeId * sizeof(symid_t), tm->mm, FN_NAME);
    }
    tm->symid_by_btypeid[btypeid] = symid;
    tm->summary_by_btypeid[btypeid].bmtid = bmtid;
    tm->summary_by_btypeid[btypeid].excl = excl;
    tm->summary_by_btypeid[btypeid]._id = _id;
    ht_replace_empty(TM_BTYPEID_BY_SYMIDHASH, tm->btypeid_by_symidhash, idx, btypeid);
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

pub btypeid_t tm_btypeid(BK_TM *tm, char *name) {
    int outcome;  u32 idx;
    idx = ht_put_idx(TM_BTYPEID_BY_SYMIDHASH, tm->btypeid_by_symidhash, sm_id(tm->sm, name), &outcome);
    if (outcome == HT_LIVE)
        return tm->btypeid_by_symidhash->entries[idx];
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
        idx = ht_put_idx(TM_BTYPEID_BY_SYMIDHASH, tm->btypeid_by_symidhash, symid, &outcome);
        if (outcome == HT_LIVE) {
            // already exists so check it's a nominal with the same exclusion
            btypeid = ht_value(tm->btypeid_by_symidhash, idx);
            struct btsummary s = tm->summary_by_btypeid[btypeid];
            if (s.bmtid != bmtnom || s.excl != excl) return B_NAT;          // OPEN: check size
            return btypeid;
        } else {
            if (btypeid == 0) btypeid = tm->next_btypeId;
            _new_type_summary_at(tm, bmtnom, excl, idx, symid, btypeid, 0);
            return btypeid;
        }
    }
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
    TM_XXXID_T intid;  struct btsummary *sum;
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
    u32 idx = ht_put_idx(TM_TLID_BY_TLHASH, tm->tlid_by_tlhash, nextTypelist, &outcome);
    switch (outcome) {
        default:
            die("%s: HT_TOMBSTONE1!", FN_NAME);
        case HT_LIVE:
            tlid = tm->tlid_by_tlhash->entries[idx];
            break;
        case HT_EMPTY:
            tlid = _commit_typelist_buf_at(tm, numTypes, idx);
            if (!tlid) return 0;       // an error occurred OPEN handle properly
    }

    // get the btypeid for the tlid
    idx = ht_put_idx(TM_XXXID_BY_TLIDHASH, tm->intid_by_tlidhash, tlid, &outcome);
    switch (outcome) {
        default:
            die("%s: HT_TOMBSTONE2!", FN_NAME);
        case HT_LIVE:
            // typelist already exists
            intid = tm->intid_by_tlidhash->entries[idx];
            if (btypeid == 0) return tm->btypid_by_intid[intid];
            else if (btypeid == tm->btypid_by_intid[intid]) return btypeid;
            else return 0;
        case HT_EMPTY:
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
            _new_type_summary_at(tm, bmtint, excl, idx, 0, btypeid, intid);
            tm->btypid_by_intid[intid] = btypeid;
            ht_replace_empty(TM_XXXID_BY_TLIDHASH, tm->intid_by_tlidhash, idx, intid);
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
            idx = ht_put_idx(TM_BTYPEID_BY_SYMIDHASH, tm->btypeid_by_symidhash, symid, &outcome);
            if (outcome == HT_LIVE)
                return B_NAT;
            else {
                ht_replace_empty(TM_BTYPEID_BY_SYMIDHASH, tm->btypeid_by_symidhash, idx, btypeid);
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
        idx = ht_put_idx(TM_BTYPEID_BY_SYMIDHASH, tm->btypeid_by_symidhash, symid, &outcome);
        if (outcome == HT_LIVE) {
            // already exists so check it's a nominal with no exclusion
            btypeid = ht_value(tm->btypeid_by_symidhash, idx);
            sum = tm->summary_by_btypeid[btypeid];
            if (sum.bmtid != bmtnom || sum.excl != btenone) return B_NAT;
            return btypeid;
        } else {
            if (btypeid == 0) btypeid = tm->next_btypeId;
            _new_type_summary_at(tm, bmtnom, btenone, idx, symid, btypeid, 0);
            return btypeid;
        }
    }
}

pub size tm_size(BK_TM * tm, btypeid_t btypeid) {
    // OPEN: implement
    return 8;
}

pub btypeid_t tm_tuple(BK_TM *tm, btypeid_t *typelist, btypeid_t btypeid) {
    i32 i, outcome, numTypes;  struct btsummary *sum;  TM_XXXID_T tupid;  TM_TLID_T tlid;  btypeid_t *p1, *nextTypelist;

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
    u32 idx = ht_put_idx(TM_TLID_BY_TLHASH, tm->tlid_by_tlhash, nextTypelist, &outcome);
    switch (outcome) {
        default:
            die("%s:%i: HT_TOMBSTONE1!", FN_NAME, __LINE__);
        case HT_LIVE:
            tlid = tm->tlid_by_tlhash->entries[idx];
            break;
        case HT_EMPTY:
            tlid = _commit_typelist_buf_at(tm, numTypes, idx);
            if (!tlid) return 0;       // an error occurred OPEN handle properly
    }

    // get the btypeid for the tlid
    idx = ht_put_idx(TM_XXXID_BY_TLIDHASH, tm->tupid_by_tlidhash, tlid, &outcome);
    switch (outcome) {
        default:
            die("%s:%i: HT_TOMBSTONE2!", FN_NAME, __LINE__);
        case HT_LIVE:
            tupid = tm->tupid_by_tlidhash->entries[idx];
            if (btypeid == 0) return tm->btypid_by_tupid[tupid];
            else if (btypeid == tm->btypid_by_tupid[tupid]) return btypeid;
            else return 0;
        case HT_EMPTY:
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
            _new_type_summary_at(tm, bmttup, btenone, idx, 0, btypeid, tupid);
            tm->btypid_by_tupid[tupid] = btypeid;
            ht_replace_empty(TM_XXXID_BY_TLIDHASH, tm->tupid_by_tlidhash, idx, tupid);
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
    btypeid_t *uniTl, *p1, *p2, *p3, *nextTypelist;
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
    u32 idx = ht_put_idx(TM_TLID_BY_TLHASH, tm->tlid_by_tlhash, nextTypelist, &outcome);
    switch (outcome) {
        default:
            die("%s: HT_TOMBSTONE1!", FN_NAME);
        case HT_LIVE:
            tlid = tm->tlid_by_tlhash->entries[idx];
            break;
        case HT_EMPTY:
            tlid = _commit_typelist_buf_at(tm, numTypes, idx);
            if (!tlid) return 0;       // an error occurred OPEN handle properly
    }

    // get the btypeid for the tlid
    idx = ht_put_idx(TM_XXXID_BY_TLIDHASH, tm->uniid_by_tlidhash, tlid, &outcome);
    switch (outcome) {
        default:
            die("%s: HT_TOMBSTONE2!", FN_NAME);
        case HT_LIVE:
            uniid = tm->uniid_by_tlidhash->entries[idx];
            if (btypeid == 0) return tm->btypid_by_uniid[uniid];
            else if (btypeid == tm->btypid_by_uniid[uniid]) return btypeid;
            else return 0;
        case HT_EMPTY:
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
            _new_type_summary_at(tm, bmtuni, btenone, idx, 0, btypeid, uniid);
            tm->btypid_by_uniid[uniid] = btypeid;
            ht_replace_empty(TM_XXXID_BY_TLIDHASH, tm->uniid_by_tlidhash, idx, uniid);
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
    memset(tm->btypid_by_intid, 0, tm->max_intid * sizeof(btypeid_t));
    tm->intid_by_tlidhash = ht_create(TM_XXXID_BY_TLIDHASH);
    tm->intid_by_tlidhash->tlid_by_xxxid = tm->tlid_by_intid;

    // unions
    tm->max_uniid = TM_MAX_ID_INC_SIZE;
    tm->next_uniid = 1;
    tm->tlid_by_uniid = (TM_TLID_T *) mm->malloc(tm->max_uniid * sizeof(TM_TLID_T));
    memset(tm->tlid_by_uniid, 0, tm->max_uniid * sizeof(TM_TLID_T));
    tm->btypid_by_uniid = (TM_TLID_T *) mm->malloc(tm->max_uniid * sizeof(btypeid_t));
    memset(tm->btypid_by_uniid, 0, tm->max_uniid * sizeof(btypeid_t));
    tm->uniid_by_tlidhash = ht_create(TM_XXXID_BY_TLIDHASH);
    tm->uniid_by_tlidhash->tlid_by_xxxid = tm->tlid_by_uniid;

    // tuples
    tm->max_tupid = TM_MAX_ID_INC_SIZE;
    tm->next_tupid = 1;
    tm->tlid_by_tupid = (TM_TLID_T *) mm->malloc(tm->max_tupid * sizeof(TM_TLID_T));
    memset(tm->tlid_by_tupid, 0, tm->max_tupid * sizeof(TM_TLID_T));
    tm->btypid_by_tupid = (TM_TLID_T *) mm->malloc(tm->max_tupid * sizeof(btypeid_t));
    memset(tm->btypid_by_tupid, 0, tm->max_tupid * sizeof(btypeid_t));
    tm->tupid_by_tlidhash = ht_create(TM_XXXID_BY_TLIDHASH);
    tm->tupid_by_tlidhash->tlid_by_xxxid = tm->tlid_by_tupid;

    // structs
    tm->max_strid = TM_MAX_ID_INC_SIZE;
    tm->next_strid = 1;

    // records
//    tm->max_recid = TM_MAX_ID_INC_SIZE;
//    tm->next_recid = 1;

    // sequences
    tm->max_seqid = TM_MAX_ID_INC_SIZE;
    tm->next_seqid = 1;
//    tm->seqid_by_btypeidhash = ht_create(TM_SEQID_BY_BTYPEIDHASH);
//    tm->btypeid_by_seqid = (TM_TLID_T *) mm->malloc(tm->max_seqid * sizeof(TM_TLID_T));
//    memset(tm->btypeid_by_seqid, 0, tm->max_seqid * sizeof(TM_TLID_T));


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
    tm->mm->free(tm->tlrp_by_tlid);
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
