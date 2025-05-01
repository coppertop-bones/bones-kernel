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
// UNION IMPLEMENTATION
// KEEPER REQUISITES: core
// ---------------------------------------------------------------------------------------------------------------------

#ifndef __BK_TM_UNION_C
#define __BK_TM_UNION_C "bk/tm.c"


#include "core.c"



pub btypeid_t tm_union(BK_TM *tm, btypeid_t self, btypeid_t *typelist) {
    // answers the validated union type corresponding to typelist, creating if necessary
    TM_TLID_T tlid;

    if (!self) return _err_invalid_btype_B_NAT(B_NAT, __FILE__, FN_NAME, __LINE__);
    if (!typelist[0]) return _err_emptyTypelist(B_NAT, __FILE__, FN_NAME, __LINE__);;
    if (self >= tm->next_btypeId) return _err_selfOutOfRange(B_NAT, __FILE__, FN_NAME, __LINE__, self);;

    tlid = tm_union_tlid(tm, typelist);
    return tlid ? tm_union_for_tlid_or_create(tm, self, tlid) : tlid;
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

pub btypeid_t tm_union_for_tlid_or_create(BK_TM *tm, btypeid_t self, TM_TLID_T tlid) {
    u32 idx;  i32 outcome, i;  TM_DETAILID_T uniid;  btypeid_t *nextTypelist; i32 numTypes;  bool hasT;

    // get self corresponding to the tlid
    idx = hi_put_idx(TM_DETAILID_BY_TLIDHASH, tm->uniid_by_tlidhash, tlid, &outcome);
    switch (outcome) {
        default:
            die("%s: HI_TOMBSTONE2!", FN_NAME);
        case HI_LIVE:
            uniid = tm->uniid_by_tlidhash->tokens[idx];
            if (self == B_NEW) return tm->btypid_by_uniid[uniid];
            else if (self == tm->btypid_by_uniid[uniid]) return self;
            else return B_NAT;
        case HI_EMPTY:
            // missing so commit the union type for tlid
            if (self == B_NEW)
                self = tm->next_btypeId;
            else if (TM_BMT_ID(tm->btsummary_by_btypeid[self]) != bmterr)
                // self is already in use so given the type list lookup above we cannot be referring to the same btype
                return B_NAT;
            uniid = tm->next_uniid++;
            if (uniid >= tm->max_uniid) {
                tm->max_uniid += TM_MAX_ID_INC_SIZE;
                _growTo((void **)&tm->tlid_by_uniid, tm->max_uniid * sizeof(TM_TLID_T), tm->mm, FN_NAME);
                _growTo((void **)&tm->btypid_by_uniid, tm->max_uniid * sizeof(btypeid_t), tm->mm, FN_NAME);
            }
            tm->tlid_by_uniid[uniid] = tlid;
            hasT = false;

            nextTypelist = tm->typelist_buf + tm->tlrp_by_tlid[tlid];
            numTypes = *nextTypelist;
            for (i = 1; i <= numTypes; i++) hasT = hasT || TM_HAS_T(tm->btsummary_by_btypeid[nextTypelist[i]]);
            _update_type_summary(tm, self, uniid, 0, hasT);
            tm->btsummary_by_btypeid[self] |= bmtuni;
            tm->btypid_by_uniid[uniid] = self;
            hi_replace_empty(TM_DETAILID_BY_TLIDHASH, tm->uniid_by_tlidhash, idx, uniid);
            return self;
    }
}

pub TM_TLID_T tm_union_tlid(BK_TM *tm, btypeid_t *typelist) {
    // answers the validated union typelist id corresponding to typelist, creating if necessary
    // very similar to tm_inter_tlid but a little different
    i32 i, j, numTypes, outcome;  btsummary *sum;  TM_TLID_T tlid;  btypeid_t *uniTl, *p1, *p2, *p3, *nextTypelist;
    u32 idx;

    // check typeid is in range, and figure total possible length (including possible duplicate from child unions)
    numTypes = 0;
    for (i = 1; i <= typelist[0]; i++) {
        if (!(TM_FIRST_VALID_BTYPEID <= typelist[i] && typelist[i] < tm->next_btypeId)) return _err_itemInTLOutOfRange(B_NAT, __FILE__, FN_NAME, __LINE__, typelist[i], i);;
        sum = tm->btsummary_by_btypeid + typelist[i];
        if (TM_BMT_ID(*sum) == bmtuni) {
            tlid = tm->tlid_by_uniid[TM_DETAILS_ID(*sum)];
            numTypes += (tm->typelist_buf + tm->tlrp_by_tlid[tlid])[0];
        } else {
            numTypes += 1;
        }
    }

    _make_next_page_of_typelist_buf_writable_if_necessary(tm, numTypes);

    nextTypelist = tm->typelist_buf + tm->next_tlrp;

    // copy typelist into typelist_buf unpacking any unions
    p1 = nextTypelist;
    *p1++ = numTypes;
    for (i = 1; i <= numTypes; i++) {
        sum = tm->btsummary_by_btypeid + typelist[i];
        if (TM_BMT_ID(*sum) == bmtuni) {
            // we have a union type - expand it
            tlid = tm->tlid_by_uniid[TM_DETAILS_ID(*sum)];
            uniTl = (tm->typelist_buf + tm->tlrp_by_tlid[tlid]);
            for (j = 1; j <= (i32) uniTl[0]; j++) *p1++ = uniTl[j];
        } else
            *p1++ = typelist[i];
    }

    // sort types into btypeid order
    ks_radix_sort(btypeid_t, nextTypelist + 1, numTypes);

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
    *nextTypelist = p1 - nextTypelist;

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
    return tlid;
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

pub btypeid_t * tm_union_tl(BK_TM *tm, btypeid_t self) {
    // answer a typelist ptr to the given union's types or 0 for error
    btsummary *sum;
    sum = tm->btsummary_by_btypeid + self;
    if (TM_BMT_ID(*sum) == bmtuni) {
        return tm->typelist_buf + tm->tlrp_by_tlid[tm->tlid_by_uniid[TM_DETAILS_ID(*sum)]];
    } else {
        return 0;
    }
}

#define BK_UNION(tm, ...) ({                                                                                            \
    btypeid_t args[] = { __VA_ARGS__ };                                                                                 \
    _tm_unionv((tm), 0, sizeof(args) / sizeof(args[0]), args);                                                        \
})

#endif  // __BK_TM_UNION_C