// ---------------------------------------------------------------------------------------------------------------------
// Copyright 2025 David Briant, https://github.com/coppertop-bones. Licensed under the Apache License, Version 2.0
//
// UNION IMPLEMENTATION
// KEEPER REQUISITES: core
// ---------------------------------------------------------------------------------------------------------------------

#ifndef SRC_BK_TM_UNION_C
#define SRC_BK_TM_UNION_C "bk/tm/union.c"


#include "core.c"

pub btypeid_t tm_union(BK_TM *tm, btypeid_t btypeid, btypeid_t *typelist) {
    // answers the validated union type corresponding to typelist, creating if necessary
    TM_TLID_T tlid, *nextTypelist;

    if (!btypeid) return _err_invalid_btype_B_NAT(B_NAT, __FILE__, FN_NAME, __LINE__);
    if (!typelist[0]) return _err_emptyTypelist(B_NAT, __FILE__, FN_NAME, __LINE__);;
    if (btypeid >= tm->next_btypeId) return _err_btypeidOutOfRange(B_NAT, __FILE__, FN_NAME, __LINE__, btypeid);;

    tlid = tm_union_tlid_for(tm, typelist);
    if (!tlid) {
        nextTypelist = tm->typelist_buf + tm->next_tlrp;
        if (nextTypelist[0] == 1) {
            // we have a single type so return it
            return nextTypelist[1];
        } else {
            // something else went wrong
            return B_NAT;
        }
    } else {
        btypeid = tm_union_for_tlid_or_create(tm, btypeid, tlid);
        // PP(info, "tm_union - btypeid: %i, tlid: %i, len: %i", btypeid, tlid, (tm->typelist_buf + tm->tlrp_by_tlid[tlid])[0]);
        return btypeid;
    }
}

pub TM_TLID_T tm_union_tlid_for(BK_TM *tm, btypeid_t *typelist) {
    // answers the validated union typelist id corresponding to typelist, creating if necessary
    // very similar to tm_inter_tlid_for but a little different
    i32 i, j, typelistCount, numTypes, outcome;  btsummary *sum;  TM_TLID_T tlid;  btypeid_t *uniTl, *p1, *p2, *p3, *nextTypelist;
    u32 idx;

    // check typeid is in range, and figure total possible length (including possible duplicates from child unions)
    numTypes = 0;  typelistCount = typelist[0];
    for (i = 1; i <= typelistCount; i++) {
        if (!(TM_FIRST_VALID_BTYPEID <= typelist[i] && typelist[i] < tm->next_btypeId)) return _err_itemInTLOutOfRange(B_NAT, __FILE__, FN_NAME, __LINE__, typelist[i], i);;
        sum = tm->btsummary_by_btypeid + typelist[i];
        if (TM_BMT_ID(*sum) == bmtuni) {
            tlid = tm->tlid_by_uniid[TM_DETAILS_ID(*sum)];
            numTypes += (int) (tm->typelist_buf + tm->tlrp_by_tlid[tlid])[0];
        } else {
            numTypes += 1;
        }
    }

    // ensure we have enough space for union
    _make_next_page_of_typelist_buf_writable_if_necessary(tm, 1 + numTypes);

    nextTypelist = tm->typelist_buf + tm->next_tlrp;

    // copy typelist into nextTypelist unpacking any unions
    // PP(info, "tm_union_tlid_for - #2, numTypes=%i, typelistCount=%i", numTypes, typelistCount);
    p1 = nextTypelist;
    p1++;
    for (i = 1; i <= typelistCount; i++) {
        sum = tm->btsummary_by_btypeid + typelist[i];
        if (TM_BMT_ID(*sum) == bmtuni) {
            // we have a union type - expand it
            tlid = tm->tlid_by_uniid[TM_DETAILS_ID(*sum)];
            uniTl = (tm->typelist_buf + tm->tlrp_by_tlid[tlid]);
            for (j = 1; j <= (i32) uniTl[0]; j++) *p1++ = uniTl[j];
        } else
            *p1++ = typelist[i];
//        PP(info, "tm_union_tlid_for - #3 btype: %i", *(p1 - 1));
    }
    numTypes = p1 - (nextTypelist + 1);

    // sort types into btypeid order
    ks_radix_sort(btypeid_t, nextTypelist + 1, numTypes);
//    for (i = 1; i <= p1 - (nextTypelist + 1); i++)
//        PP(info, "tm_union_tlid_for - #4 sortd btype: %i", nextTypelist[i]);

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
    nextTypelist[0] = numTypes = p1 - nextTypelist;

    // if just one type return error
    if (numTypes == 1) return 0;

    // OPEN: be a good citizen and zero out the scratch?

    // get the tlid for the typelist - adding if missing, returning 0 if invalid
    idx = hi_put_idx(TM_TLID_BY_TLHASH, tm->tlid_by_tlhash, nextTypelist, &outcome);
//    PP(info, "tm_union_tlid_for - #5 tl hash: %i, idx=%i", tl_hash(nextTypelist), idx);
    switch (outcome) {
        default:
            die("%s: HI_TOMBSTONE1!", FN_NAME);
        case HI_LIVE:
            tlid = tm->tlid_by_tlhash->tokens[idx];
//            PP(info, "tm_union_tlid_for - #6 return existing tlid=%i @idx=%i", tlid, idx);
            break;
        case HI_EMPTY:
            tlid = _commit_typelist_buf_at(tm, nextTypelist, idx);
//            /PP(info, "tm_union_tlid_for - #7 committed tlid=%i, len=%i, @ idx=%i", tlid, numTypes, idx);
            if (!tlid) return _seriousErrorCommitingTypelistBufHandleProperly(B_NAT, __FILE__, __LINE__);
    }
    return tlid;
}

pub btypeid_t tm_union_for_tlid(BK_TM *tm, TM_TLID_T tlid) {
    // use-case here is to check a union doesn't exist before reserving a type
    u32 idx;  i32 outcome;

    idx = hi_put_idx(TM_DETAILID_BY_TLIDHASH, tm->uniid_by_tlidhash, tlid, &outcome);
    switch (outcome) {
        default:
            die("%s: HI_TOMBSTONE2!", FN_NAME);
        case HI_LIVE:
            return tm->btypid_by_uniid[tm->uniid_by_tlidhash->tokens[idx]];
        case HI_EMPTY:
            return B_NAT;
    }
}

pub btypeid_t tm_union_for_tlid_or_create(BK_TM *tm, btypeid_t btypeid, TM_TLID_T tlid) {
    u32 idx;  i32 outcome, i;  TM_DETAILID_T uniid;  btypeid_t *typelist, other; i32 numTypes;  bool hasT;

    // get the btypeid for the tlid
    idx = hi_put_idx(TM_DETAILID_BY_TLIDHASH, tm->uniid_by_tlidhash, tlid, &outcome);
    switch (outcome) {
        default:
            die("%s: HI_TOMBSTONE2!", FN_NAME);
        case HI_LIVE:
            // typelist already exists
//            PP(info, "tm_union_for_tlid_or_create - #1");
            uniid = tm->uniid_by_tlidhash->tokens[idx];
            if (btypeid == B_NEW) return tm->btypid_by_uniid[uniid];
            else if (btypeid == (other = tm->btypid_by_uniid[uniid])) return btypeid;
            else return _err_otherAlreadyRepresentsTL(B_NAT, __FILE__, __LINE__, btypeid, other);
        case HI_EMPTY:
            // missing so commit the union type for tlid
//            PP(info, "tm_union_for_tlid_or_create - #2");
            if (btypeid == B_NEW) {
                btypeid = tm->next_btypeId;
            } else if (TM_BMT_ID(tm->btsummary_by_btypeid[btypeid]) != bmterr)
                // btypeid is already in use so given the type list lookup above we cannot be referring to the same btype
                return _err_btypeAlreadyInitialised(B_NAT, __FILE__, __LINE__, btypeid);
            uniid = tm->next_uniid++;
            if (uniid >= tm->max_uniid) {
                tm->max_uniid += TM_MAX_ID_INC_SIZE;
                _growTo((void **) &tm->tlid_by_uniid, tm->max_uniid * sizeof(TM_TLID_T), tm->mm, FN_NAME);
                _growTo((void **) &tm->btypid_by_uniid, tm->max_uniid * sizeof(btypeid_t), tm->mm, FN_NAME);
            }
            tm->tlid_by_uniid[uniid] = tlid;

            hasT = false;
            typelist = tm->typelist_buf + tm->tlrp_by_tlid[tlid];
            numTypes = *typelist;
            for (i = 1; i <= numTypes; i++) hasT = hasT || TM_HAS_T(tm->btsummary_by_btypeid[typelist[i]]);

            btypeid = _update_type_summary(tm, btypeid, uniid, 0, hasT);
            tm->btsummary_by_btypeid[btypeid] |= bmtuni;
            tm->btypid_by_uniid[uniid] = btypeid;
            hi_replace_empty(TM_DETAILID_BY_TLIDHASH, tm->uniid_by_tlidhash, idx, uniid);

//            PP(info, "tm_union_for_tlid_or_create - #3 - tl: %li, tlid: %i, len: %i", typelist, tlid, numTypes);
            return btypeid;
    }
}

pub btypeid_t tm_unionv(BK_TM *tm, btypeid_t self, u32 numTypes, ...) {
    va_list args;  btypeid_t *typelist;  int i;
    va_start(args, numTypes);
    typelist = malloc((1 + numTypes) * sizeof(btypeid_t));      // OPEN: use a typelist buffer of big enough size
    for (i = 1; i <= numTypes; i++) typelist[i] = va_arg(args, btypeid_t);
    typelist[0] = numTypes;
    self = tm_union(tm, self, typelist);
    free(typelist);
    va_end(args);
    return self;
}

//another way of doing this:
//
//    pub btypeid_t _tm_unionv2(BK_TM *tm, btypeid_t self, u32 numTypes, btypeid_t *args) {
//        btypeid_t *typelist;  int i;
//        typelist = malloc((1 + numTypes) * sizeof(btypeid_t));
//        for (i = 1; i <= numTypes; i++) typelist[i] = args[i-1];
//        typelist[0] = numTypes;
//        self = tm_union(tm, self, typelist);
//        free(typelist);
//        return self;
//    }

pub btypeid_t * tm_union_tl(BK_TM *tm, btypeid_t btypeid) {
    // answer a typelist ptr to the given union's types or null for error
    btsummary *sum;  TM_TLID_T tlid;

    if (!btypeid) return 0; // (btypeid_t *) _err_invalid_btype_B_NAT(0, __FILE__, FN_NAME, __LINE__);
    if (btypeid >= tm->next_btypeId) return 0; // _err_btypeidOutOfRange(NULL, __FILE__, FN_NAME, __LINE__, btypeid);

    sum = tm->btsummary_by_btypeid + btypeid;
    if (TM_BMT_ID(*sum) == bmtuni) {
        tlid = tm->tlid_by_uniid[TM_DETAILS_ID(*sum)];
//        tl = tm->typelist_buf + tm->tlrp_by_tlid[tlid];
//        PP(info, "tm_union_tl - btypeid: %i, tl:%li, tlid: %i, len: %i", btypeid, tl, tlid, tl[0]);
        return tm->typelist_buf + tm->tlrp_by_tlid[tlid];
    } else {
        return 0;
    }
}

#define BK_UNION(tm, ...) ({                                                                                            \
    btypeid_t args[] = { __VA_ARGS__ };                                                                                 \
    _tm_unionv((tm), 0, sizeof(args) / sizeof(args[0]), args);                                                        \
})

#endif  // SRC_BK_TM_UNION_C