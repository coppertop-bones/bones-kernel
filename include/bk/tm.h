// ---------------------------------------------------------------------------------------------------------------------
// TM - TYPE MANAGER
// ---------------------------------------------------------------------------------------------------------------------

#ifndef BK_TM_H
#define BK_TM_H "bk/tm.h"

#include "bk.h"
#include "sm.h"
#include "hi.h"


typedef u32 BTYPEID_T_TYPE;         /* currently ls 18 bits for 256k types */
typedef u8 BMETATYPEID_T_TYPE;      /* currently 4 bits for 16 metatypes */
typedef u8 BTEXCLUSIONCAST_T_TYPE;  /* currently 3 bits for 8 exclusion categories */
typedef u16 BTSIZE_TYPE;            /* only allow types up to 64k in size */

// typelist is length prefixed array of btypeid, i.e. a btypeid_t *
typedef u32 TM_TLID_T;
typedef u32 TM_SLID_T;
typedef u32 TM_XXXID_T;
#define TM_RP_BY_TLID_INC_SIZE (0x4000 / sizeof(RP))        /* DTM: i.e. 1 page on macos M1, 4 pages on windows intel */


typedef enum : BTYPEID_T_TYPE {
    B_NAT = 0,       // not-a-type - i.e. an error code
    B_NULL,          // empty set - not the same as not-a-type
    B_VOID,
    B_M8,
    B_M16,
    B_M32,
    B_M64,
    B_CHAR,         // implementation defined (poss with compiler flags)
    B_U8,
    B_U16,
    B_U32,
    B_U64,
    B_I8,
    B_I16,
    B_I32,
    B_I64,
    B_F32,
    B_F64,
    B_P,
    B_PP,
    B_PPP,
    B_RP8,
    B_RP16,
    B_RP32,
    B_RP64,
    B_LITINT,
    B_LITDEC,

    B_EXTERN,
    B_STATIC,
    B_AUTO,
    B_REGISTER,
    B_VARARGS,
    B_CONST,
    B_CONST_P,
    B_RESTRICT,
    B_VOLATILE,
    B_FN,
    B_CHAR_STAR,
    B_CHAR_CONST_STAR,
    B_CHAR_CONST_STAR_CONST,
    B_VOID_STAR,
    B_FN_PTR,
    B_EXTERN_FN,
    B_EXTERN_FN_PTR,

} btypeid_t;


typedef enum : BTSIZE_TYPE {
    NOSZ = 0,
} btypesize_t;



typedef enum : BMETATYPEID_T_TYPE {
    bmterr = 0,
    bmtnom = 1, // nominal - atomic type with a given name

                // relations
    bmtint = 2, // intersection - sorted list of other types
    bmtuni = 3, // union - sorted list of other types

                // product types (statically known size)
    bmttup = 4, // tuple - ordered list of other types
    bmtstr = 5, // struct - ordered and named list of other types
    bmtrec = 6, // record - sorted named list of other types

                // arrows - aka exponentials - variable length, elements all the same
    bmtseq = 7, // sequence - tElement
    bmtmap = 8, // map / dictionary - tKey, tValue
    bmtfnc = 9, // function - argnames, tArgs, tRet, tFunc, num args

                // schemas
    bmtsvr = 10,// schema variable
} bmetatypeid_t;


// allow up to 8 categories of exclusion so can AND these together to detail conflict - OPEN: is that enough categories
// actually since exclusion is detected on union creation speed is not that important - so we can have intersections of
// exclusions but that is a little more involved - use the bitmask for the moment
typedef enum : BTEXCLUSIONCAST_T_TYPE {
    btenone = 0,
    btememory = 1,
    bteptr = 2,
    bteccy = 4,
    bteuser1 = 8,
    bteuser2 = 16,
    bteuser3 = 32,
    bteuser4 = 64,
    bteuser5 = 128,
} btexclusioncat_t;


// OPEN: with 256k types (18 bits) and 4 bits for the metatype this could be compacted into a u32
// also will store recursion, exclusivecat
// OPEN: hasT etc
struct btsummary {
    bmetatypeid_t bmtid;        // 1
    btexclusioncat_t excl;      // 1
    btypesize_t unused2;        // 2
    union {
        btypeid_t _id;
        btypeid_t intId;
        btypeid_t uniId;
        btypeid_t tupId;
        btypeid_t strId;
        btypeid_t recId;
        btypeid_t seqId;
        btypeid_t mapId;
        btypeid_t fncId;
        btypeid_t svrId;
    };                          // 4
};

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


#define TM_MAX_TL_STORAGE 0xFFFFFFFF                            /* DTM: 4GB is max addressable by symid_t and vm space is cheap */
#define TM_MAX_TLID_INC_SIZE (0x4000 / sizeof(TM_TLID_T))       /* DTM: i.e. 1 page of ids on macos M1, 4 pages on windows intel */
#define TM_MAX_BTYPEID_INC_SIZE (0x4000 / sizeof(btypeid_t))    /* DTM: i.e. 1 page of ids on macos M1, 4 pages on windows intel */
#define TM_MAX_ID_INC_SIZE (0x1000 / sizeof(TM_XXXID_T))        /* DTM: i.e. 1/4 page of ids on macos M1, 1 page on windows intel */

// HI_STRUCT_WITH(name, token_t, extravars)
HI_STRUCT_WITH(TM_BTYPEID_BY_SYMIDHASH, btypeid_t, struct PVT_TM *tm;)
HI_STRUCT_WITH(TM_TLID_BY_TLHASH, TM_TLID_T, struct PVT_TM *tm;)
HI_STRUCT_WITH(TM_SLID_BY_TLHASH, TM_SLID_T, struct PVT_TM *tm;)
HI_STRUCT_WITH(TM_XXXID_BY_TLIDHASH, TM_XXXID_T, TM_TLID_T *tlid_by_xxxid;)    // for intersections, unions and tuples, OPEN: check we grow the tlid_by_xxxid
HI_STRUCT_WITH(TM_BTYPID_BY_SEQIDHASH, TM_XXXID_T, struct PVT_TM *tm;)
HI_STRUCT_WITH(TM_XXXID_BY_T1T2HASH, TM_XXXID_T, TM_T1T2 *t1t2_by_xxxid;)       // for fns and maps, OPEN: check we grow the t1t2_by_xxid

typedef struct PVT_TM {
    BK_MM *mm;
    Buckets *buckets;
    BK_SM *sm;
    struct TPM *tp;

    // type summaries
    struct btsummary *summary_by_btypeid;        
    btypeid_t max_btypeId;
    btypeid_t next_btypeId;
    
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

    // sym lists - all sym lists are shared by structs, records and functions
    symid_t *symlist_buf;                               // VM buffer of symid (symlist) indexed by RP
    RP max_slrp;
    RP next_slrp;
    RP *slrp_by_slid;
    TM_SLID_T max_slid;
    TM_SLID_T next_slid;
    hi_struct(TM_SLID_BY_SLHASH) *slid_by_slhash;

    // intersections
    TM_XXXID_T max_intid;
    TM_XXXID_T next_intid;
    TM_TLID_T *tlid_by_intid;
    hi_struct(TM_XXXID_BY_TLIDHASH) *intid_by_tlidhash;     // tl hash -> tupid -> btypeid
    btypeid_t *btypid_by_intid;

    // unions
    TM_XXXID_T max_uniid;
    TM_XXXID_T next_uniid;
    TM_TLID_T *tlid_by_uniid;
    hi_struct(TM_XXXID_BY_TLIDHASH) *uniid_by_tlidhash;     // tl hash -> tupid -> btypeid
    btypeid_t *btypid_by_uniid;
    
    // tuples
    TM_XXXID_T max_tupid;
    TM_XXXID_T next_tupid;
    TM_TLID_T *tlid_by_tupid;
    hi_struct(TM_XXXID_BY_TLIDHASH) *tupid_by_tlidhash;     // tl hash -> tupid -> btypeid
    btypeid_t *btypid_by_tupid;
    
    // structs
    TM_XXXID_T max_strid;
    TM_XXXID_T next_strid;
    TM_TLID_T *tlid_by_strid;
    TM_TLID_T *slid_by_strid;
    hi_struct(TM_XXXID_BY_SLIDTLIDHASH) *strid_by_slidtlidhash;     // OPEN: hash on tl and sl values?
    btypeid_t *btypid_by_strid;
    
    // records - replicate the same structure as for structs
//    TM_XXXID_T max_recid;
//    TM_XXXID_T next_recid;
    
    // sequences -> need hash map keyed by underlying type mapping to containing type - tmsummary_by_typeid[contining
    hi_struct(TM_BTYPID_BY_SEQIDHASH) *containerid_by_containedidhash;

    // maps - could be a type list of two types
    TM_XXXID_T max_mapid;
    TM_XXXID_T next_mapid;
    TM_T1T2 *t1t2_by_mapid;
    hi_struct(TM_XXXID_BY_T1T2HASH) *mapid_by_t1t2hash;     // t1t2 hash -> mapid -> btypeid
    btypeid_t *btypid_by_mapid;
    
    // functions
    TM_XXXID_T max_fncid;
    TM_XXXID_T next_fncid;
    TM_T1T2 *t1t2_by_fncid;
    hi_struct(TM_XXXID_BY_T1T2HASH) *fncid_by_t1t2hash;     // t1t2 hash -> fncid -> btypeid
    btypeid_t *btypid_by_fncid;

    // schema variables
    TM_XXXID_T max_svrid;
    TM_XXXID_T next_svrid;
    // OPEN: TBC
} BK_TM;


pub BK_TM * TM_create(BK_MM *, Buckets *, BK_SM *, struct TPM *);
pub int TM_trash(BK_TM *);

pub bmetatypeid_t tm_bmetatypeid(BK_TM *, btypeid_t);
pub btypeid_t tm_btypeid(BK_TM *, char *);
pub btypeid_t tm_exclnominal(BK_TM *, char *, btexclusioncat_t, btypesize_t, btypeid_t);
pub btexclusioncat_t tm_exclusion_cat(BK_TM *, char *, btexclusioncat_t);
pub btypeid_t tm_fn(BK_TM *, btypeid_t tArgs, btypeid_t tRet, btypeid_t);
pub TM_T1T2 tm_Fn(BK_TM *, btypeid_t);
pub btypeid_t tm_inter(BK_TM *, btypeid_t *, btypeid_t);
pub btypeid_t tm_interv(BK_TM *, u32 numTypes, ...);
pub btypeid_t * tm_inter_tl(BK_TM *, btypeid_t);
pub btypeid_t tm_map(BK_TM *, btypeid_t tKey, btypeid_t tValue, btypeid_t);
pub TM_T1T2 tm_Map(BK_TM *, btypeid_t);
pub btypeid_t tm_minus(BK_TM *, btypeid_t A, btypeid_t B, btypeid_t);
pub char * tm_name(BK_TM *, btypeid_t);                 // OPEN: return symid instead
pub btypeid_t tm_name_as(BK_TM *, btypeid_t, char *);
pub btypeid_t tm_nominal(BK_TM *, char *, btypeid_t);
pub btypeid_t tm_schemavar(BK_TM *, char *, btypeid_t);
pub btypeid_t tm_seq(BK_TM *, btypeid_t tContained, btypeid_t);
pub btypeid_t tm_seq_t(BK_TM *, btypeid_t);
pub size tm_size(BK_TM *, btypeid_t);
pub btypeid_t tm_size_as(BK_TM *, btypeid_t, size);
pub btypeid_t tm_struct(BK_TM *, symid_t *, btypeid_t *, btypeid_t);
pub btypeid_t tm_tuple(BK_TM *, btypeid_t *, btypeid_t);
pub btypeid_t tm_tuplev(BK_TM *, u32 numTypes, ...);
pub btypeid_t * tm_tuple_tl(BK_TM *, btypeid_t);
pub btypeid_t tm_union(BK_TM *, btypeid_t *, btypeid_t);
pub btypeid_t tm_unionv(BK_TM *, u32 numTypes, ...);
pub btypeid_t * tm_union_tl(BK_TM *, btypeid_t);

#endif // BK_TM_H



//// ---------------------------------------------------------------------------------------------------------------------
//// SType - StructuralType
//// ---------------------------------------------------------------------------------------------------------------------
//// the following is a reduced physical description of structs / tuples and c arrays for tracing and copying - can be
//// generated for ctypes and btypes - there will be less stypes than btypes / ctypes but don't know how many
//
//enum fieldtype : char {
//    ptrToShallow = 1,       // offset to ptr to an object that contains no pointers
//    ptrToDeep = 2,          // offset to ptr to object with pointers
//    ptrToVariable = 3,      // offset to ptr to variable size object
//    ptrToUnknown = 4,       // offset to void* (not managed)
//    firstShallowElement = 5,
//    firstPtrToShallowElement = 6,
//    firstPtrToDeepElement = 7,
//    firstPtrToVariableElement = 8,
//    firstPtrToUnknownElement = 9,
//};
//
//struct fielddesc {
//    unsigned short offset;      // 2
//    enum fieldtype type;            // 1 + 1 padding
//};
//
//enum countType : char {
//    given = 0,
//    m8 = 1,
//    m16 = 2,
//    m32 = 3,
//    m64 = 4,
//};
//
//struct fields {
//    unsigned short numfields;           // 2 + 2 padding
//    struct fielddesc fielddescs[];      // numfields * 4
//};
//
//struct SType {
//    unsigned short basicSizeOf;         // 2 - size of the object without the array
//    unsigned short elementSize;         // 2 - size of each array element
//    unsigned short numElementsOffset;   // 2 - offset to the element count or the actual count
//    enum countType numElementsType;     // 1
//    char isDeep;                        // 1
//    struct fields *fields;              // 8 - OPEN: could make this an id into an array reducing to 12 bytes
//};                                      // 16
//
//// object size is layout.basicSizeOf + &(layout.numElementsOffset) * layout.elementSize
//
//// passing pointers to heap objects means the dispatch can work as can get type from pre area
//// tagged unions can't be pass in registers but only in boxes that are at least 16 bytes for a double
//// pass pointers to temporaries?
//
//
//struct SType* stypes[0xFFFF];
//typedef unsigned short stype;
//
//
//// managed mode
//enum managedmode : char {
//    none = 0,           // nothing (0 bytes) is placed before aligned object
//    moving = 2,         // stype (2 bytes) is placed before aligned object
//    multi = 4,          // box by placing the btypeid_t (4 bytes) before aligned object
//};

// boxing on stack - a 16 byte struct is passed

// https://stackoverflow.com/questions/74832688/how-to-determine-the-correct-way-to-pass-the-struct-parameters-in-arm64-assembly
// https://github.com/ARM-software/abi-aa/blob/2982a9f3b512a5bfdc9e3fea5d3b298f9165c36b/aapcs64/aapcs64.rst#parameter-passing-rules
// https://developer.apple.com/documentation/xcode/writing-arm64-code-for-apple-platforms
// https://github.com/ARM-software/abi-aa/blob/2982a9f3b512a5bfdc9e3fea5d3b298f9165c36b/aapcs64/aapcs64.rst#arm-c-and-c-language-mappings
// https://github.com/ARM-software/abi-aa/blob/2982a9f3b512a5bfdc9e3fea5d3b298f9165c36b/aapcs64/aapcs64.rst#the-base-procedure-call-standard

// https://github.com/ARM-software/abi-aa/releases

//https://github.com/ivmai/bdwgc/


//struct box8 {
//    int pad;
//    btypeid_t btype;        // type is held in upper 4 bytes to match heap organisation
//    union {
//        double d;
//        long l;
//        void *p;
//    };
//};                      // 16

//struct box4 {
//    btypeid_t btype;        // type is held in upper 4 bytes to match heap organisation
//    union {
//        char c;
//        short s;
//        int i;
//        float f;
//    };
//};                      // 8

//add_2(a:*double+err, b:*double+err) -> double+err
//add_2(a:double+err, b:double+err) -> double+err


// calling add_2(a:*double+err, b:*double+err) -> double+err with 2 doubles
//
// stack
// ------- a -------
// m32 pad
// m32 btypeid
// double a    <- *a
// ------- b -------
// m32 pad
// m32 btypeid
// double b    <- *b
// -----------------

// calling add_2(a:double, b:double) -> double with 2 doubles
// get put in registers

// CONCLUSION for moment bones just uses pointers for calling - can optimise to registers later (and we should)

// my qbe code will need to allocate 16 bytes for doubles / longs, 8 bytes for ints, shorts, chars

// https://stackoverflow.com/questions/68369577/how-to-control-the-abi-for-unions
// https://www.open-std.org/jtc1/sc22/wg14/www/docs/n2289.pdf - exceptions proposal
// discussion on above - https://news.ycombinator.com/item?id=17922715
// arm abi - https://github.com/ARM-software/abi-aa/releases

// calling add_2(a:double+err, b:double+err) -> double+err with 2 doubles
//
// stack
// ------- a -------
// m32 pad
// m32 btypeid
// double a    <- *a
// ------- b -------
// m32 pad
// m32 btypeid
// double b    <- *b
// -----------------
