// ---------------------------------------------------------------------------------------------------------------------
// Copyright 2025 David Briant, https://github.com/coppertop-bones. Licensed under the Apache License, Version 2.0
//
// UTILS
// KEEPER REQUISITES: core
// ---------------------------------------------------------------------------------------------------------------------

#ifndef __BK_TM_UTILS_C
#define __BK_TM_UTILS_C "bk/tm/utils.c"


#include "core.c"


// name binding / lookup

pub btypeid_t tm_bind(BK_TM *tm, char const *name, btypeid_t btypeid) {
    // binds name to btypeid, checking that
    //    1) name is not already used, and,
    //    2) that btypeid has not already been bound to
    int outcome;  symid_t symid;  u32 idx;  btsummary sum;

    if (!(TM_FIRST_VALID_BTYPEID <= btypeid && btypeid < tm->next_btypeId)) return B_NAT;
    if ((symid = tm->symid_by_btypeid[btypeid]) != 0)
        // already bound to - check that name is the same as the existing name
        return strcmp(sm_name(tm->sm, symid), name) == 0 ? btypeid : B_NAT;
    else {
        // not bound - check initialised or recursive
        sum = tm->btsummary_by_btypeid[btypeid];
        // int fred = TM_BMT_ID(sum);
        // int joe = TM_IS_RECURSIVE(sum);
        if (TM_BMT_ID(sum) == bmterr && !TM_IS_RECURSIVE(sum)) return B_NAT;
        // check name is not already in use and bind
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

pub btypeid_t tm_lookup(BK_TM *tm, char const *name) {
    // answer the btypeid that name is bound to else B_NAT if name is not bound
    int outcome;  u32 idx;
    idx = hi_put_idx(TM_BTYPEID_BY_SYMIDHASH, tm->btypeid_by_symidhash, sm_id(tm->sm, name), &outcome);
    if (outcome == HI_LIVE)
        return tm->btypeid_by_symidhash->tokens[idx];
    else
        return B_NAT;
}

pub char const * tm_name_of(BK_TM *tm, btypeid_t btypeid) {
    // answers the name bound to btypeid or a null pointer there has no binding
    if (!(TM_FIRST_VALID_BTYPEID <= btypeid && btypeid < tm->next_btypeId)) return 0;
    symid_t symid = tm->symid_by_btypeid[btypeid];
    return symid ? sm_name(tm->sm, symid) : 0;
}


// id reservation

// pvt btypeid_t tm_reserve(BK_TM *tm) {
//     // answers a newly reserved btypeid
//     return _update_type_summary(tm, tm->next_btypeId, 0, 0, false);
// }

pub btypeid_t tm_reserve_in(BK_TM *tm, btypeid_t spaceid) {
    // answers a newly reserved btypeid in spaceid
    return tm_set_spaceid(tm, _update_type_summary(tm, tm->next_btypeId, 0, 0, false), spaceid);
}

pub btypeid_t tm_reserve_tbc(BK_TM *tm) {
    // answers a newly reserved btypeid marking it as recursive (usual use case for tbc)
    return tm_set_tbc(tm, _update_type_summary(tm, tm->next_btypeId, 0, 0, false));
}

pub void tm_reserve_block(BK_TM *tm, btypeid_t next_btypeId) {
    while (next_btypeId >= tm->max_btypeId) {
        tm->max_btypeId += TM_MAX_BTYPEID_INC_SIZE;
        _growTo((void **)&tm->btsummary_by_btypeid, tm->max_btypeId * sizeof(btsummary), tm->mm, FN_NAME);
        _growTo((void **)&tm->spaceid_by_btypeid, tm->max_btypeId * sizeof(btypeid_t), tm->mm, FN_NAME);
        _growTo((void **)&tm->implicitid_by_spaceid, tm->max_btypeId * sizeof(btypeid_t), tm->mm, FN_NAME);
        _growTo((void **)&tm->symid_by_btypeid, tm->max_btypeId * sizeof(symid_t), tm->mm, FN_NAME);
    }
    if (next_btypeId >= tm->next_btypeId) tm->next_btypeId = next_btypeId;
}


// attribute accessing

pub bmetatypeid_t tm_bmetatypeid(BK_TM *tm, btypeid_t btypeid) {
    // answer the bmetatypeid_t corresponding to btypeid or bmterr if not found
    if (!(TM_FIRST_VALID_BTYPEID <= btypeid && btypeid < tm->next_btypeId)) return bmterr;
    return TM_BMT_ID(tm->btsummary_by_btypeid[btypeid]);
}

pub bool tm_hasT(BK_TM *tm, btypeid_t btypeid) {
    if (!(TM_FIRST_VALID_BTYPEID <= btypeid && btypeid < tm->next_btypeId)) return false;
    return TM_HAS_T(tm->btsummary_by_btypeid[btypeid]);
}

pub btypeid_t tm_layout(BK_TM *tm, btypeid_t btypeid) {
    // OPEN: implement
    return B_NAT;
}

pub btypeid_t tm_layout_as(BK_TM *tm, btypeid_t btypeid, size sz) {
    // OPEN: implement
    return B_NAT;
}

pub btypeid_t tm_spaceid(BK_TM *tm, btypeid_t btypeid) {
    // answers the space id for the given btypeid
    return tm->spaceid_by_btypeid[btypeid];
}

pub btypeid_t tm_root_spaceid(BK_TM *tm, btypeid_t btypeid) {
    // answers the root space id for the given btypeid - allowing for 1 level of recursion (e.g. ccy in ccy)
    btypeid_t spaceid = B_NAT;
    while ((btypeid = tm->spaceid_by_btypeid[btypeid]) && btypeid != spaceid) {
        spaceid = btypeid;
    }
    return spaceid;
}

pub size tm_size(BK_TM *tm, btypeid_t btypeid) {
    // OPEN: implement (requires packing decisions which should be put in the client? except the mm needs to be able to navigate)
    //    in which case this should be field alignment, offsets and sizes, e.g. 0,8 for a f64
    return 0;
}

pub btypeid_t tm_set_spaceid(BK_TM *tm, btypeid_t btypeid, btypeid_t spaceid) {
    btypeid_t oldid;  bmetatypeid_t bmtid;
    if ((oldid = tm->spaceid_by_btypeid[btypeid]) && oldid != spaceid) return B_NAT;
    if (!spaceid) return B_NAT;
    bmtid = TM_BMT_ID(tm->btsummary_by_btypeid[btypeid]);
    if (bmtid == bmterr || bmtid == bmtatm || bmtid == bmtint) {
        tm->btsummary_by_btypeid[btypeid] |= TM_IN_SPACE_MASK;
        tm->spaceid_by_btypeid[btypeid] = spaceid;
        return btypeid;
    } else {
        return B_NAT;
    }
}

pub btypeid_t tm_set_tbc(BK_TM *tm, btypeid_t btypeid) {
    if (!(TM_FIRST_VALID_BTYPEID <= btypeid && btypeid < tm->next_btypeId)) return B_NAT;
    bmetatypeid_t bmtid = TM_BMT_ID(tm->btsummary_by_btypeid[btypeid]);
    if (bmtid == bmterr) {
        tm->btsummary_by_btypeid[btypeid] |= TM_IS_RECURSIVE_MASK;
        return btypeid;
    } else {
        return B_NAT;
    }
}

// recursion

pub btypeid_t tm_space_would_deeply_recurse(BK_TM *tm, btypeid_t btypeid, btypeid_t spaceid) {
    // answers B_NAT if no problem else answers btypeid for which spaceid would deeply recurse
    btypeid_t parentid, oldid;  int i = 0;
    if (btypeid == B_NEW) {btypeid = tm->next_btypeId;}
    if (spaceid == btypeid) return B_NAT;                   // in self, i.e. one level of recursion, is fine
    oldid = tm->spaceid_by_btypeid[btypeid];
    tm->spaceid_by_btypeid[btypeid] = spaceid;              // temporarily put btypeid in the space hierarchy
    parentid = tm->spaceid_by_btypeid[spaceid];
    while (i < 20 && parentid != B_NAT) {
        parentid = tm->spaceid_by_btypeid[parentid];        // we hope to hit a B_NAT within 20 iterations
        i++;
    }
    tm->spaceid_by_btypeid[btypeid] = oldid;                    // restore the space hierarchy to its prior state
    return (parentid) ? btypeid : B_NAT;
}

pub btypeid_t tm_clear_recursive_if_not(BK_TM *tm, btypeid_t btypeid) {
    // OPEN: implement
    // tm->btsummary_by_btypeid[btypeid] &= ~TM_IS_RECURSIVE_MASK;       // clear the recursive flag
    return btypeid;
}


// utils

pub btypeid_t tm_minus(BK_TM *tm, btypeid_t btypeid, btypeid_t A, btypeid_t B) {
    // for the intersection or union A answers the same metatype less B (or B's types if intersection or union),
    // creating if necessary, returning B_NAT if empty
    btsummary *sumA, *sumB; btypeid_t *tlA1, *tlB1, *tlA2, *tlB2, *p1, *p2, *p;  int nA, nB, nDest;
    TM_TLID_T tlid;  u32 idx;  i32 outcome;

    if (!(TM_FIRST_VALID_BTYPEID <= A && A < tm->next_btypeId)) return _err_btypeidOutOfRange(B_NAT, __FILE__, FN_NAME, __LINE__, btypeid);
    if (!(TM_FIRST_VALID_BTYPEID <= B && B < tm->next_btypeId)) return _err_btypeidOutOfRange(B_NAT, __FILE__, FN_NAME, __LINE__, btypeid);
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
    _make_next_page_of_typelist_buf_writable_if_necessary(tm, nA*2);
    p1 = p2 = tm->typelist_buf + tm->next_tlrp;                               // both point to size element in tlDest
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
            bool match = 0;
            for (p = tlB1; p <= tlB2; p++) {
                if (*tlA1 == *p) {
                    match = true;
                    break;
                }
            }
            if (!match) *(++p2) = *tlA1;    // if no match inc dest ptr and copy
        }
    }
    else {
        nB = 1;
        for (++tlA1; tlA1 <= tlA2; tlA1++) if (*tlA1 != B) *(++p2) = *tlA1;    // if no match inc dest ptr and copy
    }
    nDest = p2 - p1;
    if (!nDest) return B_NAT;               // no types left
    if (nDest == nA) return B_NAT;          // all types matched
    if (nDest + nB != nA) return B_NAT;     // OPEN: should we allow (t1, t2, t3) - (t2, t4)?
    if (nDest == 1) return *p2;
    *p1 = nDest;

    // get the tlid for the typelist - adding if missing, returning 0 if invalid
    idx = hi_put_idx(TM_TLID_BY_TLHASH, tm->tlid_by_tlhash, p1, &outcome);
    switch (outcome) {
        default:
            die("%s: HI_TOMBSTONE1!", FN_NAME);
        case HI_LIVE:
            tlid = tm->tlid_by_tlhash->tokens[idx];
            break;
        case HI_EMPTY:
            tlid = _commit_typelist_buf_at(tm, p1, idx);
            if (!tlid) return _seriousErrorCommitingTypelistBufHandleProperly(B_NAT, __FILE__, __LINE__);
    }

    return (TM_BMT_ID(*sumA) == bmtint) ? tm_inter_for_tlid_or_create(tm, btypeid, tlid) : tm_union_for_tlid_or_create(tm, btypeid, tlid);
}

pub TM_TLID_T tm_tlid_for(BK_TM *tm, btypeid_t *typelist) {
    i32 i, outcome, numTypes;  btsummary *sum;  btypeid_t *p1, *nextTypelist;  TM_TLID_T tlid;  u32 idx;

    numTypes = typelist[0];

    // check each typeid in the list is valid
    // OPEN: can this loop be merged with the copying loop?
    for (i = 1; i <= numTypes; i++) {
        if (!(TM_FIRST_VALID_BTYPEID <= typelist[i] && typelist[i] < tm->next_btypeId)) return _err_itemInTLOutOfRange(B_NAT, __FILE__, FN_NAME, __LINE__, typelist[i], i);
        sum = tm->btsummary_by_btypeid + typelist[i];
        if (!TM_IS_RECURSIVE(*sum) && TM_BMT_ID(*sum) == bmterr) return _btypeInTypeListNotInitialised(B_NAT, __FILE__, __LINE__, typelist[i], i);;
    }

    // make next page of tm->typelist_buf writable if necessary
    // ensure we have enough space for typelist
    _make_next_page_of_typelist_buf_writable_if_necessary(tm, numTypes * 2);

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
            tlid = _commit_typelist_buf_at(tm, nextTypelist, idx);
            if (!tlid) return _seriousErrorCommitingTypelistBufHandleProperly(B_NAT, __FILE__, __LINE__);
    }

    return tlid;
}

#endif  // __BK_TM_UTILS_C