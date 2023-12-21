// ---------------------------------------------------------------------------------------------------------------------
// TM - TYPE MANAGER
// ---------------------------------------------------------------------------------------------------------------------

#ifndef API_BK_BM_H
#define API_BK_BM_H "bk/tm.h"

#include "bk.h"
#include "sm.h"
#include "ht.h"


// typelist is length prefixed array of btypeid, i.e. a btypeid_t *
typedef unsigned int TM_TLID_T;
typedef unsigned int TM_XXXID_T;
#define TM_RP_BY_TLID_INC_SIZE (0x4000 / sizeof(RP))        /* DTM: i.e. 1 page on macos M1, 4 pages on windows intel */

typedef unsigned int BTYPEID_T_TYPE;            /* currently ls 18 bits for 256k types */
typedef unsigned char BMETATYPEID_T_TYPE;       /* currently 4 bits for 16 metatypes */
typedef unsigned char BTEXCLUSIONCAST_T_TYPE;   /* currently 3 bits for 8 exclusion categories */


typedef enum : BTYPEID_T_TYPE {
    _nat = 0,           // not-a-type - i.e. an error code
    _m8 = 1,
    _m16 = 2,
    _m32 = 3,
    _m64 = 4,
    _p64 = 5,
    _i32 = 6,
    _litint = 7,
    _null = 8,          // empty set - not the same as not-a-type
} btypeid_t;


typedef enum : BMETATYPEID_T_TYPE {
    bmterr = 0,
    bmtnom = 1, // nominal - atomic type with a given name

                // set relations
    bmtint = 2, // intersection - sorted list of other types
    bmtuni = 3, // union - sorted list of other types

                // product types (statically known size)
    bmttup = 4, // tuple - ordered list of other types
    bmtstr = 5, // struct - ordered and named list of other types
    bmtrec = 6, // record - sorted named list of other types

                // arrows - aka exponentials - variable size, elements all the same size
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
    btnone = 0,
    btmemory = 1,
    btuser1 = 2,
    btuser2 = 4,
    btuser3 = 8,
    btuser4 = 16,
    btuser5 = 32,
    btuser6 = 64,
    btuser7 = 128,
} btexclusioncat_t;


// OPEN: with 256k types (18 bits) and 4 bits for the metatype this could be compacted into a u32
// also will store recursion, exclusivecat
struct btsummary {
    bmetatypeid_t bmtid;        // 1
    btexclusioncat_t excl;   // 1
    unsigned char unused2;      // 1
    unsigned char unused3;      // 1
    union {
        btypeid_t nomId;
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


#define TM_MAX_TL_STORAGE 0xFFFFFFFF                            /* DTM: 4GB is max addressable by SYM_ID_T and vm space is cheap */
#define TM_MAX_TLID_INC_SIZE (0x4000 / sizeof(TM_TLID_T))       /* DTM: i.e. 1 page of ids on macos M1, 4 pages on windows intel */
#define TM_MAX_BTYPEID_INC_SIZE (0x4000 / sizeof(btypeid_t))  /* DTM: i.e. 1 page of ids on macos M1, 4 pages on windows intel */
#define TM_MAX_ID_INC_SIZE (0x1000 / sizeof(TM_XXXID_T))        /* DTM: i.e. 1/4 page of ids on macos M1, 1 page on windows intel */

// HT_STRUCT2(name, slot_t, extravars)
HT_STRUCT2(TM_BTYPEID_BY_SYMIDHASH, btypeid_t, struct PVT_TM *tm;)
HT_STRUCT2(TM_TLID_BY_TLHASH, TM_TLID_T, struct PVT_TM *tm;)
HT_STRUCT2(TM_XXXID_BY_TLIDHASH, TM_XXXID_T, TM_TLID_T *tlid_by_xxxid;)

typedef struct PVT_TM {
    BK_MM *mm;
    Buckets *buckets;
    BK_SM *sm;
    struct TPM *tp;

    // type summaries
    struct btsummary *summary_by_btypeid;        
    btypeid_t max_btypeId;
    btypeid_t next_btypeId;
    
    // type names - colder than type summaries
    SYM_ID_T *symid_by_btypeid;                 
    ht_struct(TM_BTYPEID_BY_SYMIDHASH) *btypeid_by_symidhash; 

    // type lists
    btypeid_t *typelist_buf;                           // VM buffer of btypeid (typelist) indexed by RP
    RP max_rp;
    RP next_rp;
    RP *rp_by_tlid;  
    TM_TLID_T max_tlid;
    TM_TLID_T next_tlid;
    ht_struct(TM_TLID_BY_TLHASH) *tlid_by_tlhash;   

    // intersections
    TM_XXXID_T max_intid;
    TM_XXXID_T next_intid;
    ht_struct(TM_XXXID_BY_TLIDHASH) *intid_by_tlidhash;
    TM_TLID_T *tlid_by_intid;
    btypeid_t *btypid_by_intid;

    // unions
    TM_XXXID_T max_uniid;
    TM_XXXID_T next_uniid;
    ht_struct(TM_XXXID_BY_TLIDHASH) *uniid_by_tlidhash;
    TM_TLID_T *tlid_by_uniid;
    btypeid_t *btypid_by_uniid;
    
    // tuples
    TM_XXXID_T max_tupid;
    TM_XXXID_T next_tupid;
    
    // structs
    TM_XXXID_T max_strid;
    TM_XXXID_T next_strid;
    
    // records
    TM_XXXID_T max_recid;
    TM_XXXID_T next_recid;
    
    // sequences
    TM_XXXID_T max_seqid;
    TM_XXXID_T next_seqid;
    
    // maps
    TM_XXXID_T max_mapid;
    TM_XXXID_T next_mapid;
    
    // functions
    TM_XXXID_T max_fncid;
    TM_XXXID_T next_fncid;
    
    // schema variables
    TM_XXXID_T max_svrid;
    TM_XXXID_T next_svrid;
} BK_TM;

pub BK_TM * TM_create(BK_MM *, Buckets *, BK_SM *, struct TPM *);
pub int TM_trash(BK_TM *);

pub btypeid_t tm_exclnominal(BK_TM *, char *, btexclusioncat_t);
pub btypeid_t tm_btypeid(BK_TM *, char *);
pub btypeid_t tm_inter(BK_TM *, btypeid_t *);
pub char * tm_name(BK_TM *, btypeid_t);
pub btypeid_t tm_name_as(BK_TM *, btypeid_t, char *);
pub btypeid_t tm_nominal(BK_TM *, char *);
pub size tm_size(BK_TM *, btypeid_t);


#endif // API_BK_BM_H







// NTS
//
// OPEN: add aliases so can do - typedef can automatically add aliased
//<:Symb> lval(<:Node&ptr> n) {
//<:Symb> lval(<:pNode> n) {
// could we do <:unsigned int>
//
// addressOf and deref - may create new types
//
// need exclusions for M8, M16, M32, M64 -> i8: m8 & i8_ or poss i8: m8_ & i & signed, etc
// ptr1, const1, ptr2, const2, ptr3, const3, extern, the basic c types



//#define DESC_ID unsigned int


//struct BType {
//    bmetatypeid_t meta;             // 1 + 3 pad OPEN: could do 4 bits + 28 bits (250k) for type
//    DESC_ID descId;                 // 4
//};
//
//typedef struct {
//    btypeid_t n;                    // 4
//    btype ts[];                     // n * 4
//} BTypeList;
//
//struct BTIntersection {
//    BTypeList types;                // 4 + n * 4
//};
//
//struct BTUnion {
//    BTypeList types;                // 4 + n * 4
//};
//
//struct BTTuple {
//    BTypeList types;                // 4 + n * 4
//};
//
//typedef int bsym;
//
//struct BTStruct {
//    bsymlist *names;                // 8
//    btypelist typelist;             // length prefix array of btypeid
//};
//
//struct BTRec {
//    bsymlist *names;                // 8
//    btypelist types;                // 4 + n * 4
//};
//
//struct BTSeq {
//    btype tElem;                    // 4
//};
//
//struct BTMap {
//    btype tKey;                     // 4
//    btype tValue;                   // 4
//};
//
//struct BTFunc {
//    btype tRet;                     // 4
//    btype tFn;                      // 4
//    bsymlist *names;                // 8
//    btypelist *argtypes;            // 4 + n * 4
//};
//
//
//// ---------------------------------------------------------------------------------------------------------------------
//// SType - StructuralType
//// ---------------------------------------------------------------------------------------------------------------------
//// the following is a reduced physical description of structs / tuples and c arrays for tracing and copying - can be
//// generated for ctypes and btypes - there will be less stypes than btypes / ctypes but don't know how many
//
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
//    multi = 4,          // box by placing the btype (4 bytes) before aligned object
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
//    btype btype;        // type is held in upper 4 bytes to match heap organisation
//    union {
//        double d;
//        long l;
//        void *p;
//    };
//};                      // 16

//struct box4 {
//    btype btype;        // type is held in upper 4 bytes to match heap organisation
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
// m32 btype
// double a    <- *a
// ------- b -------
// m32 pad
// m32 btype
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
// m32 btype
// double a    <- *a
// ------- b -------
// m32 pad
// m32 btype
// double b    <- *b
// -----------------
