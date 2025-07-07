// ---------------------------------------------------------------------------------------------------------------------
// Copyright 2025 David Briant, https://github.com/coppertop-bones. Licensed under the Apache License, Version 2.0
//
// TM - TYPE MANAGER
// ---------------------------------------------------------------------------------------------------------------------

#ifndef BK_TM_H
#define BK_TM_H "bk/tm.h"

#include "bk.h"
#include "sm.h"
#include "tp.h"
#include "lib/hi.h"


typedef u32 BTYPEID_T_TYPE;         // 20 bits for 1M types
#include "tm_btypeid_t_enum.h"

typedef u32 TM_TLID_T;              // typelist id - length prefixed array of btypeid, i.e. a btypeid_t *
typedef u32 TM_DETAILID_T;          // the id of the details of a type, e.g. the element types in a tuple etc


#define TM_RP_BY_TLID_INC_SIZE (_16K / sizeof(RP))  /* relative pointer by typelist id buffer increment size */


#if defined _APPLE_ || defined __MACH__
typedef u16 BTSIZE_TYPE;            // restrict the known size of memory represented by a btype to be 64k
typedef enum : BTSIZE_TYPE {
#else
typedef enum {
#endif
    NOSZ = 0,
} btypesize_t;


// bmetatypeid_t - 4 bits - 15 metatypes plus the err / uninitialized type
#if defined _APPLE_ || defined __MACH__
typedef enum : u32 {
#else
typedef enum {
#endif
    bmterr = 0x00000000,
    bmtatm = 0x10000000, // atomic type with a given name

    // relations
    bmtint = 0x20000000, // intersection - sorted list of other types
    bmtuni = 0x30000000, // union - sorted list of other types

    // product types (statically known size)
    bmttup = 0x40000000, // tuple - ordered list of other types
    bmtstr = 0x50000000, // struct - ordered and named list of other types
    bmtrec = 0x60000000, // record - unordered named list of other types

    // arrows - aka exponentials - variable length, elements all the same
    bmtseq = 0x70000000, // sequence - tElement
    bmtmap = 0x80000000, // map / dictionary - tKey, tValue
    bmtfnc = 0x90000000, // function - argnames, tArgs, tRet, tFunc, num args

    // schemas
    bmtsvr = 0xA0000000,// schema variable

    // inference metatypes? 11 - 15

} bmetatypeid_t;


// OM_BTYPEID_MASK is 20 bits - 1,048,576 types. We can possibly drop a bit from the DETAILS_ID allowing an extra
// btsummary flag or drop a flag from the object header doubling the number of types.

// btsummary - compressed into a u32
typedef u32 btsummary;      // XXXX FFFF FFFF YYYY YYYY YYYY YYYY YYYY - X is metatypeid, F is flag, Y is detailsid

#define TM_BMTID_MASK           0xF0000000      /* 4-bits allowing up to 15 metatypes */
#define TM_DETAILS_ID_MASK      0x000FFFFF      /* 20-bits at least up to 1,048,576 of each of the meta-types */
#define TM_HAS_T_MASK           0x08000000
#define TM_IN_SPACE_MASK        0x04000000      /* optimisation to not lookup spaceid_by_btypeid */
#define TM_IS_EXPLICIT_MASK     0x02000000
#define TM_IS_RECURSIVE_MASK    0x01000000      /* used in PP and <: to be aware that depth awareness / control is required */
#define TM_IS_MEM_MASK          0x00800000
#define TM_IS_VARIABLE_MASK     0x00400000      /* replicated in object header for faster MM */
#define TM_IS_PTR_MASK          0x00200000      /* replicated in object header for faster MM does not require a type look up when tracing */
#define TM_HAS_PTR_MASK         0x00100000      /* replicated in object header for faster MM requires a type look up when tracing */


#define TM_BMT_ID(x) ((x) & TM_BMTID_MASK)
#define TM_DETAILS_ID(x) ((x) & TM_DETAILS_ID_MASK)
#define TM_HAS_T(x) ((x) & TM_HAS_T_MASK)
#define TM_IN_SPACE(x) ((x) & TM_IN_SPACE_MASK)
#define TM_IS_RECURSIVE(x) ((x) & TM_IS_RECURSIVE_MASK)
#define TM_IS_MEM(x) ((x) & TM_IS_MEM_MASK)
#define TM_IS_EXPLICIT(x) ((x) & TM_IS_EXPLICIT_MASK)


typedef struct {
    union {
        btypeid_t tArgs;
        btypeid_t tK;
        btypeid_t t1;
    };
    union {
        btypeid_t tRet;
        btypeid_t tV;
        btypeid_t t2;
    };
} TM_T1T2;

typedef struct {
    SM_SLID_T slid;
    TM_TLID_T tlid;
} TM_SLID_TLID;




#define TM_MAX_TL_STORAGE 0xFFFFFFFF                            /* DTM: 4GB is max addressable by symid_t / btypeid_t and vm space is cheap */
#define TM_MAX_SLID_INC_SIZE (0x4000 / sizeof(TM_TLID_T))       /* DTM: i.e. 1 page of ids on macOS M1, 4 pages on windows intel */
#define TM_MAX_BTYPEID_INC_SIZE (0x4000 / sizeof(btypeid_t))    /* DTM: i.e. 1 page of ids on macOS M1, 4 pages on windows intel */
#define TM_MAX_ID_INC_SIZE (0x1000 / sizeof(TM_DETAILID_T))     /* DTM: i.e. 1/4 page of ids on macOS M1, 1 page on windows intel */

// HI_STRUCT_WITH(name, token_t, extravars)
HI_STRUCT_WITH(TM_BTYPEID_BY_SYMIDHASH, btypeid_t, struct PVT_TM *tm;)
HI_STRUCT_WITH(TM_TLID_BY_TLHASH, TM_TLID_T, struct PVT_TM *tm;)
HI_STRUCT_WITH(TM_DETAILID_BY_TLIDHASH, TM_DETAILID_T, TM_TLID_T *tlid_by_detailid;)    // for intersections, unions and tuples, OPEN: check we grow the tlid_by_detailid
HI_STRUCT_WITH(TM_DETAILID_BY_SLIDTLIDHASH, TM_DETAILID_T, struct PVT_TM *tm;)         // for structs, records
HI_STRUCT_WITH(TM_BTYPID_BY_SEQIDHASH, TM_DETAILID_T, struct PVT_TM *tm;)
HI_STRUCT_WITH(TM_DETAILID_BY_T1T2HASH, TM_DETAILID_T, TM_T1T2 *t1t2_by_detailid;)      // for fns and maps, OPEN: check we grow the t1t2_by_xxid

typedef struct PVT_TM {
    BK_MM *mm;
    Buckets *buckets;
    BK_SM *sm;
    BK_TP *tp;

    // type summaries
    btsummary *btsummary_by_btypeid;        // atoms take up no more space
    btypeid_t max_btypeId;
    btypeid_t next_btypeId;

    // orthogonal spaces - first cut expediently uses an array of spaceid_by_btypeid, later on could use a hash_index
    // reducing memory and improving locality?
    btypeid_t *spaceid_by_btypeid;
    btypeid_t *implicitid_by_spaceid;
    
    // type names - kept separately as they don't need to be as hot as type summaries
    symid_t *symid_by_btypeid;
    hi_struct(TM_BTYPEID_BY_SYMIDHASH) *btypeid_by_symidhash;

    // type lists - all type lists are shared by tuples, structs, records and functions
    btypeid_t *typelist_buf;                            // VM buffer of btypeid (typelist) indexed by RP
    RP max_tlrp;
    RP next_tlrp;
    RP *tlrp_by_tlid;
    TM_TLID_T max_tlid;
    TM_TLID_T next_tlid;
    hi_struct(TM_TLID_BY_TLHASH) *tlid_by_tlhash;

    // intersection details
    TM_DETAILID_T max_intid;
    TM_DETAILID_T next_intid;
    TM_TLID_T *tlid_by_intid;
    hi_struct(TM_DETAILID_BY_TLIDHASH) *intid_by_tlidhash;      // tl hash -> tupid -> btypeid
    btypeid_t *btypid_by_intid;

    // union details
    TM_DETAILID_T max_uniid;
    TM_DETAILID_T next_uniid;
    TM_TLID_T *tlid_by_uniid;
    hi_struct(TM_DETAILID_BY_TLIDHASH) *uniid_by_tlidhash;      // tl hash -> tupid -> btypeid
    btypeid_t *btypid_by_uniid;
    
    // tuple details
    TM_DETAILID_T max_tupid;
    TM_DETAILID_T next_tupid;
    TM_TLID_T *tlid_by_tupid;
    hi_struct(TM_DETAILID_BY_TLIDHASH) *tupid_by_tlidhash;      // tl hash -> tupid -> btypeid
    btypeid_t *btypid_by_tupid;
    
    // struct details
    TM_DETAILID_T max_strid;
    TM_DETAILID_T next_strid;
    btypeid_t *tlid_by_strid;           // split into two arrays assuming we mainly just interact with the type list
    SM_SLID_T *slid_by_strid;
    hi_struct(TM_DETAILID_BY_SLIDTLIDHASH) *strid_by_slidtlidhash;
    btypeid_t *btypid_by_strid;

    // sequence details - detailsid is the contained type
    hi_struct(TM_BTYPID_BY_SEQIDHASH) *containerid_by_containedidhash;

    // map details - could be a type list of two types
    TM_DETAILID_T max_mapid;
    TM_DETAILID_T next_mapid;
    TM_T1T2 *t1t2_by_mapid;
    hi_struct(TM_DETAILID_BY_T1T2HASH) *mapid_by_t1t2hash;      // t1t2 hash -> mapid -> btypeid
    btypeid_t *btypid_by_mapid;
    
    // function details
    TM_DETAILID_T max_fncid;
    TM_DETAILID_T next_fncid;
    TM_T1T2 *t1t2_by_fncid;
    hi_struct(TM_DETAILID_BY_T1T2HASH) *fncid_by_t1t2hash;      // t1t2 hash -> fncid -> btypeid
    btypeid_t *btypid_by_fncid;

    // schema variable details - nothing needed here as is just a atom in a defined range with the hasT flag set

} BK_TM;


// construction
// we want types to be immutable singleton sentinels, so we can only create them lazily when they do not have extra
// attributes, i.e. othogonal space id, is explicit flag, implicitly in id
// when constructing a compound type, e.g. maps, functions, structs, it is tempting to use a typelistid as an argument,
// so that we don't have to create the tuple fully first. This leads to problems later on.

// we allow intersections to recurse themselves so we don't need to crate dummy property types e.g. ccy or fx etc

// if recursion is needed, e.g. for intersections, tuples, structs, maps, sequences, fns
// reserve a typeid with the flags
// create the type in the process initialising the reserved typeid
// on error we are left

// OPEN: in general use pointer to structs (btsummery, t1t2, etc) rather than copying? although small structs can be
// passed in two registers.

// atoms
pub btypeid_t tm_init_atom(BK_TM *, btypeid_t btypeid, btypeid_t implicitid, bool explicit);
pub btypeid_t tm_check_atom(BK_TM *, btypeid_t btypeid, btypeid_t implicitid, bool explicit, btypeid_t spaceid);

// functions
pub btypeid_t tm_fn(BK_TM *, btypeid_t btypeid, btypeid_t argsid, btypeid_t retid);
pub TM_T1T2 tm_fn_targs_tret(BK_TM *, btypeid_t);

// intersections
pub btypeid_t tm_check_inter(BK_TM *, btypeid_t btypeid, btypeid_t spaceid);
pub btypeid_t tm_inter(BK_TM *, btypeid_t btypeid, btypeid_t *);
pub TM_TLID_T tm_inter_tlid_for(BK_TM *, btypeid_t const *);      // intersection appropiate tlid (sorted, all intersections expanded, and space checked)
pub btypeid_t tm_inter_get_for_tlid(BK_TM *, TM_TLID_T);
pub btypeid_t tm_inter_for_tlid_or_create(BK_TM *, btypeid_t btypeid, TM_TLID_T);
pub btypeid_t tm_inter_in(BK_TM *, btypeid_t btypeid, btypeid_t spaceid, btypeid_t *);
pub btypeid_t tm_interv(BK_TM *, btypeid_t btypeid, u32 numTypes, ...);
pub btypeid_t tm_interv_in(BK_TM *, btypeid_t btypeid, btypeid_t spaceid, u32 numTypes, ...);
pub btypeid_t * tm_inter_tl(BK_TM *, btypeid_t);

// maps
pub btypeid_t tm_map(BK_TM *, btypeid_t btypeid, btypeid_t keyid, btypeid_t valueid);
pub TM_T1T2 tm_map_tk_tv(BK_TM *, btypeid_t);

// schema variables
pub btypeid_t tm_schemavar(BK_TM *, btypeid_t btypeid);

// sequences
pub btypeid_t tm_seq(BK_TM *, btypeid_t btypeid, btypeid_t containedid);
pub btypeid_t tm_seq_t(BK_TM *, btypeid_t);

// structs
pub btypeid_t tm_struct(BK_TM *, btypeid_t btypeid, SM_SLID_T, TM_TLID_T);
pub SM_SLID_T tm_struct_slid(BK_TM *, btypeid_t);
pub TM_TLID_T tm_struct_tlid(BK_TM *, btypeid_t);

// tuples
pub btypeid_t tm_tuple(BK_TM *, btypeid_t btypeid, TM_TLID_T);
pub btypeid_t tm_tuplev(BK_TM *, btypeid_t btypeid, u32 numTypes, ...);
pub TM_TLID_T tm_tuple_tlid(BK_TM *, btypeid_t);

// unions
pub btypeid_t tm_union(BK_TM *, btypeid_t btypeid, btypeid_t *);
pub TM_TLID_T tm_union_tlid_for(BK_TM *, btypeid_t *);      // union appropiate tlid (sorted and all unions expanded)
pub btypeid_t tm_union_for_tlid(BK_TM *, TM_TLID_T);
pub btypeid_t tm_union_for_tlid_or_create(BK_TM *, btypeid_t btypeid, TM_TLID_T);
pub btypeid_t tm_unionv(BK_TM *, btypeid_t btypeid, u32 numTypes, ...);
pub btypeid_t * tm_union_tl(BK_TM *, btypeid_t);

// binding / lookup
// OPEN: use symid instead *char
pub btypeid_t tm_bind(BK_TM *, char const *, btypeid_t);
pub btypeid_t tm_lookup(BK_TM *, char const *);
pub char const * tm_name_of(BK_TM *, btypeid_t);

// id reservation
// pvt btypeid_t tm_reserve(BK_TM *);
pub btypeid_t tm_reserve_in(BK_TM *, btypeid_t);
pub btypeid_t tm_reserve_tbc(BK_TM *);
pub void tm_reserve_block(BK_TM *, btypeid_t);

// attribute accessing
pub bmetatypeid_t tm_bmetatypeid(BK_TM *, btypeid_t);
pub bool tm_hasT(BK_TM *, btypeid_t);
pub btypeid_t tm_layout(BK_TM *, btypeid_t);
pub btypeid_t tm_layout_as(BK_TM *, btypeid_t, size);
pub btypeid_t tm_spaceid(BK_TM *, btypeid_t);
pub btypeid_t tm_root_spaceid(BK_TM *, btypeid_t);
pub size tm_size(BK_TM *, btypeid_t);
pub btypeid_t tm_set_spaceid(BK_TM *, btypeid_t btypeid, btypeid_t spaceid);
pub btypeid_t tm_set_tbc(BK_TM *, btypeid_t);

// recursion
pub btypeid_t tm_clear_recursive_if_not(BK_TM *, btypeid_t);
pub btypeid_t tm_space_would_deeply_recurse(BK_TM *, btypeid_t btypeid, btypeid_t spaceid);

// utils
pub btypeid_t tm_minus(BK_TM *, btypeid_t btypeid, btypeid_t A, btypeid_t B);
pub TM_TLID_T tm_tlid_for(BK_TM *tm, btypeid_t *);

// type manager lifecycle fns
pub BK_TM * TM_create(BK_MM *, Buckets *, BK_SM *, BK_TP *);
pub int TM_trash(BK_TM *);



#endif  // BK_TM_H


