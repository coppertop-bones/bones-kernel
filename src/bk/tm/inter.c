// ---------------------------------------------------------------------------------------------------------------------
// Copyright 2025 David Briant, https://github.com/coppertop-bones. Licensed under the Apache License, Version 2.0
//
// INTERSECTION IMPLEMENTATION
// KEEPER REQUISITES: core
// ---------------------------------------------------------------------------------------------------------------------

#ifndef SRC_BK_TM_INTER_C
#define SRC_BK_TM_INTER_C "bk/tm/inter.c"


#include "core.c"


pub btypeid_t tm_check_inter(BK_TM *tm, btypeid_t btypeid, btypeid_t spaceid) {
    // An existing intersection is being defined a second time check that the attributes don't conflict
    //      a) if current.space is already set then space may be the same or missing
    return btypeid;
}

pvt TM_TLID_T tm_inter_tlid_for_impl(BK_TM *tm, const btypeid_t *types, bool check) {
    // answers the validated intersection tlid corresponding to types, creating if necessary
    // NB: the length of a resulting tl may be longer than the length of types being intersected (dues to expansion) 
    // and may contain types with conflicting spaces

    int typesCount=(int)types[0], tlCount=0, spaceCount=0, i, j, i_tl, i_sbi, outcome;  btsummary sum;
    TM_TLID_T tlid;  u32 idx;  bool hasUnions=false;  btypeid_t *tl, *childtl, btypeid, childId, spaceid, *spaces, *btypes, *origins;

    // memory layout
    //  tl                                           sbi
    // |   N    | btype1 | btype2 |  ...   | btypeN |           sbi0           |           sbi1           |           ...            |

    // PP(info, "%s@%i -------------------------", FN_NAME, __LINE__);

    // check types are in range, estimate number of types in answer and count types with spaces
    for (i = 1; i <= typesCount; i++) {
        btypeid = types[i];
        if (!(TM_FIRST_VALID_BTYPEID <= btypeid && btypeid < tm->next_btypeId)) return _err_itemInTLOutOfRange(0, __FILE__, FN_NAME, __LINE__, btypeid, i);
        sum = tm->btsummary_by_btypeid[btypeid];
        if (TM_BMT_ID(sum) == bmtint) {
            if (TM_IN_SPACE(sum) && (tm_root_spaceid(tm, btypeid)))
                spaceCount++;
            childtl = tm->typelist_buf + tm->tlrp_by_tlid[tm->tlid_by_intid[TM_DETAILS_ID(sum)]];
            for (j = 1; j <= (i32) childtl[0]; j++) {
                childId = childtl[j];
                sum = tm->btsummary_by_btypeid[childId];
                if (TM_IN_SPACE(sum) && (tm_root_spaceid(tm, childId)))
                    spaceCount++;
                tlCount++;
                hasUnions |= TM_BMT_ID(sum) == bmtuni;
            }
        } else if (TM_BMT_ID(sum) == bmtatm) {
            if (TM_IN_SPACE(sum) && (tm_root_spaceid(tm, btypeid)))
                spaceCount++;
            tlCount++;
            hasUnions |= TM_BMT_ID(sum) == bmtuni;
        } else {
            tlCount++;
            hasUnions |= TM_BMT_ID(sum) == bmtuni;
        }
    }
    if (tlCount <= 1) return setErrAndDesc(0, "Not enough input types", __FILE__, __LINE__);

    // OPEN: handle intersections of unions?
    if (hasUnions) return setErrAndDesc(0, "Has unions", __FILE__, __LINE__);

    // ensure we have enough space for all types in answer (which will likely be compacted) plus a buffer of spaceCount * 3
    _make_next_page_of_typelist_buf_writable_if_necessary(tm, 1 + tlCount + 3 * spaceCount);

    // fill sbi and tl
    tl = tm->typelist_buf + tm->next_tlrp;
    spaces = tl + 1 + tlCount;
    btypes = spaces + spaceCount;
    origins = btypes + spaceCount;

    i_tl = 1;  i_sbi = 0;
    for (i = 1; i <= typesCount; i++) {
        btypeid = types[i];
        sum = tm->btsummary_by_btypeid[btypeid];
        if (TM_BMT_ID(sum) == bmtint) {
            if (TM_IN_SPACE(sum) && (spaceid = tm_root_spaceid(tm, btypeid))) {
                spaces[i_sbi] = spaceid;
                btypes[i_sbi] = btypeid;
                origins[i_sbi] = i;
                i_sbi++;
            }
            childtl = tm->typelist_buf + tm->tlrp_by_tlid[tm->tlid_by_intid[TM_DETAILS_ID(sum)]];
            for (j = 1; j <= (i32) childtl[0]; j++) {
                childId = childtl[j];
                sum = tm->btsummary_by_btypeid[childId];
                if (TM_IN_SPACE(sum) && (spaceid = tm_root_spaceid(tm, childId))) {
                    spaces[i_sbi] = spaceid;
                    btypes[i_sbi] = childId;
                    origins[i_sbi] = i;
                    i_sbi++;
                }
                tl[i_tl++] = childId;
            }
        } else if (TM_BMT_ID(sum) == bmtatm) {
            if (TM_IN_SPACE(sum) && (spaceid = tm_root_spaceid(tm, btypeid))) {
                spaces[i_sbi] = spaceid;
                btypes[i_sbi] = btypeid;
                origins[i_sbi] = i;
                i_sbi++;
            }
            tl[i_tl++] = btypeid;
        } else {
            tl[i_tl++] = btypeid;
        }
    }
    // PP(info, "%s@%i - tlCount=%i", FN_NAME, __LINE__, tlCount);
    // for (i = 1; i <= tlCount; i++) {
    //     PP(info, "    #%i: t%i", i, tl[i]);
    // }

    if (check && spaceCount > 1) {
        // check for space conflicts, i.e. two types with same spaceid but different btypeid and from different input types
        for (i = 0; i < spaceCount; i++) {
            for (j = i; j < spaceCount; j++) {
                if (origins[i] != origins[j] && spaces[i] == spaces[j] && btypes[i] != btypes[j]) {
                    // PP(info, "%s@%i ----------------- SPACE CONFLICT -----------------", FN_NAME, __LINE__);
                    // PP(info, "  intersection of %i types:", types[0]);
                    // for (int i = 1; i <= types[0]; i++) {
                    //     PP(info, "    #%i: t%i", i, types[i]);
                    // }
                    // PP(info, "  tlCount=%i, spaceCount=%i, conflict between i=%i and j=%i", FN_NAME, __LINE__, tlCount, spaceCount, i, j);
                    // for (i = 0; i < spaceCount; i++) {
                    //     PP(info, "    #%i: spaceid=%i, btypeid=%i, origin=%i", i, spaces[i], btypes[i], origins[i]);
                    // }
                    return 0; //setErrAndDesc(0, "Space conflict", __FILE__, __LINE__);      // OPEN: add details
                }
            }
        }
    }

    // sort types into btypeid order
    ks_radix_sort(btypeid_t, tl + 1, tlCount);

    // compact
    int i1 = 1,  i2 = 2,  i3 = tlCount, compactedCount = 1;
    for(; i2 <= i3;) {
        if (tl[i1] != tl[i2]) {
            tl[++i1] = tl[i2++];
            compactedCount++;
        } else {
            // skip duplicates
            while (tl[i1] == tl[i2] && i2 <= i3) i2++;
        }
    }
    tl[0] = compactedCount;
    // PP(info, "%s@%i - compactedCount=%i", FN_NAME, __LINE__, compactedCount);
    // for (int i = 1; i <= compactedCount; i++) {
    //     PP(info, "    tl[%i]: t%i", i, tl[i]);
    // }

    // if just one type return error
    if (compactedCount == 1) return setErrAndDesc(0, "Only one type", __FILE__, __LINE__);

    // OPEN: be a good citizen and zero out the scratch?

    // get the tlid for typelist - adding if missing, returning 0 if invalid
    // PP(info, "%s@%i", FN_NAME, __LINE__);
    idx = hi_put_idx(TM_TLID_BY_TLHASH, tm->tlid_by_tlhash, tl, &outcome);
    switch (outcome) {
        default:
            die("%s: HI_TOMBSTONE1!", FN_NAME);
        case HI_LIVE:
            tlid = tm->tlid_by_tlhash->tokens[idx];
            break;
        case HI_EMPTY:
            tlid = _commit_typelist_buf_at(tm, tl, idx);
            if (!tlid) return _seriousErrorCommitingTypelistBufHandleProperly(0, __FILE__, __LINE__);
    }
    return tlid;
}

pub TM_TLID_T tm_inter_tlid_for(BK_TM *tm, const btypeid_t *types) {
    return tm_inter_tlid_for_impl(tm, types, true);
}

pvt btypeid_t tm_inter_impl(BK_TM *tm, btypeid_t btypeid, btypeid_t *types, bool check) {
    TM_TLID_T tlid;

    // use tm->typelist_buf as scratch so don't have to allocate memory
    if (!btypeid) return _err_invalid_btype_B_NAT(B_NAT, __FILE__, FN_NAME, __LINE__);
    if (!types[0]) return _err_emptyTypelist(B_NAT, __FILE__, FN_NAME, __LINE__);
    if (btypeid >= tm->next_btypeId) return _err_btypeidOutOfRange(B_NAT, __FILE__, FN_NAME, __LINE__, btypeid);

    // PP(info, "%s@%i -------------------------", FN_NAME, __LINE__);
    // PP(info, "    intersection of %i types:", types[0]);
    // for (int i = 1; i <= types[0]; i++) {
    //     PP(info, "    #%i: t%i", i, types[i]);
    // }
    tlid = tm_inter_tlid_for_impl(tm, types, check);
    // if (tlid) {
    //     btypeid_t *tl = tm->typelist_buf + tm->tlrp_by_tlid[tlid];
    //     PP(info, "  typelist - tlid=%i, len=%i", tlid, tl[0]);
    //     for (int i = 1; i <= tl[0]; i++) {
    //         PP(info, "    tl[%i]: t%i", i, tl[i]);
    //     }
    // }
    btypeid = tlid ? tm_inter_for_tlid_or_create(tm, btypeid, tlid) : B_NAT;
    // PP(info, "  return btypeid=t%i", btypeid);
    return btypeid;
}

pub btypeid_t tm_inter(BK_TM *tm, btypeid_t btypeid, btypeid_t *types) {
    return tm_inter_impl(tm, btypeid, types, true);
}

pub btypeid_t tm_inter_get_for_tlid(BK_TM *tm, TM_TLID_T tlid) {
    // use-case here is to check that an intersection doesn't exist before reserving a type
    u32 idx;  i32 outcome;

    idx = hi_put_idx(TM_DETAILID_BY_TLIDHASH, tm->intid_by_tlidhash, tlid, &outcome);
    switch (outcome) {
        default:
            die("%s: HI_TOMBSTONE2!", FN_NAME);
        case HI_LIVE:
            return tm->btypid_by_intid[tm->intid_by_tlidhash->tokens[idx]];
        case HI_EMPTY:
            return B_NAT;
    }
}

pub btypeid_t tm_inter_for_tlid_or_create(BK_TM *tm, btypeid_t btypeid, TM_TLID_T tlid) {
    u32 idx;  i32 i, outcome, tlCount;  TM_DETAILID_T intid;  bool hasT;  btsummary *sum;  btypeid_t *thisTypeList, other;

    // check for space conflicts - must be done here as it is valid to create an intersection typelist that later on
    // has space conflicts

    // PP(info, "tm_inter_for_tlid_or_create - #1");
    thisTypeList = tm->typelist_buf + tm->tlrp_by_tlid[tlid];
    tlCount = (i32) thisTypeList[0];
    // OPEN: make the next two lines atomic - tm_typelist_scratch - which will return a pointer to the memory above the next_tlrp + its size
    _make_next_page_of_typelist_buf_writable_if_necessary(tm, tlCount);
    hasT = false;

    for (i = 1; i <= tlCount; i++) {
        sum = tm->btsummary_by_btypeid + thisTypeList[i];
        hasT = hasT || TM_HAS_T(*sum);
    }
    // OPEN: ensure we have required space for typelist conflict check - for the moment wing it

    // get the btypeid for the tlid
    // PP(info, "tm_inter_for_tlid_or_create - #2");
    idx = hi_put_idx(TM_DETAILID_BY_TLIDHASH, tm->intid_by_tlidhash, tlid, &outcome);
    switch (outcome) {
        default:
            die("%s: HI_TOMBSTONE2!", FN_NAME);
        case HI_LIVE:
            // typelist already exists
            // PP(info, "tm_inter_for_tlid_or_create - #3");
            intid = tm->intid_by_tlidhash->tokens[idx];
            if (btypeid == B_NEW) return tm->btypid_by_intid[intid];
            else if (btypeid == (other = tm->btypid_by_intid[intid])) return btypeid;
            else return _err_otherAlreadyRepresentsTL(B_NAT, __FILE__, __LINE__, btypeid, other);
        case HI_EMPTY:
            // missing so commit the intersection type for tlid
            // PP(info, "tm_inter_for_tlid_or_create - #4");
            if (btypeid == B_NEW) {
                btypeid = tm->next_btypeId;
            } else if (TM_BMT_ID(tm->btsummary_by_btypeid[btypeid]) != bmterr)
                // btypeid is already in use so given the type list lookup above we cannot be referring to the same btype
                return _err_btypeAlreadyInitialised(B_NAT, __FILE__, __LINE__, btypeid);
            intid = tm->next_intid++;
            if (intid >= tm->max_intid) {
                tm->max_intid += TM_MAX_ID_INC_SIZE;
                _growTo((void **) &tm->tlid_by_intid, tm->max_intid * sizeof(TM_TLID_T), tm->mm, FN_NAME);
                _growTo((void **) &tm->btypid_by_intid, tm->max_intid * sizeof(btypeid_t), tm->mm, FN_NAME);
            }
            tm->tlid_by_intid[intid] = tlid;
            btypeid = _update_type_summary(tm, btypeid, intid, 0, hasT);
            tm->btsummary_by_btypeid[btypeid] |= bmtint;
            tm->btypid_by_intid[intid] = btypeid;
            hi_replace_empty(TM_DETAILID_BY_TLIDHASH, tm->intid_by_tlidhash, idx, intid);
            return btypeid;
    }
}

pub btypeid_t tm_interv(BK_TM *tm, btypeid_t btypeid, u32 typesCount, ...) {
    va_list args;  btypeid_t *types;  int i;
    va_start(args, typesCount);
    types = malloc((1 + typesCount) * sizeof(btypeid_t));
    for (i = 1; i <= typesCount; i++) types[i] = va_arg(args, btypeid_t);
    types[0] = typesCount;
    btypeid = tm_inter(tm, btypeid, types);
    free(types);
    va_end(args);
    return btypeid;
}

pub btypeid_t tm_interv_in(BK_TM *tm, btypeid_t btypeid, btypeid_t spaceid, u32 typesCount, ...) {
    va_list args;  btypeid_t *types;  int i;
    va_start(args, typesCount);
    if (spaceid && tm_space_would_deeply_recurse(tm, btypeid, spaceid)) return B_NAT;
    types = malloc((1 + typesCount) * sizeof(btypeid_t));
    for (i = 1; i <= typesCount; i++) types[i] = va_arg(args, btypeid_t);
    types[0] = typesCount;
    btypeid = tm_inter(tm, btypeid, types);
    free(types);
    va_end(args);
    tm->spaceid_by_btypeid[btypeid] = spaceid;
    return btypeid;
}

pub btypeid_t * tm_inter_tl(BK_TM *tm, btypeid_t btypeid) {
    // answer a typelist ptr to the given intersection's types or 0 for error
    btsummary *sum;
    sum = tm->btsummary_by_btypeid + btypeid;
    if (TM_BMT_ID(*sum) == bmtint) {
        u32 detailsid = TM_DETAILS_ID(*sum);  // leave for debugging
        return tm->typelist_buf + tm->tlrp_by_tlid[tm->tlid_by_intid[detailsid]];
    } else {
        return 0;
    }
}

#define BK_INTERSECTION(tm, ...) ({                                                                                     \
    btypeid_t args[] = { __VA_ARGS__ };                                                                                 \
    tm_interv((tm), 0, sizeof(args) / sizeof(args[0]), args);                                                           \
})

#endif  // SRC_BK_TM_INTER_C