// ---------------------------------------------------------------------------------------------------------------------
// Copyright 2025 David Briant, https://github.com/coppertop-bones. Licensed under the Apache License, Version 2.0
//
// TM - TYPE MANAGER LIFE CYCLE
// KEEPER REQUISITES: core
// ---------------------------------------------------------------------------------------------------------------------

#ifndef __BK_TM_TM_C
#define __BK_TM_TM_C "bk/tm/tm.c"


#include "core.c"



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

    // type lists
    tm->typelist_buf = os_vm_reserve(0, TM_MAX_TL_STORAGE);
    tm->max_tlrp = os_page_size() / sizeof(TM_TLID_T);
    os_mprotect(tm->typelist_buf, tm->max_tlrp * sizeof(TM_TLID_T), BK_PROT_READ | BK_PROT_WRITE);  // make first page of typelist storage R/W
    os_madvise(tm->typelist_buf, tm->max_tlrp * sizeof(TM_TLID_T), BK_MADV_RANDOM);                 // and advise as randomly accessed
    tm->next_tlrp = 1;                                          // this means tlid 0 will have zero length as desired
    tm->max_tlid = TM_MAX_SLID_INC_SIZE;
    tm->next_tlid = 1;
    tm->tlrp_by_tlid = (RP *) mm->malloc(tm->max_tlid * sizeof(RP));
    memset(tm->tlrp_by_tlid, 0, tm->max_tlid * sizeof(RP));     // zero out the first chunk
    tm->tlid_by_tlhash = hi_create(TM_TLID_BY_TLHASH);
    tm->tlid_by_tlhash->tm = tm;

    // type names
    tm->btypeid_by_symidhash = hi_create(TM_BTYPEID_BY_SYMIDHASH);
    tm->btypeid_by_symidhash->tm = tm;
    tm->max_btypeId = TM_MAX_BTYPEID_INC_SIZE;
    tm->next_btypeId = TM_FIRST_VALID_BTYPEID;
    tm->symid_by_btypeid = (TM_DETAILID_T *) mm->malloc(tm->max_btypeId * sizeof(TM_DETAILID_T));
    memset(tm->symid_by_btypeid, 0, tm->max_btypeId * sizeof(btypeid_t));

    // type summaries
    tm->btsummary_by_btypeid = (btsummary *) mm->malloc(tm->max_btypeId * sizeof(btsummary));
    memset(tm->btsummary_by_btypeid, 0, tm->max_btypeId * sizeof(btsummary));

    // spaces
    tm->spaceid_by_btypeid = (btypeid_t *) mm->malloc(tm->max_btypeId * sizeof(btypeid_t));
    memset(tm->spaceid_by_btypeid, 0, tm->max_btypeId * sizeof(btypeid_t));
    tm->implicitid_by_spaceid = (btypeid_t *) mm->malloc(tm->max_btypeId * sizeof(btypeid_t));
    memset(tm->implicitid_by_spaceid, 0, tm->max_btypeId * sizeof(btypeid_t));

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
    tm->tlid_by_strid = (TM_TLID_T *) mm->malloc(tm->max_strid * sizeof(TM_TLID_T));
    memset(tm->tlid_by_strid, 0, tm->max_strid * sizeof(TM_TLID_T));
    tm->slid_by_strid = (SM_SLID_T *) mm->malloc(tm->max_strid * sizeof(SM_SLID_T));
    memset(tm->slid_by_strid, 0, tm->max_strid * sizeof(SM_SLID_T));
    tm->btypid_by_strid = (btypeid_t *) mm->malloc(tm->max_strid * sizeof(btypeid_t));
    memset(tm->btypid_by_strid, 0, tm->max_strid * sizeof(btypeid_t));
    tm->strid_by_slidtlidhash = hi_create(TM_DETAILID_BY_SLIDTLIDHASH);
    tm->strid_by_slidtlidhash->tm = tm;

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

    // spaces
    tm->mm->free(tm->spaceid_by_btypeid);
    tm->mm->free(tm->implicitid_by_spaceid);

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
    tm->mm->free(tm->tlid_by_strid);
    tm->mm->free(tm->slid_by_strid);
    tm->mm->free(tm->btypid_by_strid);
    hi_trash(TM_DETAILID_BY_SLIDTLIDHASH, tm->strid_by_slidtlidhash);

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

#endif  // __BK_TM_TM_C
