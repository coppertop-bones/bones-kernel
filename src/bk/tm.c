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


pub BTYPE_ID_T tm_id(struct TM *tm, char const * const name) {
    int res;
    // 1. get symid of name
    SM_SYM_ID_T symId = sm_id(tm->sm, name);
    // 2. get index of the btypeId corresponding to symId
    u32 idx = ht_put_idx(TM_BTYPE_ID_HT, tm->btypeId_ht, symId, &res);
    if (res == HT_EXISTS)
        return tm->btypeId_ht->slots[idx];
    else
        return 0;
}

pub char * tm_name(struct TM *tm, BTYPE_ID_T btype) {
    return 0;
}

pub BTYPE_ID_T tm_nominal(struct TM *tm, char const * const name) {
    int res; BTYPE_ID_T btypeId;
    // 1. get symid of name
    SM_SYM_ID_T symId = sm_id(tm->sm, name);
    // 2. get index of the btypeId corresponding to symId
    u32 idx = ht_put_idx(TM_BTYPE_ID_HT, tm->btypeId_ht, symId, &res);
    if (res == HT_EXISTS) {
        // check if it is a nominal
        btypeId = ht_entry(tm->btypeId_ht, idx);
        return (tm->sumByBTypeId[btypeId].mt == btnom) ? btypeId : 0;
    }
    else {
        // create a new nominal
        btypeId = tm->nextBTypeId++;
        tm->symIdByBTypeId[btypeId] = symId;
        tm->sumByBTypeId[btypeId].mt = btnom;
        tm->sumByBTypeId[btypeId].nomId = 1;
        ht_replace_empty(TM_BTYPE_ID_HT, tm->btypeId_ht, idx, btypeId);
        return btypeId;
    }
}

tdd int tm_setNominal(struct TM *tm, char *name, btype bt) {
    return 1;
}



pub struct TM * TM_create(struct MM *mm, struct SM *sm) {
    struct TM *tm = (struct TM *) mm->malloc(sizeof(struct TM));
    tm->mm = mm;
    tm->sm = sm;
    tm->symIdByBTypeIdSize = 0x10000;     // 64k btypes
    tm->symIdByBTypeId = (BTYPE_ID_T *) mm->malloc(tm->symIdByBTypeIdSize * sizeof(BTYPE_ID_T));
    tm->sumByBTypeId = (struct btsummary *) mm->malloc(tm->symIdByBTypeIdSize * sizeof(struct btsummary));
    tm->btypeId_ht = ht_create(TM_BTYPE_ID_HT);
    tm->btypeId_ht->tm = tm;
    tm->nextBTypeId = 1;

    int res = 0;
    res += tm_setNominal(tm, "nat", _nat) == 0;
    res += tm_setNominal(tm, "m8", _m8) == 0;
    res += tm_setNominal(tm, "m16", _m16) == 0;
    res += tm_setNominal(tm, "m32", _m32) == 0;
    res += tm_setNominal(tm, "m64", _m64) == 0;
    res += tm_setNominal(tm, "p64", _p64) == 0;
    res += tm_setNominal(tm, "p64", _litint) == 0;
    res += tm_setNominal(tm, "p64", _i32) == 0;

    if (res) {
        mm->free(tm);
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