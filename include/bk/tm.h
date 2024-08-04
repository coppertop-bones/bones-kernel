// ---------------------------------------------------------------------------------------------------------------------
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
    bmtnom = 0x10000000, // nominal - atomic type with a given name

    // relations
    bmtint = 0x20000000, // intersection - sorted list of other types
    bmtuni = 0x30000000, // union - sorted list of other types

    // product types (statically known size)
    bmttup = 0x40000000, // tuple - ordered list of other types
    bmtstr = 0x50000000, // struct - ordered and named list of other types
    bmtrec = 0x60000000, // record - sorted named list of other types

    // arrows - aka exponentials - variable length, elements all the same
    bmtseq = 0x70000000, // sequence - tElement
    bmtmap = 0x80000000, // map / dictionary - tKey, tValue
    bmtfnc = 0x90000000, // function - argnames, tArgs, tRet, tFunc, num args

    // schemas
    bmtsvr = 0xA0000000,// schema variable

    // inference metatypes? 11 - 15

} bmetatypeid_t;


// btypeid is 20 bits - we can possibly drop a bit from the details id without problem

// btsummary - compressed into a u32
typedef u32 btsummary;      // XXXX FFFF FFFF YYYY YYYY YYYY YYYY YYYY - X is metatypeid, F is flag, Y is detailsid

#define TM_BMTID_MASK           0xF0000000      /* 4-bits allowing up to 15 metatypes */
#define TM_DETAILS_ID_MASK      0x000FFFFF      /* 20-bits at least up to 1,048,576 of each of the meta-types */
#define TM_HAS_T_MASK           0x08000000
#define TM_IN_FAMILY_MASK       0x04000000
#define TM_IS_MEM_MASK          0x02000000
#define TM_IS_EXPLICIT_MASK     0x01000000
#define TM_IS_RECURSIVE_MASK    0x00800000
#define TM_IS_VARIABLE_MASK     0x00400000      /* replicated in object header for faster MM */
#define TM_IS_PTR_MASK          0x00200000      /* replicated in object header for faster MM does not require a type look up when tracing */
#define TM_HAS_PTR_MASK         0x00100000      /* replicated in object header for faster MM requires a type look up when tracing */


#define TM_BMT_ID(x) (x & TM_BMTID_MASK)
#define TM_DETAILS_ID(x) (x & TM_DETAILS_ID_MASK)
#define TM_HAS_T(x) (x & TM_HAS_T_MASK)
#define TM_IN_FAMILY(x) (x & TM_IN_FAMILY_MASK)
#define TM_IS_RECURSIVE(x) (x & TM_IS_RECURSIVE_MASK)
#define TM_IS_MEM(x) (x & TM_IS_MEM_MASK)
#define TM_IS_EXPLICIT(x) (x & TM_IS_EXPLICIT_MASK)


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
    btypeid_t tupid;
} TM_SLID_TUPID;




#define TM_MAX_TL_STORAGE 0xFFFFFFFF                            /* DTM: 4GB is max addressable by symid_t / btypeid_t and vm space is cheap */
#define TM_MAX_SLID_INC_SIZE (0x4000 / sizeof(TM_TLID_T))       /* DTM: i.e. 1 page of ids on macOS M1, 4 pages on windows intel */
#define TM_MAX_BTYPEID_INC_SIZE (0x4000 / sizeof(btypeid_t))    /* DTM: i.e. 1 page of ids on macOS M1, 4 pages on windows intel */
#define TM_MAX_ID_INC_SIZE (0x1000 / sizeof(TM_DETAILID_T))     /* DTM: i.e. 1/4 page of ids on macOS M1, 1 page on windows intel */

// HI_STRUCT_WITH(name, token_t, extravars)
HI_STRUCT_WITH(TM_BTYPEID_BY_SYMIDHASH, btypeid_t, struct PVT_TM *tm;)
HI_STRUCT_WITH(TM_TLID_BY_TLHASH, TM_TLID_T, struct PVT_TM *tm;)
HI_STRUCT_WITH(TM_DETAILID_BY_TLIDHASH, TM_DETAILID_T, TM_TLID_T *tlid_by_detailid;)    // for intersections, unions and tuples, OPEN: check we grow the tlid_by_detailid
HI_STRUCT_WITH(TM_DETAILID_BY_SLIDTUPIDHASH, TM_DETAILID_T, struct PVT_TM *tm;)         // for structs, records
HI_STRUCT_WITH(TM_BTYPID_BY_SEQIDHASH, TM_DETAILID_T, struct PVT_TM *tm;)
HI_STRUCT_WITH(TM_DETAILID_BY_T1T2HASH, TM_DETAILID_T, TM_T1T2 *t1t2_by_detailid;)      // for fns and maps, OPEN: check we grow the t1t2_by_xxid

typedef struct PVT_TM {
    BK_MM *mm;
    Buckets *buckets;
    BK_SM *sm;
    BK_TP *tp;

    // type summaries
    btsummary *btsummary_by_btypeid;
    btypeid_t max_btypeId;
    btypeid_t next_btypeId;

    // family memberships - for speed of first implementation expediently use an array of familyid_by_btypeid
    // later family could be kept per details id (same amount of space?) or in a hash_index
    btypeid_t *familyid_by_btypeid;
    
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
    btypeid_t *tupid_by_strid;           // split into two arrays assuming we mainly just interact with the type list
    SM_SLID_T *slid_by_strid;
    hi_struct(TM_DETAILID_BY_SLIDTUPIDHASH) *strid_by_slidtupidhash;
    btypeid_t *btypid_by_strid;
    
    // record details
    TM_DETAILID_T max_recid;
    TM_DETAILID_T next_recid;
    btypeid_t *tupid_by_recid;           // split into two arrays assuming we mainly just interact with the type list
    SM_SLID_T *slid_by_recid;
    hi_struct(TM_DETAILID_BY_SLIDTUPIDHASH) *recid_by_slidtlidhash;
    btypeid_t *btypid_by_recid;
    
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

    // schema variable details - nothing needed here as is just a nominal in a defined range with the hasT flag set

} BK_TM;


pub BK_TM * TM_create(BK_MM *, Buckets *, BK_SM *, BK_TP *);
pub int TM_trash(BK_TM *);

pub bmetatypeid_t tm_bmetatypeid(BK_TM *, btypeid_t);
pub btypeid_t tm_btypeid(BK_TM *, char const *);
pub btypeid_t tm_exp(BK_TM *, btypeid_t self);
pub btypeid_t tm_fn(BK_TM *, btypeid_t tArgs, btypeid_t tRet, btypeid_t);
pub TM_T1T2 tm_fn_targs_tret(BK_TM *, btypeid_t);
pub bool tm_hasT(BK_TM *, btypeid_t);
pub btypeid_t tm_inter(BK_TM *, btypeid_t, btypeid_t *);
pub btypeid_t * tm_inter_tl(BK_TM *, btypeid_t);
pub btypeid_t tm_inter_v(BK_TM *, btypeid_t, u32 numTypes, ...);
pub btypeid_t tm_map(BK_TM *, btypeid_t tKey, btypeid_t tValue, btypeid_t);
pub TM_T1T2 tm_map_tk_tv(BK_TM *, btypeid_t);
pub btypeid_t tm_minus(BK_TM *, btypeid_t A, btypeid_t B, btypeid_t);
pub char const * tm_name(BK_TM *, btypeid_t);                 // OPEN: return symid instead
pub btypeid_t tm_name_as(BK_TM *, btypeid_t, char const *);
pub btypeid_t tm_nominal(BK_TM *, btypeid_t self, btypeid_t familyid, char const *);
pub btypeid_t tm_nominal_impin(BK_TM *, btypeid_t self, btypeid_t familyid, char const *);
pub btypeid_t tm_rec(BK_TM *, symid_t *, btypeid_t *, btypeid_t);
pub btypeid_t tm_recursive(BK_TM *, btypeid_t);
pub btypeid_t tm_recursive_in(BK_TM *, btypeid_t self, btypeid_t familyid);
pub btypeid_t tm_schemavar(BK_TM *, btypeid_t, char const *);
pub btypeid_t tm_seq(BK_TM *, btypeid_t, btypeid_t tContained);
pub btypeid_t tm_seq_t(BK_TM *, btypeid_t);
pub size tm_size(BK_TM *, btypeid_t);
pub btypeid_t tm_size_as(BK_TM *, btypeid_t, size);
pub btypeid_t tm_struct(BK_TM *, btypeid_t, SM_SLID_T, btypeid_t);
pub btypeid_t tm_structv_sts(BK_TM *, u32 numTypes, ...);
pub btypeid_t tm_structv_ssts(BK_TM *, u32 numTypes, ...);
pub symid_t * tm_struct_sl(BK_TM *, btypeid_t);
pub btypeid_t * tm_struct_tl(BK_TM *, btypeid_t);
pub btypeid_t tm_tuple(BK_TM *, btypeid_t, btypeid_t *);
pub btypeid_t tm_tuplev(BK_TM *, u32 numTypes, ...);
pub btypeid_t * tm_tuple_tl(BK_TM *, btypeid_t);
pub btypeid_t tm_union(BK_TM *, btypeid_t, btypeid_t *);
pub btypeid_t tm_unionv(BK_TM *, u32 numTypes, ...);
pub btypeid_t * tm_union_tl(BK_TM *, btypeid_t);
pub btypeid_t tm_union_v(BK_TM *, btypeid_t, u32 numTypes, ...);

#endif // BK_TM_H


