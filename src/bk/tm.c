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

pvt inline SYM_ID_T symIdFromEntry(ht_struct(TM_BTYPEID_BY_SYMIDHASH) const * ht, SYM_ID_T entry) {
    return ht->tm->symid_by_btypeid[entry];
}

pvt bool inline found(ht_struct(TM_BTYPEID_BY_SYMIDHASH) const * ht, SYM_ID_T entry, BTYPEID_T key) {
    return ht->tm->symid_by_btypeid[entry] == key;
}

// HT_IMPL(name, slot_t, key_t, __hash_fn, __found_fn, __key_from_entry_fn)
HT_IMPL(TM_BTYPEID_BY_SYMIDHASH, BTYPEID_T, SYM_ID_T, ht_int32_hash, found, symIdFromEntry)


// ---------------------------------------------------------------------------------------------------------------------
// rp_by_tlid hash fns
// ---------------------------------------------------------------------------------------------------------------------

pvt inline BTYPEID_T const * tlFromEntry(ht_struct(TM_TLID_BY_TLHASH) const * ht, TM_TLID_T entry) {
    return ht->tm->typelist_buf + ht->tm->rp_by_tlid[entry];
}

pvt inline bool tlCompare(BTYPEID_T const *a, BTYPEID_T const *b) {
    BTYPEID_T size;
    if ((size=a[0]) != b[0]) return 0;
    for (BTYPEID_T i=1; i<=size; i++) if (a[i] != b[i]) return 0;     // beware <= :)
    return 1;
}

pvt u32 tl_hash(BTYPEID_T const *tl) {
    u32 n = tl[0] * sizeof(BTYPEID_T);
    u8 *s = (unsigned char *) tl;
    u8 *e = s + n;
    u32 hash = *s++;
    for (; s < e; s++) if (*s) hash = (hash << 5) - hash + *s;  // OPEN: explain why ignoring zeros
    return hash;
}

pvt bool inline tlFound(ht_struct(TM_TLID_BY_TLHASH) const *ht, TM_TLID_T entry, BTYPEID_T const *key) {
    return tlCompare(tlFromEntry(ht, entry), key);
}

// HT_IMPL(name, slot_t, key_t, __hash_fn, __found_fn, __key_from_entry_fn)
HT_IMPL(TM_TLID_BY_TLHASH, TM_TLID_T, BTYPEID_T const *, tl_hash, tlFound, tlFromEntry)


// ---------------------------------------------------------------------------------------------------------------------
// TM_XXXID_BY_TLIDHASH fns
// ---------------------------------------------------------------------------------------------------------------------

pvt inline TM_TLID_T tlidFromEntry(ht_struct(TM_XXXID_BY_TLIDHASH) const * ht, TM_XXXID_T entry) {
    return ht->tlid_by_xxxid[entry];
}

pvt bool inline tlidFound(ht_struct(TM_XXXID_BY_TLIDHASH) const * ht, TM_XXXID_T entry, TM_TLID_T key) {
    return ht->tlid_by_xxxid[entry] == key;
}

// HT_IMPL(name, slot_t, key_t, __hash_fn, __found_fn, __key_from_entry_fn)
HT_IMPL(TM_XXXID_BY_TLIDHASH, u32, u32, ht_int32_hash, tlidFound, tlidFromEntry)



// ---------------------------------------------------------------------------------------------------------------------
// utils
// ---------------------------------------------------------------------------------------------------------------------

pvt void _growTo(void **p, size_t size, struct MM *mm, char const *fnName) {
    void *t = *p;
    t = mm->realloc(t, size);
    onOomDie(t, "%s: realloc #1 failed", fnName);
    *p = t;
}

tdd TM_TLID_T tm_add_typelist_at(struct TM *tm, BTYPEID_T const *typelist, u32 idx) {
    bool needsAnotherPage;  int length, pageSize, i;  TM_TLID_T tlid;  RP rp;
    length = typelist[0] + 1;
    rp = tm->next_rp;

    if ((needsAnotherPage = rp + length >= tm->max_rp)) {
        if (rp + length >= TM_MAX_TL_STORAGE) die("%s: out of typelist storage", __FUNCTION__);  // OPEN: really we should add an error reporting mechanism, e.g. TM_ERR_OUT_OF_NAME_STORAGE, etc
        // make next page r/w and mark as random access
        pageSize = os_page_size();
        os_mprotect(tm->typelist_buf + tm->max_rp, pageSize, BK_M_READ | BK_M_WRITE);
        os_madvise(tm->typelist_buf + tm->max_rp, pageSize, BK_AD_RANDOM);
    }

    if ((tlid = tm->next_tlid++) >= tm->max_tlid) {
        tm->max_tlid += TM_RP_BY_TLID_INC_SIZE;
        _growTo((void **)&tm->rp_by_tlid, tm->max_tlid * sizeof(RP), tm->mm, __FUNCTION__);
        tm->intid_by_tlidhash->tlid_by_xxxid = tm->tlid_by_intid;
    }

    BTYPEID_T *s = tm->typelist_buf + rp;
    for (i = 0; i < length; i++) s[i] = typelist[i];

    tm->rp_by_tlid[tlid] = rp;
    ht_replace_empty(TM_TLID_BY_TLHASH, tm->tlid_by_tlhash, idx, tlid);

    tm->next_rp += length;
    if (needsAnotherPage) {
        os_mprotect(tm->typelist_buf + tm->next_rp - pageSize, pageSize, BK_M_READ);     // make the prior last page read only
        tm->max_rp += pageSize;
    }
    return tlid;
}

tdd BTYPEID_T _new_type_summary_at(struct TM *tm, BMETATYPE_ID_T bmtid, enum btexclusioncat excl, u32 idx, SYM_ID_T symid) {
    BTYPEID_T btypeid = tm->next_btypeId++;
    if (btypeid >= tm->max_btypeId) {
        tm->max_btypeId += TM_MAX_BTYPEID_INC_SIZE;
        _growTo((void **)&tm->summary_by_btypeid, tm->max_btypeId * sizeof(struct btsummary), tm->mm, __FUNCTION__);
        _growTo((void **)&tm->symid_by_btypeid, tm->max_intid * sizeof(SYM_ID_T), tm->mm, __FUNCTION__);
    }
    tm->symid_by_btypeid[btypeid] = symid;
    tm->summary_by_btypeid[btypeid].bmtid = bmtid;
    tm->summary_by_btypeid[btypeid].excl = excl;
    ht_replace_empty(TM_BTYPEID_BY_SYMIDHASH, tm->btypeid_by_symidhash, idx, btypeid);
    return btypeid;
}



// ---------------------------------------------------------------------------------------------------------------------
// type accessing / creation fns
// ---------------------------------------------------------------------------------------------------------------------

pub BTYPEID_T tm_btypeid(struct TM *tm, char const * name) {
    int res;  u32 idx;
    idx = ht_put_idx(TM_BTYPEID_BY_SYMIDHASH, tm->btypeid_by_symidhash, sm_id(tm->sm, name), &res);
    if (res == HT_EXISTS)
        return tm->btypeid_by_symidhash->slots[idx];
    else
        return 0;
}

pub BTYPEID_T tm_exclnominal(struct TM *tm, char const * name, enum btexclusioncat excl) {
    int res;  BTYPEID_T btypeid;  SYM_ID_T symid;  u32 idx;
    symid = sm_id(tm->sm, name);                                        // get symid of name
    idx = ht_put_idx(TM_BTYPEID_BY_SYMIDHASH, tm->btypeid_by_symidhash, symid, &res);      // get index of the btypeid corresponding to symid
    if (res == HT_EXISTS) {
        btypeid = ht_entry(tm->btypeid_by_symidhash, idx);
        struct btsummary summary = tm->summary_by_btypeid[btypeid];
        if (summary.excl != excl || summary.bmtid != btnom) return 0;      // check it's a nominal with the same exclusion
        return btypeid;
    }
    else {
        // create a new nominal in an exclusive category
        return _new_type_summary_at(tm, btnom, excl, idx, symid);
    }
}

tdd bool _tm_is_valid_intersection(struct TM *tm, BTYPEID_T *typelist) {
    // verify that the typelist is valid (no mutual exclusions)
    // OPEN: IMPORTANT STUFF TO DO HERE
    return true;
}

pub BTYPEID_T tm_inter(struct TM *tm, BTYPEID_T *typelist) {
    int res, l;  TM_TLID_T tlid;  BTYPEID_T btypeid;  TM_XXXID_T intid;
    if (!(l = typelist[0])) return 0;
    for (int i=0; i<l; i++) if (typelist[i] == 0 || typelist[i] > tm->next_btypeId) return 0;
    ks_radix_sort(BTYPEID_T, typelist + 1, typelist[0]);   // sort types into btypeid order, type list is length prefixed

    // get the tlid for the typelist - adding if missing, returning 0 if invalid
    u32 idx = ht_put_idx(TM_TLID_BY_TLHASH, tm->tlid_by_tlhash, typelist, &res);
    switch (res) {
        default:
            die("%s: HT_TOMBSTONE1!", __FUNCTION__);
        case HT_EXISTS:
            tlid = tm->tlid_by_tlhash->slots[idx];
            break;
        case HT_EMPTY:
            if (!_tm_is_valid_intersection(tm, typelist)) return 0;
            tlid = tm_add_typelist_at(tm, typelist, idx);
            if (!tlid) return 0;            // an error occurred OPEN handle properly
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
            // missing so create a new intersection type
            intid = tm->next_intid++;
            if (intid >= tm->max_intid) {
                tm->max_intid += TM_MAX_ID_INC_SIZE;
                _growTo((void **)&tm->tlid_by_intid, tm->max_intid * sizeof(TM_TLID_T), tm->mm, __FUNCTION__);
                _growTo((void **)&tm->btypid_by_intid, tm->max_intid * sizeof(BTYPEID_T), tm->mm, __FUNCTION__);
            }
            enum btexclusioncat excl = 0;  // OPEN: compute exclusion
            btypeid = _new_type_summary_at(tm, btint, excl, idx, 0);
            tm->tlid_by_intid[intid] = tlid;
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

pub BTYPEID_T tm_name_as(struct TM *tm, BTYPEID_T btypeid, char const *name) {
    int res;  BTYPEID_T existingId;  SYM_ID_T symid;  u32 idx;
    symid = sm_id(tm->sm, name);
    idx = ht_put_idx(TM_BTYPEID_BY_SYMIDHASH, tm->btypeid_by_symidhash, symid, &res);
    if (res == HT_EXISTS) {
        existingId = ht_entry(tm->btypeid_by_symidhash, idx);
        return (existingId == btypeid) ? btypeid : 0;
    } else {
        tm->symid_by_btypeid[btypeid] = symid;
        return btypeid;
    }
}

pub BTYPEID_T tm_nominal(struct TM *tm, char const * name) {
    int res;  BTYPEID_T btypeid;  SYM_ID_T symid;  u32 idx;
    symid = sm_id(tm->sm, name);                                        // get symid of name
    idx = ht_put_idx(TM_BTYPEID_BY_SYMIDHASH, tm->btypeid_by_symidhash, symid, &res);      // get index of the btypeid corresponding to symid
    if (res == HT_EXISTS) {
        btypeid = ht_entry(tm->btypeid_by_symidhash, idx);
        return (tm->summary_by_btypeid[btypeid].bmtid == btnom) ? btypeid : 0;   // check it's a nominal
    }
    else {
        // create a new nominal
        btypeid = tm->next_btypeId++;
        tm->symid_by_btypeid[btypeid] = symid;
        tm->summary_by_btypeid[btypeid].bmtid = btnom;
        ht_replace_empty(TM_BTYPEID_BY_SYMIDHASH, tm->btypeid_by_symidhash, idx, btypeid);
        return btypeid;
    }
}

tdd int tm_setNominal(struct TM *tm, char *name, BTYPEID_T btypeid) {
     SYM_ID_T symid;  int res;  u32 idx;
    if (tm->summary_by_btypeid[btypeid].bmtid != bterr) return 0;
    symid = sm_id(tm->sm, name);
    idx = ht_put_idx(TM_BTYPEID_BY_SYMIDHASH, tm->btypeid_by_symidhash, symid, &res);
    if (res == HT_EXISTS) return 0;

    tm->summary_by_btypeid[btypeid].bmtid = btnom;
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
    tm->max_rp = os_page_size();
    os_mprotect(tm->typelist_buf, tm->max_rp, BK_M_READ | BK_M_WRITE);      // make first page of typelist storage R/W
    os_madvise(tm->typelist_buf, tm->max_rp, BK_AD_RANDOM);                 // and advise as randomly accessed
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