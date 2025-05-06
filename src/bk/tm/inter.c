// ---------------------------------------------------------------------------------------------------------------------
//
//                             Copyright (c) 2019-2025 David Briant. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance
// with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software distributed under the License is distributed
// on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for
// the specific language governing permissions and limitations under the License.
//
// ---------------------------------------------------------------------------------------------------------------------


// ---------------------------------------------------------------------------------------------------------------------
// INTERSECTION IMPLEMENTATION
// KEEPER REQUISITES: core
// ---------------------------------------------------------------------------------------------------------------------

#ifndef __BK_TM_INTER_C
#define __BK_TM_INTER_C "bk/tm/inter.c"


#include "core.c"

pub btypeid_t tm_check_inter(BK_TM *tm, btypeid_t btypeid, btypeid_t spaceid) {
    // An existing intersection is being defined a second time check that the attributes don't conflict
    //      a) if current.space is already set then space may be the same or missing
    return btypeid;
}

pub btypeid_t tm_inter(BK_TM *tm, btypeid_t btypeid, btypeid_t *typelist) {
    TM_TLID_T tlid;

    // use tm->typelist_buf as scratch so don't have to allocate memory
    if (!btypeid) return _err_invalid_btype_B_NAT(B_NAT, __FILE__, FN_NAME, __LINE__);
    if (!typelist[0]) return _err_emptyTypelist(B_NAT, __FILE__, FN_NAME, __LINE__);
    if (btypeid >= tm->next_btypeId) return _err_selfOutOfRange(B_NAT, __FILE__, FN_NAME, __LINE__, btypeid);

    tlid = tm_inter_tlid_for(tm, typelist);
    btypeid = tlid ? tm_inter_for_tlid_or_create(tm, btypeid, tlid) : B_NAT;
    PP(info, "tm_inter - btypeid: %i, tlid: %i, len: %i", btypeid, tlid, (tm->typelist_buf + tm->tlrp_by_tlid[tlid])[0]);
    return btypeid;
}

pub TM_TLID_T tm_inter_tlid_for(BK_TM *tm, btypeid_t *typelist) {
    // answers the validated intersection typelist id corresponding to typelist, creating if necessary
    // very similar to tm_union_tlid_for but a little different
    i32 i, j, typelistCount, numTypes, outcome, numRoots;  btsummary *sum;  TM_TLID_T tlid;  u32 idx;  bool hasUnions;
    btypeid_t *interTl, *p1, *p2, *p3, *nextTypelist, *rootBuffer, root;

    // check btypeids in typelist are in range, and figure total possible length (including possible duplicates from child intersections)
//    PP(info, "tm_inter_tlid_for - #1");
    numTypes = 0;  typelistCount = typelist[0];
    for (i = 1; i <= typelistCount; i++) {
        if (!(TM_FIRST_VALID_BTYPEID <= typelist[i] && typelist[i] < tm->next_btypeId)) return _err_itemInTLOutOfRange(B_NAT, __FILE__, FN_NAME, __LINE__, typelist[i], i);
        sum = tm->btsummary_by_btypeid + typelist[i];
        if (TM_BMT_ID(*sum) == bmtint) {
            tlid = tm->tlid_by_intid[TM_DETAILS_ID(*sum)];
            numTypes += (int) (tm->typelist_buf + tm->tlrp_by_tlid[tlid])[0];
        } else {
            numTypes +=1;
        }
    }

    // ensure we have enough space for intersection plus a buffer of same size (for space conflict detection)
    _make_next_page_of_typelist_buf_writable_if_necessary(tm, 1 + numTypes * 2);

    nextTypelist = tm->typelist_buf + tm->next_tlrp;

    // copy typelist into nextTypelist unpacking any intersections
//    PP(info, "tm_inter_tlid_for - #2, numTypes=%i, typelistCount=%i", numTypes, typelistCount);
    p1 = nextTypelist;
    p1++;
    for (i = 1; i <= typelistCount; i++) {
//        PP(info, "tm_inter_tlid_for - #2a typelist[%i]=%i", i, typelist[i]);
        sum = tm->btsummary_by_btypeid + typelist[i];
        if (TM_BMT_ID(*sum) == bmtint) {
            // we have an intersection type - expand it
            tlid = tm->tlid_by_intid[TM_DETAILS_ID(*sum)];
            interTl = (tm->typelist_buf + tm->tlrp_by_tlid[tlid]);
            for (j = 1; j <= (i32) interTl[0]; j++) *p1++ = interTl[j];
        } else
            *p1++ = typelist[i];
    }
    numTypes = p1 - (nextTypelist + 1);

    // sort types into btypeid order
//    PP(info, "tm_inter_tlid_for - #3");
    ks_radix_sort(btypeid_t, nextTypelist + 1, numTypes);

    // compact + check for unions
    p1 = nextTypelist + 1;
    p2 = p1 + 1;
    p3 = p1 + numTypes;
    hasUnions = TM_BMT_ID(tm->btsummary_by_btypeid[*p1]) == bmtuni;
    while (p2 < p3) {
        if (*p1 != *p2)
            *++p1 = *p2++;
        else
            while (*p1 == *p2 && p2 < p3) p2++;
        hasUnions |= TM_BMT_ID(tm->btsummary_by_btypeid[*p1]) == bmtuni;
    }
    nextTypelist[0] = numTypes = p1 - nextTypelist;

    // handle intersections of unions?
    if (hasUnions) return setErrAndDesc(B_NAT, "Has unions", __FILE__, __LINE__);

    // check for space conflicts
    // get root spaceid for each type
    rootBuffer = nextTypelist + numTypes;
    numRoots = 0;
    for (i = 0; i < numTypes; i++) {
        sum = tm->btsummary_by_btypeid + nextTypelist[i];
        if (TM_BMT_ID(*sum) == bmtint || TM_BMT_ID(*sum) == bmtatm) {
            root = tm_root_spaceid(tm, nextTypelist[i]);
            if (root) rootBuffer[++numRoots] = root;
        }
    }
    if (numRoots >= 2) {
        // check for duplicates - bubble sort style check (could sort and check for consecutive ids)
        for (i = 0; i < numRoots; i++) {
            if (rootBuffer[i]) continue;
            for (j = i + 1; j < numRoots; j++) {
                if (rootBuffer[i] == rootBuffer[j]) {
                    return setErrAndDesc(B_NAT, "Space conflict - t%s and t%s have common root t%s\n", __FILE__, __LINE__, nextTypelist[i+1], nextTypelist[j+1], rootBuffer[i]);}
            }
        }
    }

    // if just one type return error
    if (numTypes == 1) return 0;

    // OPEN: be a good citizen and zero out the scratch?


    // get the tlid for the typelist - adding if missing, returning 0 if invalid
//    PP(info, "tm_inter_tlid_for - #4");
    idx = hi_put_idx(TM_TLID_BY_TLHASH, tm->tlid_by_tlhash, nextTypelist, &outcome);
    switch (outcome) {
        default:
            die("%s: HI_TOMBSTONE1!", FN_NAME);
        case HI_LIVE:
            tlid = tm->tlid_by_tlhash->tokens[idx];
            // OPEM: need to check space if given is compatible? here or later? probably later
            break;
        case HI_EMPTY:
            tlid = _commit_typelist_buf_at(tm, nextTypelist, idx);
            if (!tlid) return _seriousErrorCommitingTypelistBufHandleProperly(B_NAT, __FILE__, __LINE__);
    }
    return tlid;
}

pub btypeid_t tm_inter_for_tlid(BK_TM *tm, TM_TLID_T tlid) {
    // use-case here is to check a intersection doesn't exist before reserving a type
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
    u32 idx;  i32 i, outcome, j, k, numTypes;  TM_DETAILID_T intid;  bool hasT;  btsummary *sum;
    btypeid_t *thisTypeList, *conflicts_buf, rootspcid, other;  char const *pp_this, *pp_prior, *pp_root;

    thisTypeList = tm->typelist_buf + tm->tlrp_by_tlid[tlid];
    numTypes = *thisTypeList;
    conflicts_buf = thisTypeList + 1 + numTypes;

    // check for space conflicts
//    PP(info, "tm_inter_for_tlid_or_create - #1");
    hasT = false;
    for (i = 1; i <= numTypes; i++) {
        sum = tm->btsummary_by_btypeid + thisTypeList[i];
        rootspcid = conflicts_buf[k = i - 1] = tm_root_spaceid(tm, thisTypeList[i]);
        for (j = 0; j < k; j++) {
            if (rootspcid && rootspcid == conflicts_buf[j]) {
                pp_this = tm_s8(tm, tm->tp, thisTypeList[i]).cs;
                pp_prior = tm_s8(tm, tm->tp, thisTypeList[j + 1]).cs;     // i starts at 1, j starts at 0
                pp_root = tm_s8(tm, tm->tp, conflicts_buf[j]).cs;
//                PP(info, "Space conflict - %s and %s have common root %s\n", pp_this, pp_prior, pp_root);
                return B_NAT;
            }
        }
        hasT = hasT || TM_HAS_T(*sum);
    }

    // get the btypeid for the tlid
//    PP(info, "tm_inter_for_tlid_or_create - #2");
    idx = hi_put_idx(TM_DETAILID_BY_TLIDHASH, tm->intid_by_tlidhash, tlid, &outcome);
    switch (outcome) {
        default:
            die("%s: HI_TOMBSTONE2!", FN_NAME);
        case HI_LIVE:
            // typelist already exists
//            PP(info, "tm_inter_for_tlid_or_create - #3");
            intid = tm->intid_by_tlidhash->tokens[idx];
            if (btypeid == B_NEW) return tm->btypid_by_intid[intid];
            else if (btypeid == (other = tm->btypid_by_intid[intid])) return btypeid;
            else return _err_otherAlreadyRepresentsTL(B_NAT, __FILE__, __LINE__, btypeid, other);
        case HI_EMPTY:
            // missing so commit the intersection type for tlid
//            PP(info, "tm_inter_for_tlid_or_create - #4");
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
            _update_type_summary(tm, btypeid, intid, 0, hasT);
            tm->btsummary_by_btypeid[btypeid] |= bmtint;
            tm->btypid_by_intid[intid] = btypeid;
            hi_replace_empty(TM_DETAILID_BY_TLIDHASH, tm->intid_by_tlidhash, idx, intid);
            return btypeid;
    }
}

pub btypeid_t tm_inter_get_or_create_for_tlid(BK_TM *tm, TM_TLID_T tlid) {
    u32 idx;  i32 i, outcome, j, k, numTypes;  bool hasT;  btsummary *sum;
    btypeid_t *thisTypeList, *conflicts_buf, rootspcid;  char const *pp_this, *pp_prior, *pp_root;

    thisTypeList = tm->typelist_buf + tm->tlrp_by_tlid[tlid];
    numTypes = *thisTypeList;
    conflicts_buf = thisTypeList + 1 + numTypes;

    // check for orthogonal space conflicts
    hasT = false;
    for (i = 1; i <= numTypes; i++) {
        sum = tm->btsummary_by_btypeid + thisTypeList[i];
        rootspcid = conflicts_buf[k = i - 1] = tm_root_spaceid(tm, thisTypeList[i]);
        for (j = 0; j < k; j++) {
            if (rootspcid && rootspcid == conflicts_buf[j]) {
                pp_this = tm_s8(tm, tm->tp, thisTypeList[i]).cs;
                pp_prior = tm_s8(tm, tm->tp, thisTypeList[j + 1]).cs;     // i starts at 1, j starts at 0
                pp_root = tm_s8(tm, tm->tp, conflicts_buf[j]).cs;
                printf("Space conflict - %s and %s have common root %s\n", pp_this, pp_prior, pp_root);
                return 0;
            }
        }
        hasT = hasT || TM_HAS_T(*sum);
    }

    // get the btypeid for the tlid
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

pub btypeid_t tm_interv(BK_TM *tm, btypeid_t btypeid, u32 numTypes, ...) {
    va_list args;  btypeid_t *typelist;  int i;
    va_start(args, numTypes);
    typelist = malloc((1 + numTypes) * sizeof(btypeid_t));
    for (i = 1; i <= numTypes; i++) typelist[i] = va_arg(args, btypeid_t);
    typelist[0] = numTypes;
    btypeid = tm_inter(tm, btypeid, typelist);
    free(typelist);
    va_end(args);
    return btypeid;
}

pub btypeid_t tm_interv_in(BK_TM *tm, btypeid_t btypeid, btypeid_t spaceid, u32 numTypes, ...) {
    va_list args;  btypeid_t *typelist;  int i;
    va_start(args, numTypes);
    if (spaceid && tm_space_would_recurse(tm, btypeid, spaceid)) return B_NAT;
    typelist = malloc((1 + numTypes) * sizeof(btypeid_t));
    for (i = 1; i <= numTypes; i++) typelist[i] = va_arg(args, btypeid_t);
    typelist[0] = numTypes;
    btypeid = tm_inter(tm, btypeid, typelist);
    free(typelist);
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

#endif  // __BK_TM_INTER_C