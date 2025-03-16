// ---------------------------------------------------------------------------------------------------------------------
// UTILS
// KEEPER REQUISITES: core
// ---------------------------------------------------------------------------------------------------------------------

#ifndef __BK_TM_UTILS_C
#define __BK_TM_UTILS_C "bk/tm/utils.c"


#include "core.c"


// variable get/set etc

pub btypeid_t tm_get(BK_TM *tm, char const *name) {
    // answer the btypeid corresponding to name or B_NAT if not found
    int outcome;  u32 idx;
    idx = hi_put_idx(TM_BTYPEID_BY_SYMIDHASH, tm->btypeid_by_symidhash, sm_id(tm->sm, name), &outcome);
    if (outcome == HI_LIVE)
        return tm->btypeid_by_symidhash->tokens[idx];
    else
        return B_NAT;
}

pub btypeid_t tm_set(BK_TM *tm, btypeid_t self, char const *name) {
    int outcome;  symid_t symid;  u32 idx;

    // assigns name to the unnamed btypedid, checking that name is not already used
    if (!(TM_FIRST_VALID_BTYPEID <= self && self < tm->next_btypeId)) return B_NAT;
    if ((symid = tm->symid_by_btypeid[self]) != 0)
        // already named - check the given name is the same as the existing name
        return strcmp(sm_name(tm->sm, symid), name) == 0 ? self : B_NAT;
    else {
        // not named so check name is not already in use
        symid = sm_id(tm->sm, name);
        idx = hi_put_idx(TM_BTYPEID_BY_SYMIDHASH, tm->btypeid_by_symidhash, symid, &outcome);
        if (outcome == HI_LIVE)
            return B_NAT;
        else {
            hi_replace_empty(TM_BTYPEID_BY_SYMIDHASH, tm->btypeid_by_symidhash, idx, self);
            tm->symid_by_btypeid[self] = symid;
            return self;
        }
    }
}


// id reservation

pub btypeid_t tm_reserve(BK_TM *tm, btypeid_t self, btypeid_t orthspcid, bool isexp, btypeid_t implicitid) {
    // answers self, allocating if required, initialised (partially) with the given properties
    if (self == B_NEW) {
        self = tm->next_btypeId;
        _update_type_summary(tm, self, 0, 0, 0);
    } else {
        if (!(TM_FIRST_VALID_BTYPEID <= self && self < tm->next_btypeId)) return B_NAT;
        if (TM_BMT_ID(tm->btsummary_by_btypeid[self]) != bmterr) return B_NAT;
    }
    tm->btsummary_by_btypeid[self] |=
            TM_IS_RECURSIVE_MASK |                  // assume recursive, might be unset later
            (isexp ? TM_IS_EXPLICIT_MASK : 0) |
            (orthspcid ? TM_IN_ORTHSPC_MASK : 0);
    if (orthspcid) tm->orthspcid_by_btypeid[self] = orthspcid;
    if (implicitid) tm->implicitid_by_orthspcid[self] = implicitid;
    return self;
}

pub void tm_reserve_btypeids(BK_TM *tm, btypeid_t next_btypeId) {
    while (next_btypeId >= tm->max_btypeId) {
        tm->max_btypeId += TM_MAX_BTYPEID_INC_SIZE;
        _growTo((void **)&tm->btsummary_by_btypeid, tm->max_btypeId * sizeof(btsummary), tm->mm, FN_NAME);
        _growTo((void **)&tm->orthspcid_by_btypeid, tm->max_btypeId * sizeof(btypeid_t), tm->mm, FN_NAME);
        _growTo((void **)&tm->implicitid_by_orthspcid, tm->max_btypeId * sizeof(btypeid_t), tm->mm, FN_NAME);
        _growTo((void **)&tm->symid_by_btypeid, tm->max_btypeId * sizeof(symid_t), tm->mm, FN_NAME);
    }
    if (next_btypeId >= tm->next_btypeId) tm->next_btypeId = next_btypeId;
}


// attribute accessing

pub bmetatypeid_t tm_bmetatypeid(BK_TM *tm, btypeid_t self) {
    // answer the bmetatypeid_t corresponding to self or bmterr if not found
    if (!(TM_FIRST_VALID_BTYPEID <= self && self < tm->next_btypeId)) return bmterr;
    return TM_BMT_ID(tm->btsummary_by_btypeid[self]);
}

pub bool tm_hasT(BK_TM *tm, btypeid_t self) {
    if (!(TM_FIRST_VALID_BTYPEID <= self && self < tm->next_btypeId)) return false;
    return TM_HAS_T(tm->btsummary_by_btypeid[self]);
}

pub btypeid_t tm_layout(BK_TM *tm, btypeid_t self) {
    // OPEN: implement
    return B_NAT;
}

pub btypeid_t tm_layout_as(BK_TM *tm, btypeid_t self, size sz) {
    // OPEN: implement
    return B_NAT;
}

pub char const * tm_name(BK_TM *tm, btypeid_t self) {
    // answers the name of the given type or a null pointer it has no name
    if (!(TM_FIRST_VALID_BTYPEID <= self && self < tm->next_btypeId)) return 0;
    symid_t symid = tm->symid_by_btypeid[self];
    return symid ? sm_name(tm->sm, symid) : 0;
}

pub btypeid_t tm_orthspcid(BK_TM *tm, btypeid_t self) {
    // answers the orthogonal space id for the given btype
    return tm->orthspcid_by_btypeid[self];
}

pub btypeid_t tm_root_orthspcid(BK_TM *tm, btypeid_t self) {
    // answers the root orthogonal space id for the given btype
    btypeid_t orthspcid = 0;
    while ((self = tm->orthspcid_by_btypeid[self])) {
        orthspcid = self;
    }
    return orthspcid;
}

pub size tm_size(BK_TM *tm, btypeid_t self) {
    // OPEN: implement (requires packing decisions which should be put in the client? except the mm needs to be able to navigate)
    //    in which case this should be field alignment, offsets and sizes, e.g. 0,8 for a f64
    return 0;
}


// utils

pub btypeid_t tm_minus(BK_TM *tm, btypeid_t self, btypeid_t A, btypeid_t B) {
    btsummary *sumA, *sumB; btypeid_t *tlA1, *tlB1, *tlA2, *tlB2, *tlDest1, *tlDest2, *p;  int nA, nB, nDest;
    TM_TLID_T tlid;  u32 idx;  i32 outcome;  i32 numTypes;  btypeid_t *nextTypelist;

    if (!(TM_FIRST_VALID_BTYPEID <= A && A < tm->next_btypeId)) return B_NAT;
    if (!(TM_FIRST_VALID_BTYPEID <= B && B < tm->next_btypeId)) return B_NAT;
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
            if (!tlid) return _seriousErrorCommitingTypelistBufHandleProperly(B_NAT, __FILE__, __LINE__);
    }

    return (TM_BMT_ID(*sumA) == bmtint) ? tm_inter_for_tlid_or_create(tm, self, tlid) : tm_union_for_tlid_or_create(tm, self, tlid);
}

pub TM_TLID_T tm_tlid(BK_TM *tm, btypeid_t *typelist) {
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
            tlid = _commit_typelist_buf_at(tm, numTypes, idx);
            if (!tlid) return _seriousErrorCommitingTypelistBufHandleProperly(B_NAT, __FILE__, __LINE__);
    }

    return tlid;
}

#endif  // __BK_TM_UTILS_C