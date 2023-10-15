#ifndef __BK_BTYPE_C
#define __BK_BTYPE_C "bk/btype.c"


#include "../../include/all.cfg"
#include "buckets.c"
#include "../../include/bk/mm.h"
#include "../../include/bk/tm.h"
#include "ht_impl.h"


pvt inline SM_SYM_ID_T symIdFromEntry(ht_struct(TM_BTYPE_ID_HT) const * const ht, SM_SYM_ID_T entry) {
    return ht->tm->symIdByBTypeId[entry];
}

pvt bool inline found(ht_struct(TM_BTYPE_ID_HT) const * const ht, SM_SYM_ID_T entry, BTYPE_ID_T key) {
    return ht->tm->symIdByBTypeId[entry] == key;
}

// HT_IMPL(name, slot_t, key_t, __hash_fn, __found_fn, __key_from_entry_fn)
HT_IMPL(TM_BTYPE_ID_HT, BTYPE_ID_T, SM_SYM_ID_T, ht_int32_hash, found, symIdFromEntry)


pub BTYPE_ID_T tm_exclnominal(struct TM *tm, char const * const name, enum btexclusioncat excl) {
    int res;  BTYPE_ID_T btypeId;  SM_SYM_ID_T symId;  u32 idx;
    symId = sm_id(tm->sm, name);                                        // get symid of name
    idx = ht_put_idx(TM_BTYPE_ID_HT, tm->btypeId_ht, symId, &res);      // get index of the btypeId corresponding to symId
    if (res == HT_EXISTS) {
        btypeId = ht_entry(tm->btypeId_ht, idx);
        struct btsummary summary = tm->sumByBTypeId[btypeId];
        if (summary.excl != excl || summary.mt != btnom) return 0;      // check it's a nominal with the same exclusion
        return btypeId;
    }
    else {
        // create a new nominal in an exclusive category
        btypeId = tm->nextBTypeId++;
        tm->symIdByBTypeId[btypeId] = symId;
        tm->sumByBTypeId[btypeId].mt = btnom;
        tm->sumByBTypeId[btypeId].excl = excl;
        ht_replace_empty(TM_BTYPE_ID_HT, tm->btypeId_ht, idx, btypeId);
        return btypeId;
    }
}

pub BTYPE_ID_T tm_id(struct TM *tm, char const * const name) {
    int res;  u32 idx;
    idx = ht_put_idx(TM_BTYPE_ID_HT, tm->btypeId_ht, sm_id(tm->sm, name), &res);
    if (res == HT_EXISTS)
        return tm->btypeId_ht->slots[idx];
    else
        return 0;
}

pub BTYPE_ID_T tm_inter(struct TM *tm, BTYPE_ID_T const *typelist) {
    return 0;
}

pub char * tm_name(struct TM *tm, BTYPE_ID_T btypeId) {
    // OPEN: do we return a null or "" if the type is unnamed? in Python we might want to return f't{btypeId}'
    SM_SYM_ID_T symId;
    if (btypeId >= tm->nextBTypeId) return "";
    symId = tm->symIdByBTypeId[btypeId];
    if (symId)
        return sm_name(tm->sm, symId);
    else
        return "";
}

pub BTYPE_ID_T tm_nominal(struct TM *tm, char const * const name) {
    int res;  BTYPE_ID_T btypeId;  SM_SYM_ID_T symId;  u32 idx;
    symId = sm_id(tm->sm, name);                                        // get symid of name
    idx = ht_put_idx(TM_BTYPE_ID_HT, tm->btypeId_ht, symId, &res);      // get index of the btypeId corresponding to symId
    if (res == HT_EXISTS) {
        btypeId = ht_entry(tm->btypeId_ht, idx);
        return (tm->sumByBTypeId[btypeId].mt == btnom) ? btypeId : 0;   // check it's a nominal
    }
    else {
        // create a new nominal
        btypeId = tm->nextBTypeId++;
        tm->symIdByBTypeId[btypeId] = symId;
        tm->sumByBTypeId[btypeId].mt = btnom;
        ht_replace_empty(TM_BTYPE_ID_HT, tm->btypeId_ht, idx, btypeId);
        return btypeId;
    }
}

tdd int tm_setNominal(struct TM *tm, char *name, BTYPE_ID_T btypeId) {
    struct btsummary summary;  SM_SYM_ID_T symId;  int res;  u32 idx;
    summary = tm->sumByBTypeId[btypeId];
    if (summary.mt != bterr) return 0;
    symId = sm_id(tm->sm, name);                                        // get symid of name
    idx = ht_put_idx(TM_BTYPE_ID_HT, tm->btypeId_ht, symId, &res);      // get index of the btypeId corresponding to symId
    if (res == HT_EXISTS) return 0;                                     // the name is already in use
    // create a new nominal
    summary.mt = btnom;
    tm->symIdByBTypeId[btypeId] = symId;
    ht_replace_empty(TM_BTYPE_ID_HT, tm->btypeId_ht, idx, btypeId);
    if (btypeId >= tm->nextBTypeId) tm->nextBTypeId = btypeId + 1;
    return btypeId;
}

pub struct TM * TM_create(struct MM *mm, struct SM *sm) {
    struct TM *tm = (struct TM *) mm->malloc(sizeof(struct TM));
    tm->mm = mm;
    tm->sm = sm;
    tm->symIdByBTypeIdSize = 0x10000;     // 64k btypes
    tm->symIdByBTypeId = (BTYPE_ID_T *) mm->malloc(tm->symIdByBTypeIdSize * sizeof(BTYPE_ID_T));
    memset(tm->symIdByBTypeId, 0, tm->symIdByBTypeIdSize * sizeof(BTYPE_ID_T));             // init to 0 OPEN: should we use calloc?
    tm->sumByBTypeId = (struct btsummary *) mm->malloc(tm->symIdByBTypeIdSize * sizeof(struct btsummary));
    memset(tm->sumByBTypeId, 0, tm->symIdByBTypeIdSize * sizeof(struct btsummary));         // init to 0 OPEN: should we use calloc?
    tm->btypeId_ht = ht_create(TM_BTYPE_ID_HT);
    tm->btypeId_ht->tm = tm;
    tm->nextBTypeId = 1;

    int n = 0;
    n += tm_setNominal(tm, "m8", _m8) == 0;
    n += tm_setNominal(tm, "m16", _m16) == 0;
    n += tm_setNominal(tm, "m32", _m32) == 0;
    n += tm_setNominal(tm, "m64", _m64) == 0;
    n += tm_setNominal(tm, "p64", _p64) == 0;
    n += tm_setNominal(tm, "litint", _litint) == 0;
    n += tm_setNominal(tm, "i32", _i32) == 0;

    if (n) {
        mm->free(tm);
        die("%i conflicts in tm_setNominal\n", n);
        return 0;
    }
    else {
        return tm;
    }
}

pub int TM_trash(struct TM *tm) {
    ht_trash(TM_BTYPE_ID_HT, tm->btypeId_ht);
    tm->mm->free(tm->symIdByBTypeId);
    tm->mm->free(tm);
    return 0;
}


#endif  // __BK_BTYPE_C