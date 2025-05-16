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


// nomenclature
// types is the length prefixed list of types being intersected
// tl is the length prefixed list that is used for internally representing the types in an intersection (union or tuple etc)
// as an implementation feature tl may be longer than types (dues to expansion) and may contain types with conflicting spaces
// tlid is the id corresponding to a tl

// so we have a conumdrum

// const: atom in consty
// mut: const & mut in consty
// for mut <: const and mut <: mut
// we need tl(mut) => (const, mut)

// fred: atom
// joe: mut & fred
//
// for joe <: mut and joe <: fred and joe <: const
// we either need
// a) tl(mut) => (mut, joe) and unpack mut when doing <:, or,
// b) tl(mut) => (const, mut, fred) in which case we just partition without unpacking
//
// however
// sally: atom
// arthur: joe & sally
// but for tl(joe) & sally we either have
// a) => (const, mut, fred, sally) which has a space conflict (since mut and const are both in consty), or,
// b) => (mut, fred, sally) which doesn't have a space conflict but does need checking / unpacking
// tl(joe) => const, mut, fred

// or we figure another function types(tl(joe)) => (mut, fred, sally) which we only need when intersecting
// two passes over the tl - get spaces for recursive intersections

// we only need to worry about tls of recursive intersections since they are the only ones that can conflict with
// their parents


// const: atom in consty
// mut: fred & const & mut in consty
// mut2: joe & mut & mut2 in consty

// tl(mut2) => (fred, joe, const, mut', mut2') mut removes const and mut2 removes mut
// could keep the definition of recursive intersections - how? by noting whenever a type in types has a childSpaceId that
// equals the parent space id, and keeping an additional unpacked tl - do we have any spare flags for HAS_UNPACKED_TL?

// can we just we ignore conflicts in consty?

// so by allowing in to be done after the recursive intersection (so we can say a subtype is orthogonal to its parent) we create
// excessive conflicts later on

// does the fact we created the tl tell us anything - for mut we know that the tl is valid

// do we need mut & const to be illegal? why not just let mut & const => mut?

// mem in mem
// f64: f64 & mem
// i32: i32 & mem
// f64 & i32 => f64 & mem & i32

// this we need to use
// f64: f64 & mem in mem
// i32: i32 & mem in mem
// for f64 & i32 to be illegal

// nat: i32 & nat in mem

// since we desire nat to be a subtype of i32


// when we unpack mut in mut: joe & mut & mut2 in consty
// we ignore the consty space?
//
// when we unpack mut in mut: joe & mut & mut2 in fred
// mut in consty & const in ignored
//
// so when intersecting types we ignore the spaces of any children types that are the same as the parent type

// does this work?????
// so we cannot inherently space check a tl but only when intersecting types, when we make the space if childSpaceId == parentSpaceId
// do we ever need to create a tl without the orginiating types - minus is a candidate, what about intersect in the
// type inference process?


// do we check the summary for TM_IN_SPACE_MASK before looking up in spaceid_by_btypeid? will this actually be more
// cache efficient? (hopefully branch prediction will predict not in space).



typedef struct {
    btypeid_t spaceid;
    btypeid_t btypeid;
} spaceid_btypeid_t;

// utility fns
pvt void _sortBySpaceIdThenBtypeId(spaceid_btypeid_t *, int);
// pvt int _compactTypes(btypeid_t *p1, int N);


pub btypeid_t tm_check_inter(BK_TM *tm, btypeid_t btypeid, btypeid_t spaceid) {
    // An existing intersection is being defined a second time check that the attributes don't conflict
    //      a) if current.space is already set then space may be the same or missing
    return btypeid;
}

pub btypeid_t tm_inter(BK_TM *tm, btypeid_t btypeid, btypeid_t *types) {
    TM_TLID_T tlid;

    // use tm->typelist_buf as scratch so don't have to allocate memory
    if (!btypeid) return _err_invalid_btype_B_NAT(B_NAT, __FILE__, FN_NAME, __LINE__);
    if (!types[0]) return _err_emptyTypelist(B_NAT, __FILE__, FN_NAME, __LINE__);
    if (btypeid >= tm->next_btypeId) return _err_btypeidOutOfRange(B_NAT, __FILE__, FN_NAME, __LINE__, btypeid);

    // for (int i = 1; i <= types[0]; i++) {
    //     PP(info, "tm_inter #1 types[%i]: t%i", i, types[i]);
    // }
    tlid = tm_inter_tlid_for(tm, types);
    // PP(info, "tm_inter #2 tlid=%i", tlid);
    // if (tlid) {
    //     btypeid_t *tl = tm->typelist_buf + tm->tlrp_by_tlid[tlid];
    //     for (int i = 0; i <= tl[0]; i++) {
    //         PP(info, "tm_inter #3 tl[%i]: t%i", i, tl[i]);
    //     }
    // }

    btypeid = tlid ? tm_inter_for_tlid_or_create(tm, btypeid, tlid) : B_NAT;
//    PP(info, "tm_inter - btypeid: %i, tlid: %i, len: %i", btypeid, tlid, (tm->typelist_buf + tm->tlrp_by_tlid[tlid])[0]);
    return btypeid;
}

pub bool inter_types_has_conflict(BK_TM *tm, const btypeid_t *types) {
    // PP(info, "inter_types_has_conflict);

    // algo
    // for each type in types:
    //     expand children if type is except when the child is in the same space (because would automatically fail)
    // sort by spaceid then by btypeid
    // check for conflicts - i.e. each spaceid has max one distinct btypeid

    // collate spaceid and btypeid from the types being intersected, expanding intersections but not including any
    // children in the same space as the parent intersection
    spaceid_btypeid_t *vec;  int vecSize = 0;  int typesCount=types[0];  btsummary *sum;
    vec = (spaceid_btypeid_t *) tm->typelist_buf + tm->next_tlrp;     // we can just use the memory for the next (uncommitted) types
    for (int i = 1; i <= typesCount; i++) {
        btypeid_t parentId = types[i];
        btypeid_t parentSpaceId = tm->spaceid_by_btypeid[parentId];
        if (parentSpaceId != B_NAT) {
            // PP(info, "tm_inter_tlid_for - t%i=%i, root=%i", i, types[i], *p3);
            vec[vecSize].btypeid = parentId;
            vec[vecSize].spaceid = parentSpaceId;
            // PP(info, "#%i: parent spaceid=%i, btypeid=%i", vecSize, vec[vecSize].spaceid, vec[vecSize].btypeid);
            vecSize++;
        }
        sum = tm->btsummary_by_btypeid + parentId;
        if (TM_BMT_ID(*sum) == bmtint) {
            TM_TLID_T tlid = tm->tlid_by_intid[TM_DETAILS_ID(*sum)];
            btypeid_t *childTl = tm->typelist_buf + tm->tlrp_by_tlid[tlid];
            for (int j = 1; j <= (i32) childTl[0]; j++) {
                btypeid_t childId = childTl[i];
                btypeid_t childSpaceId = tm->spaceid_by_btypeid[childId];
                if (childSpaceId != B_NAT && childSpaceId != parentSpaceId) {
                    // PP(info, "tm_inter_tlid_for - t%i_%i=%i, root=%i", i, j, childTl[j], *p3);
                    vec[vecSize].spaceid = childSpaceId;
                    vec[vecSize].btypeid = childId;
                    // PP(info, "#%i: child spaceid=%i, btypeid=%i", vecSize, vec[vecSize].spaceid, vec[vecSize].btypeid);
                    vecSize++;
                }
            }
        }
    }

    if (vecSize) {
        // sort by spaceid then btypeid
        _sortBySpaceIdThenBtypeId(vec, vecSize);
        // for (btypeid_t *p = p1; p < p3; p++) {
        //     PP(info, "tm_inter_tlid_for - all sorted: %i", *p);
        // }

        // check for conflicts - i.e. each spaceid has max one distinct btypeid
        btypeid_t spaceid = vec[0].spaceid;
        btypeid_t btypeid = vec[0].btypeid;
        for (int i = 1; i < vecSize; i++) {
            if (vec[i].spaceid == B_NAT) continue;
            if (vec[i].spaceid != spaceid) {
                spaceid = vec[i].spaceid;
                btypeid = vec[i].btypeid;
            } else {
                if (btypeid != vec[i].btypeid) {
                    // for (int j = 0; j < vecSize; j++) {
                    //     PP(info, "#%i: spaceid=%i, btypeid=%i", j, vec[j].spaceid, vec[j].btypeid);
                    // }
                    return setErrAndDesc(true, "Space conflict", __FILE__, __LINE__);
                }
            }
        }
    }
    return false;
}


pub TM_TLID_T tm_inter_tlid_for(BK_TM *tm, btypeid_t *types) {
    // answers the validated intersection tlid corresponding to types, creating if necessary
    // algo
    // for each type in types:
    //     expand children dropping space if same as parent (because would automatically fail)
    // sort by spaceid then by btypeid
    // check for conflicts - i.e. each spaceid has max one distinct btypeid
    // sort by btypeid
    // compact

    int typesCount=(int)types[0], tlCount=0, outcome, spaceCount=0, k;  btsummary *sum;  spaceid_btypeid_t *vec;
    TM_TLID_T tlid;  u32 idx;  bool hasUnions;  btypeid_t *p1, *p2, *p3, *tl;

    // check btypeids in types are in range, and figure total possible length (including possible duplicates from child intersections)
    // PP(info, "tm_inter_tlid_for - #1");
    for (int i = 1; i <= typesCount; i++) {
        if (!(TM_FIRST_VALID_BTYPEID <= types[i] && types[i] < tm->next_btypeId)) return _err_itemInTLOutOfRange(0, __FILE__, FN_NAME, __LINE__, types[i], i);
        sum = tm->btsummary_by_btypeid + types[i];
        if (TM_BMT_ID(*sum) != bmtint) {
            tlCount++;
        } else {
            tlid = tm->tlid_by_intid[TM_DETAILS_ID(*sum)];
            btypeid_t *childTl = tm->typelist_buf + tm->tlrp_by_tlid[tlid];
            tlCount += (int) childTl[0];
            tlCount++;  // may be an intersection in a space
        }
    }
    if (tlCount <= 1) return setErrAndDesc(0, "Not enough types to create intersection", __FILE__, __LINE__);

    // ensure we have enough space for intersection plus a buffer of twice the same size for the spaceid_btypeid_t vec
    _make_next_page_of_typelist_buf_writable_if_necessary(tm, 1 + tlCount * 3);

    tl = tm->typelist_buf + tm->next_tlrp;
    vec = (spaceid_btypeid_t *) (tm->typelist_buf + tm->next_tlrp + 1 + tlCount);     // we can just use the memory for the next (uncommitted) types
    
    // collate spaceid and btypeid from the types being intersected, expanding intersections but not including any
    // children in the same space as the parent intersection
    k = 0;
    for (int i = 1; i <= typesCount; i++) {
        btypeid_t parentId = types[i], parentSpaceId = B_NAT;
        sum = tm->btsummary_by_btypeid + parentId;
        bool inSpace = TM_IN_SPACE(*sum) && (parentSpaceId = tm_root_spaceid(tm, parentId));
        bool isRec = TM_IS_RECURSIVE(*sum);
        if (TM_BMT_ID(*sum) != bmtint) {
            if (inSpace) spaceCount++;
            vec[k].btypeid = parentId;
            vec[k++].spaceid = parentSpaceId;
            // PP(info, "#%i: other spaceid=%i, btypeid=%i", i, parentSpaceId, parentId);
        } else {
            if (inSpace && isRec) {
                vec[k].btypeid = parentId;
                vec[k++].spaceid = parentSpaceId;
                // PP(info, "#%i: recursive parent spaceid=%i, btypeid=%i", i, parentSpaceId, parentId);
                spaceCount++;
            }
            TM_TLID_T childTlid = tm->tlid_by_intid[TM_DETAILS_ID(*sum)];
            btypeid_t *childTl = tm->typelist_buf + tm->tlrp_by_tlid[childTlid];
            for (int j = 1; j <= (i32) childTl[0]; j++) {
                btypeid_t childId = childTl[j], childSpaceId = B_NAT;
                vec[k].btypeid = childId;
                bool childInSpace = TM_IN_SPACE(*(tm->btsummary_by_btypeid + childId)) && (childSpaceId = tm_root_spaceid(tm, childId));
                // bool childIsRec = TM_IS_RECURSIVE(*(tm->btsummary_by_btypeid + childId));
                if (childInSpace && childSpaceId == parentSpaceId) {
                    vec[k++].spaceid = B_NAT;
                    // PP(info, "#%i_%i: bmtint spaceid=%i, btypeid=%i", i, j, B_NAT, childId);
                } else {
                    spaceCount++;
                    vec[k++].spaceid = childSpaceId;
                    // PP(info, "#%i_%i: bmtint spaceid=%i, btypeid=%i", i, j, childSpaceId, childId);
                }
            }
        }
    }

    if (spaceCount > 1) {
        // sort then check for conflicts - i.e. each spaceid has max one distinct btypeid
        _sortBySpaceIdThenBtypeId(vec, k);

        btypeid_t spaceid = vec[0].spaceid;
        btypeid_t btypeid = vec[0].btypeid;
        for (int i = 1; i < k; i++) {
            if (vec[i].spaceid == B_NAT) continue;
            if (vec[i].spaceid != spaceid) {
                spaceid = vec[i].spaceid;
                btypeid = vec[i].btypeid;
            } else {
                if (btypeid != vec[i].btypeid) {
                    // for (int j = 0; j < k; j++) {
                    //     PP(info, "#%i: spaceid=%i, btypeid=%i", j, vec[j].spaceid, vec[j].btypeid);
                    // }
                    return setErrAndDesc(0, "Space conflict", __FILE__, __LINE__);
                }
            }
        }
    }

    // copy types into tl unpacking any intersections
    // PP(info, "tm_inter_tlid_for - #2, tlCount=%i, typesCount=%i", tlCount, typesCount);
    for (int i = 1; i <= k; i++) tl[i] = vec[i-1].btypeid;

    // sort types into btypeid order
    // PP(info, "tm_inter_tlid_for - #3");
    ks_radix_sort(btypeid_t, tl + 1, k);

    // compact + check for unions
    p1 = tl + 1;
    p2 = p1 + 1;
    p3 = p1 + tlCount + 1;
    int compactedCount = 0;
    hasUnions = TM_BMT_ID(tm->btsummary_by_btypeid[*p1]) == bmtuni;
    while (p2 < p3) {
        if (*p1 != *p2) {
            *++p1 = *p2++;
            compactedCount++;
        }
        else
            while (*p1 == *p2 && p2 < p3) p2++;
        hasUnions |= TM_BMT_ID(tm->btsummary_by_btypeid[*p1]) == bmtuni;
    }
    // handle intersections of unions?
    if (hasUnions) return setErrAndDesc(0, "Has unions", __FILE__, __LINE__);

    tl[0] = compactedCount;

    // if just one type return error
    if (compactedCount == 1) return setErrAndDesc(0, "Only one type", __FILE__, __LINE__);

    // OPEN: be a good citizen and zero out the scratch?

    // get the tlid for typelist - adding if missing, returning 0 if invalid
    // PP(info, "tm_inter_tlid_for - #4");
    idx = hi_put_idx(TM_TLID_BY_TLHASH, tm->tlid_by_tlhash, tl, &outcome);
    switch (outcome) {
        default:
            die("%s: HI_TOMBSTONE1!", FN_NAME);
        case HI_LIVE:
            tlid = tm->tlid_by_tlhash->tokens[idx];
            // OPEM: need to check space if given is compatible? here or later? probably later
            break;
        case HI_EMPTY:
            tlid = _commit_typelist_buf_at(tm, tl, idx);
            if (!tlid) return _seriousErrorCommitingTypelistBufHandleProperly(0, __FILE__, __LINE__);
    }
    return tlid;
}

pub TM_TLID_T tm_inter_tlid_for2(BK_TM *tm, btypeid_t *types) {
    // answers the validated intersection typelist id corresponding to types, creating if necessary
    // very similar to tm_union_tlid_for but a little different
    i32 typesCount=(int)types[0], tlCount=0, outcome, spaceCount=0;  btsummary *sum;
    TM_TLID_T tlid;  u32 idx;  bool hasUnions;  btypeid_t *p1, *p2, *p3, *nextTypelist;

    // check btypeids in types are in range, and figure total possible length (including possible duplicates from child intersections)
    // PP(info, "tm_inter_tlid_for - #1");
    for (int i = 1; i <= typesCount; i++) {
        btypeid_t parentSpace, childSpace;
        if (!(TM_FIRST_VALID_BTYPEID <= types[i] && types[i] < tm->next_btypeId)) return _err_itemInTLOutOfRange(0, __FILE__, FN_NAME, __LINE__, types[i], i);
        sum = tm->btsummary_by_btypeid + types[i];
        if ((parentSpace = tm_root_spaceid(tm, types[i]))) spaceCount++;
        if (TM_BMT_ID(*sum) == bmtint) {
            tlid = tm->tlid_by_intid[TM_DETAILS_ID(*sum)];
            btypeid_t *childTl = tm->typelist_buf + tm->tlrp_by_tlid[tlid];
            for (int j = 1; j <= childTl[0]; j++) {
                childSpace = tm_root_spaceid(tm, childTl[j]);
                if (!(parentSpace != B_NAT && childSpace != parentSpace)) {
                    tlCount++;
                    spaceCount++;
                }
            }
        } else {
            // PP(info, "TM_BMT_ID(*sum): %i", TM_BMT_ID(*sum) >> 28);
            tlCount++;
        }
    }

    // ensure we have enough space for intersection plus a buffer of same size (for space conflict detection)
    _make_next_page_of_typelist_buf_writable_if_necessary(tm, 1 + tlCount * 2);
    _make_next_page_of_typelist_buf_writable_if_necessary(tm, spaceCount * 2);

    nextTypelist = tm->typelist_buf + tm->next_tlrp;

    if (spaceCount > 1 && inter_types_has_conflict(tm, types)) {
        // for (int i = 1; i <= typesCount; i++) {
        //     PP(info, "conflicts %i: t%i", i, types[i]);
        // }
        return setErrAndDesc(0, "Space conflict", __FILE__, __LINE__);
    }

    // copy types into nextTypelist unpacking any intersections
    // PP(info, "tm_inter_tlid_for - #2, tlCount=%i, typesCount=%i", tlCount, typesCount);
    p1 = nextTypelist;
    p1++;
    for (int i = 1; i <= typesCount; i++) {
        btypeid_t parentSpace, childSpace;
        // PP(info, "tm_inter_tlid_for - #2a types[%i]=%i", i, types[i]);
        sum = tm->btsummary_by_btypeid + types[i];
        if (TM_BMT_ID(*sum) == bmtint) {
            // we have an intersection type - expand it
            tlid = tm->tlid_by_intid[TM_DETAILS_ID(*sum)];
            btypeid_t *childTl = tm->typelist_buf + tm->tlrp_by_tlid[tlid];
            parentSpace = tm_root_spaceid(tm, types[i]);
            for (int j = 1; j <= (i32) childTl[0]; j++) {
                childSpace = tm_root_spaceid(tm, childTl[j]);
                if (!(parentSpace != B_NAT && childSpace != parentSpace)) {
                    *p1++ = childTl[j];
                }
            }
        } else
            *p1++ = types[i];
    }
    tlCount = (int)(p1 - (nextTypelist + 1));

    // sort types into btypeid order
    // PP(info, "tm_inter_tlid_for - #3");
    ks_radix_sort(btypeid_t, nextTypelist + 1, tlCount);

    // compact + check for unions
    p1 = nextTypelist + 1;
    p2 = p1 + 1;
    p3 = p1 + tlCount;
    hasUnions = TM_BMT_ID(tm->btsummary_by_btypeid[*p1]) == bmtuni;
    while (p2 < p3) {
        if (*p1 != *p2)
            *++p1 = *p2++;
        else
            while (*p1 == *p2 && p2 < p3) p2++;
        hasUnions |= TM_BMT_ID(tm->btsummary_by_btypeid[*p1]) == bmtuni;
    }
    nextTypelist[0] = tlCount = p1 - nextTypelist;

    // handle intersections of unions?
    if (hasUnions) return setErrAndDesc(0, "Has unions", __FILE__, __LINE__);

    // if just one type return error
    if (tlCount == 1) return 0;

    // OPEN: be a good citizen and zero out the scratch?

    // get the tlid for typelist - adding if missing, returning 0 if invalid
    // PP(info, "tm_inter_tlid_for - #4");
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
            if (!tlid) return _seriousErrorCommitingTypelistBufHandleProperly(0, __FILE__, __LINE__);
    }
    return tlid;
}

pub btypeid_t tm_inter_get_for_tlid(BK_TM *tm, TM_TLID_T tlid) {
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

// we have mut & const <: const or rather mut: subconst & const <: const
// blue & const <: const
// blue & mut = blue & subconst & const

// then we are saying we want mut & const to be NAT - our purpose in that is???

// what is more important is that mut cannot be used by itself?
// #define TM_IN_SPACE_MASK        0x04000000      /* optimisation to not lookup spaceid_by_btypeid - could add a DO_NOT_EXPAND flag here */
// but that means the partition algo needs to be aware that a tl may not be complete, e.g. f64 & mut <: f64 & const would need the mut expanding

// we want to prevent accidentally allowing something to be inferred as mut when it should be const
// e.g.
//
// __update__: {[values: constptr & (N** const & f64), i: index, x: f64] -> constptr & const & f64 ...}
// __update__: {[values: constptr & (N** mut &f64), i: index, x: f64] -> constptr & const & f64 ...}
//
// fred{[values: constptr & const & f64, i: index, x: f64] -> constptr & const & f64
//     values[i]: x     <<<< values could be an immutable or a mutable update fn
//     ^ values
// }
// with implicit const
// set{[values, i: index, x: f64] -> constptr & const & f64
//     values[i]: x     <<<< values could be an immutable or a mutable update fn
//     ^ values
// }

// fred
//     fn1:{[text: const & txt] fn2(text)}
//     fn2: {[text: const & txt] upper(text)}
//     fn2: {[text: mut & txt] upper(text)}


// what we don't want to do is accidentally make something mut when it should be const

    // fred: mut & f64
    // ...
    // sally: fred & cm
    //
    // x: N ** sally
    // x[1]: 1
    // y: x
    // x[1]: 2
    //
    // normally y[1] would be 1

    // CONCLUSION - I can't easily think of a mechanism to enforce mut & const is NAT **and** mut <: const
    //
    // what about a special case? the problem is wanting to have tl(mut) = (mut, const) for subtyping, i.e. <:, and
    // wanting mut & f64 to use tl(mut) for construction
    //
    // const: mut + const ? but that requires runtime dispatch all over the place
    //
    // the DO_NOT_EXPAND requires that the partition function does the expansion
    //
    // the double tl (a definitional one and a partition one) with a parent space and child space check might work but
    // it may contain weird edge cases
    //
    // construction is allowed to be slow (ish) as long as inference is fast enough
    // <: must be fast (but we do cache the results)
    //
    // flag in typelistid could indication a definitional tl - too much work for now?

    // for do not expect space assignment post creation will make too much sense

    // leave the spaceid_btypeid vec for now but could be simplified to just a btypeid_t sort, compact and check

    // may change
    // might change
    // won't change
    //
    // must not be affected (const)    might affect (mut)
    // may be affected (mut)           won't affect (const)
    //
    // might affect
    // won't affect
    // affectable or unaffectable
    //
    // mut = affectable or unaffectable
    // const = unaffectable
    //
    // mut or const => mut
    //
    // mut and const => const
    //
    // only need dynamic dispatch when we have a heterogenous input
    // mut or const is a homogenous union
    //
    // txt: isin + name + cusip + label + ....

    // we can conceive of some machinery to disallow const & mut however I'm not sure how useful it is so for the moment,
    // although it grates, leave it til later

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
    // if (inter_types_has_conflict(tm, thisTypeList)) {
    //     return B_NAT;
    // }

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

// OPEN: unused, delete?
// pub btypeid_t tm_inter_get_or_create_for_tlid(BK_TM *tm, TM_TLID_T tlid) {
//     u32 idx;  i32 i, outcome, j, k, tlCount;  bool hasT;  btsummary *sum;
//     btypeid_t *thisTypeList, *conflicts_buf, rootspcid;  char const *pp_this, *pp_prior, *pp_root;
//
//     thisTypeList = tm->typelist_buf + tm->tlrp_by_tlid[tlid];
//     tlCount = *thisTypeList;
//     conflicts_buf = thisTypeList + 1 + tlCount;
//
//     // check for orthogonal space conflicts
//     hasT = false;
//     for (i = 1; i <= tlCount; i++) {
//         sum = tm->btsummary_by_btypeid + thisTypeList[i];
//         rootspcid = conflicts_buf[k = i - 1] = tm_root_spaceid(tm, thisTypeList[i]);
//         for (j = 0; j < k; j++) {
//             if (rootspcid && rootspcid == conflicts_buf[j]) {
//                 pp_this = tm_s8(tm, tm->tp, thisTypeList[i]).cs;
//                 pp_prior = tm_s8(tm, tm->tp, thisTypeList[j + 1]).cs;     // i starts at 1, j starts at 0
//                 pp_root = tm_s8(tm, tm->tp, conflicts_buf[j]).cs;
//                 printf("Space conflict - %s and %s have common root %s\n", pp_this, pp_prior, pp_root);
//                 return 0;
//             }
//         }
//         hasT = hasT || TM_HAS_T(*sum);
//     }
//
//     // get the btypeid for the tlid
//     idx = hi_put_idx(TM_DETAILID_BY_TLIDHASH, tm->intid_by_tlidhash, tlid, &outcome);
//     switch (outcome) {
//         default:
//             die("%s: HI_TOMBSTONE2!", FN_NAME);
//         case HI_LIVE:
//             return tm->btypid_by_intid[tm->intid_by_tlidhash->tokens[idx]];
//         case HI_EMPTY:
//             return B_NAT;
//     }
// }

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

// pvt int _compactTypes(btypeid_t *p, int N) {
//     btypeid_t *p2, *p3;  int n = 0;
//     p2 = p + 1;
//     p3 = p + N;
//     if (*p) n++;            // count the first btype if it is not B_NAT
//     while (p2 < p3) {
//         if (*p != *p2) {
//             *++p = *p2++;
//             n++;
//         } else {
//             while (*p == *p2 && p2 < p3) p2++;
//         }
//     }
//     return n;
// }

#define _spaceid_btypeid_key(x) ((long)(x).spaceid << 32 | (x).btypeid)

pvt void _sortBySpaceIdThenBtypeId(spaceid_btypeid_t *beg, int count) {
    spaceid_btypeid_t *end, *i, *j, tmp;
    end = beg + count;                                                                                                  \
    for (i = beg + 1; i < end; ++i)                                                                                     \
        if (_spaceid_btypeid_key(*i) < _spaceid_btypeid_key(*(i - 1))) {                                                \
            tmp = *i;                                                                                                   \
            for (j = i; j > beg && _spaceid_btypeid_key(tmp) < _spaceid_btypeid_key(*(j-1)); --j)                       \
                *j = *(j - 1);                                                                                          \
            *j = tmp;                                                                                                   \
        }                                                                                                               \
}



#endif  // __BK_TM_INTER_C