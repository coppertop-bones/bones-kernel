#ifndef __BK_TM_C
#define __BK_TM_C "bk/tm.c"


#include "../../include/all.cfg"
#include "buckets.c"
#include "../../include/bk/mm.h"
#include "../../include/bk/tm.h"
#include "../lib/ht_impl.c"
#include "../lib/radix.c"


KRADIX_SORT_INIT(BTYPEID_T, BTYPEID_T, ,sizeof(BTYPEID_T))


// ---------------------------------------------------------------------------------------------------------------------
// symid_by_btypeid hash fns
// ---------------------------------------------------------------------------------------------------------------------

pvt inline SYM_ID_T symIdFromEntry(ht_struct(TM_BTYPEID_BY_SYMIDHASH) *ht, SYM_ID_T entry) {
    return ht->tm->symid_by_btypeid[entry];
}

pvt bool inline found(ht_struct(TM_BTYPEID_BY_SYMIDHASH) *ht, SYM_ID_T entry, BTYPEID_T key) {
    return ht->tm->symid_by_btypeid[entry] == key;
}

// HT_IMPL(name, slot_t, key_t, __hash_fn, __found_fn, __key_from_entry_fn)
HT_IMPL(TM_BTYPEID_BY_SYMIDHASH, BTYPEID_T, SYM_ID_T, ht_int32_hash, found, symIdFromEntry)


// ---------------------------------------------------------------------------------------------------------------------
// rp_by_tlid hash fns
// ---------------------------------------------------------------------------------------------------------------------

pvt inline BTYPEID_T * tlFromEntry(ht_struct(TM_TLID_BY_TLHASH) *ht, TM_TLID_T entry) {
    return ht->tm->typelist_buf + ht->tm->rp_by_tlid[entry];
}

pvt inline bool tlCompare(BTYPEID_T *a, BTYPEID_T *b) {
    BTYPEID_T size;
    if ((size=a[0]) != b[0]) return 0;
    for (BTYPEID_T i=1; i<=size; i++) if (a[i] != b[i]) return 0;     // beware <= :)
    return 1;
}

pvt u32 tl_hash(BTYPEID_T *tl) {
    u32 n = tl[0] * sizeof(BTYPEID_T);
    u8 *s = (unsigned char *) tl;
    u8 *e = s + n;
    u32 hash = *s++;
    for (; s < e; s++) if (*s) hash = (hash << 5) - hash + *s;  // OPEN: explain why ignoring zeros
    return hash;
}

pvt bool inline tlFound(ht_struct(TM_TLID_BY_TLHASH) *ht, TM_TLID_T entry, BTYPEID_T *key) {
    return tlCompare(tlFromEntry(ht, entry), key);
}

// HT_IMPL(name, slot_t, key_t, __hash_fn, __found_fn, __key_from_entry_fn)
HT_IMPL(TM_TLID_BY_TLHASH, TM_TLID_T, BTYPEID_T *, tl_hash, tlFound, tlFromEntry)


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

pvt void _growTo(void **p, size_t size, struct MM *mm, char *fnName) {
    void *t = *p;
    t = mm->realloc(t, size);
    onOomDie(t, "%s: realloc #1 failed", fnName);
    *p = t;
}

tdd TM_TLID_T _commit_typelist_buf_at(struct TM *tm, TM_TLID_T numTypes, u32 idx) {
    TM_TLID_T tlid;
    if ((tlid = tm->next_tlid++) >= tm->max_tlid) {
        tm->max_tlid += TM_RP_BY_TLID_INC_SIZE;
        _growTo((void **)&tm->rp_by_tlid, tm->max_tlid * sizeof(RP), tm->mm, __FUNCTION__);
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

tdd BTYPEID_T _new_type_summary_at(struct TM *tm, BMETATYPE_ID_T bmtid, enum btexclusioncat excl, u32 idx, SYM_ID_T symid, BTYPEID_T id) {
    BTYPEID_T btypeid = tm->next_btypeId++;
    if (btypeid >= tm->max_btypeId) {
        tm->max_btypeId += TM_MAX_BTYPEID_INC_SIZE;
        _growTo((void **)&tm->summary_by_btypeid, tm->max_btypeId * sizeof(struct btsummary), tm->mm, __FUNCTION__);
        _growTo((void **)&tm->symid_by_btypeid, tm->max_intid * sizeof(SYM_ID_T), tm->mm, __FUNCTION__);
    }
    tm->symid_by_btypeid[btypeid] = symid;
    tm->summary_by_btypeid[btypeid].bmtid = bmtid;
    tm->summary_by_btypeid[btypeid].excl = excl;
    tm->summary_by_btypeid[btypeid].intId = id;
    ht_replace_empty(TM_BTYPEID_BY_SYMIDHASH, tm->btypeid_by_symidhash, idx, btypeid);
    return btypeid;
}



// ---------------------------------------------------------------------------------------------------------------------
// type accessing / creation fns
// ---------------------------------------------------------------------------------------------------------------------

pub BTYPEID_T tm_btypeid(struct TM *tm, char *name) {
    int res;  u32 idx;
    idx = ht_put_idx(TM_BTYPEID_BY_SYMIDHASH, tm->btypeid_by_symidhash, sm_id(tm->sm, name), &res);
    if (res == HT_EXISTS)
        return tm->btypeid_by_symidhash->slots[idx];
    else
        return 0;
}

pub BTYPEID_T tm_exclnominal(struct TM *tm, char *name, enum btexclusioncat excl) {
    int res;  BTYPEID_T btypeid;  SYM_ID_T symid;  u32 idx;
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


void tm_pp(struct TM *tm, BTYPEID_T btypeid) {
    struct btsummary *sum;  SYM_ID_T symid;  BTYPEID_T *tl;  int i;  char sep;
    sum = tm->summary_by_btypeid + btypeid;
    switch (sum->bmtid) {
        case bmtnom:
            if (symid = tm->symid_by_btypeid[btypeid]) {
                fprintf(stderr, "%s", sm_name(tm->sm, symid));
            }
            else {
                fprintf(stderr, "t%i", btypeid);
            }
            break;
        case bmtint:
            tl = tm->typelist_buf + tm->rp_by_tlid[tm->tlid_by_intid[sum->intId]];
            sep = 0;
            for (i = 1; i <= tl[0]; i++) {
                if (sep) fprintf(stderr, " & ");
                sep = 1;
                tm_pp(tm, tl[i]);
            }
            break;
        default:
            printf("???");
    }
}


// set of values intersection ((1 2 3) + (4 5)) & ((1 2 3) + (6 7)) = (1 2 3 4 5) & (1 2 3 6 7) = (1 2 3)
// (int + str) & (int + bool) => (int+int) & (int+bool) & (str+int) & (str+bool)
// types only make sense in the context of fitsWithin a LHS might not behaviour as a RHS


pub BTYPEID_T tm_inter(struct TM *tm, BTYPEID_T *typelist) {
    int i, j, res, numTypes, hasUnions;  enum btexclusioncat excl = 0;  TM_TLID_T tlid;
    BTYPEID_T btypeid, *interTl, *p1, *p2, *p3, *nextTypelist;
    TM_XXXID_T intid;  struct btsummary *sum;
    // (A&B) & (C&D)  = A & B & C & D
    // (A&B) & (B&C)  = A & B & C
    // (A+B) & (B+C)  = (A+B) & (B+C)  why not B? because we need to keep the detail when the program is causes intersections
    //
    // (A&B) + (B&C)  = B & (A + C)    - not the same for unions of intersections

    // use tm->typelist_buf as scratch so don't have to allocate memory
    // OPEN: potentially though messy we could do intersections without child intersections in place in typelist to keep a little cache locality

    if (!(numTypes = typelist[0])) return 0;

    // check typeid is in range, and figure total length (including possible duplicate from child intersections)
    for (i = 1; i <= typelist[0]; i++) {
        if (!(0 < typelist[i] && typelist[i] < tm->next_btypeId)) return 0;
        sum = tm->summary_by_btypeid + typelist[i];
        if (sum->bmtid == bmtint) {
            tlid = tm->tlid_by_intid[sum->intId];
            numTypes += (tm->typelist_buf + tm->rp_by_tlid[tlid])[0] - 1;
        }
    }

    // grow tm->typelist_buf if necessary
    if (tm->next_rp + numTypes >= tm->max_rp) {
        if (tm->next_rp + numTypes >= TM_MAX_TL_STORAGE) die("%s: out of typelist storage", __FUNCTION__);  // OPEN: really we should add an error reporting mechanism, e.g. TM_ERR_OUT_OF_NAME_STORAGE, etc
        size_t pageSize = os_page_size();
        os_mprotect(tm->typelist_buf + tm->max_rp, pageSize, BK_M_READ | BK_M_WRITE);
        os_madvise(tm->typelist_buf + tm->max_rp, pageSize, BK_AD_RANDOM);
    }

    nextTypelist = tm->typelist_buf + tm->next_rp;

    // copy typelist into typelist_buf unpacking any intersections
    p1 = nextTypelist;
    *p1++ = numTypes;
    for (i = 1; i <= typelist[0]; i++) {
        sum = tm->summary_by_btypeid + typelist[i];
        if (sum->bmtid == bmtint) {
            // we have an intersection type - expand it
            tlid = tm->tlid_by_intid[sum->intId];
            interTl = (tm->typelist_buf + tm->rp_by_tlid[tlid]);
            for (j = 1; j <= interTl[0]; j++) *p1++ = interTl[j];
        }
        else
            *p1++ = typelist[i];
    }

    // sort types into btypeid order
    ks_radix_sort(BTYPEID_T, nextTypelist + 1, numTypes);

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
            die("%s: HT_TOMBSTONE1!", __FUNCTION__);
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
            die("%s: HT_TOMBSTONE2!", __FUNCTION__);
        case HT_EXISTS:
            intid = tm->intid_by_tlidhash->slots[idx];
            return tm->btypid_by_intid[intid];
        case HT_EMPTY:
            // missing so commit the intersection type for tlid
            intid = tm->next_intid++;
            if (intid >= tm->max_intid) {
                tm->max_intid += TM_MAX_ID_INC_SIZE;
                _growTo((void **)&tm->tlid_by_intid, tm->max_intid * sizeof(TM_TLID_T), tm->mm, __FUNCTION__);
                _growTo((void **)&tm->btypid_by_intid, tm->max_intid * sizeof(BTYPEID_T), tm->mm, __FUNCTION__);
            }
            tm->tlid_by_intid[intid] = tlid;
            btypeid = _new_type_summary_at(tm, bmtint, excl, idx, 0, intid);
            tm->btypid_by_intid[intid] = btypeid;

            ht_replace_empty(TM_XXXID_BY_TLIDHASH, tm->intid_by_tlidhash, idx, intid);

            return btypeid;
    }
}

pub char * tm_name(struct TM *tm, BTYPEID_T btypeid) {
    // OPEN: do we return a null or "" if the type is unnamed? in Python we might want to return f't{btypeid}'
    SYM_ID_T symid;
    if (btypeid >= tm->next_btypeId) return "";
    symid = tm->symid_by_btypeid[btypeid];
    if (symid)
        return sm_name(tm->sm, symid);
    else
        return 0;
}

pub BTYPEID_T tm_name_as(struct TM *tm, BTYPEID_T btypeid, char *name) {
    int res;  BTYPEID_T existingId;  SYM_ID_T symid;  u32 idx;
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

pub BTYPEID_T tm_nominal(struct TM *tm, char *name) {
    int res;  BTYPEID_T btypeid;  SYM_ID_T symid;  u32 idx;
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

tdd int tm_setNominalTo(struct TM *tm, char *name, BTYPEID_T btypeid) {
     SYM_ID_T symid;  int res;  u32 idx;
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



// ---------------------------------------------------------------------------------------------------------------------
// type manager lifecycle fns
// ---------------------------------------------------------------------------------------------------------------------

pub struct TM * TM_create(struct MM *mm, struct SM *sm) {
    // OPEN: should we use calloc instead of memset to init arrays to zero?
    struct TM *tm = (struct TM *) mm->malloc(sizeof(struct TM));
    tm->mm = mm;
    tm->sm = sm;
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
    tm->symid_by_btypeid = (BTYPEID_T *) mm->malloc(tm->max_btypeId * sizeof(BTYPEID_T));
    memset(tm->symid_by_btypeid, 0, tm->max_btypeId * sizeof(BTYPEID_T));

    // type summaries
    tm->summary_by_btypeid = (struct btsummary *) mm->malloc(tm->max_btypeId * sizeof(struct btsummary));
    memset(tm->summary_by_btypeid, 0, tm->max_btypeId * sizeof(struct btsummary));

    // intersections
    tm->max_intid = TM_MAX_ID_INC_SIZE;
    tm->next_intid = 1;
    tm->tlid_by_intid = (TM_TLID_T *) mm->malloc(tm->max_intid * sizeof(TM_TLID_T));
    memset(tm->tlid_by_intid, 0, tm->max_intid * sizeof(TM_TLID_T));
    tm->btypid_by_intid = (TM_TLID_T *) mm->malloc(tm->max_intid * sizeof(BTYPEID_T));
    memset(tm->summary_by_btypeid, 0, tm->max_intid * sizeof(BTYPEID_T));
    tm->intid_by_tlidhash = ht_create(TM_XXXID_BY_TLIDHASH);
    tm->intid_by_tlidhash->tlid_by_xxxid = tm->tlid_by_intid;

    // unions
    tm->max_uniid = TM_MAX_ID_INC_SIZE;
    tm->next_uniid = 1;

    // tuples
    tm->max_tupid = TM_MAX_ID_INC_SIZE;
    tm->next_tupid = 1;

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

pub int TM_trash(struct TM *tm) {
    ht_trash(TM_BTYPEID_BY_SYMIDHASH, tm->btypeid_by_symidhash);
    tm->mm->free(tm->symid_by_btypeid);
    tm->mm->free(tm);
    return 0;
}


#endif  // __BK_TM_C
