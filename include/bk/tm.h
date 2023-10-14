#ifndef __BK_BM_H
#define __BK_BM_H "bk/tm.h"

#include "bk.h"
#include "sm.h"
#include "ht.h"
#include "buckets.h"


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
// counting sort - for enums
// dictionary sort for sym names
// general sort for type lists



typedef enum : BTYPE_ID_T {
    _nat = 0,           // not-a-type - i.e. an error code
    _m8 = 1,
    _m16 = 2,
    _m32 = 3,
    _m64 = 4,
    _p64 = 5,
    _i32 = 6,
    _litint = 7,
    _null = 8,          // empty set - not the same as not-a-type
} btype;


#define TM_TL_ID_T unsigned int

enum bmetatype : unsigned char {
    btnom = 1,  // nominal - atomic type with a given name
        // set ops
    btint = 2,  // intersection - sorted list of other types
    btuni = 3,  // union - sorted list of other types
        // product types (statically known size)
    bttup = 4,  // tuple - ordered list of other types
    btstr = 5,  // struct - ordered and named list of other types
    btrec = 6,  // record - sorted named list of other types
        // exponentials (variable size, elements all the same size)
    btseq = 7,  // sequence - tElement
    btmap = 8,  // map / dictionary - tKey, tValue
    btfnc = 9,  // function - argnames, tArgs, tRet, tFunc, num args
        // schemas
    btsvr = 10,  // schema variable
};


// OPEN: with 256k types (18 bits) and 4 bits for the metatype this could be compacted into a u32
// also will store recursion, exclusiveId
struct btsummary {
    enum bmetatype mt;
    unsigned char unused1;
    unsigned char unused2;
    unsigned char unused3;
    union {
        BTYPE_ID_T nomId;
        BTYPE_ID_T intId;
        BTYPE_ID_T uniId;
        BTYPE_ID_T tupId;
        BTYPE_ID_T strId;
        BTYPE_ID_T recId;
        BTYPE_ID_T seqId;
        BTYPE_ID_T mapId;
        BTYPE_ID_T fncId;
        BTYPE_ID_T svrId;
    };
};




#define DESC_ID unsigned int


struct BType {
    enum bmetatype meta;            // 1 + 3 pad OPEN: could do 4 bits + 28 bits (250k) for type
    DESC_ID descId;                 // 4
};

typedef struct {
    BTYPE_ID_T n;                   // 4
    btype ts[];                     // n * 4
} BTypeList;

struct BTIntersection {
    BTypeList types;                // 4 + n * 4
};

struct BTUnion {
    BTypeList types;                // 4 + n * 4
};

struct BTTuple {
    BTypeList types;                // 4 + n * 4
};

typedef int bsym;

struct BTStruct {
    bsym *names;                    // 8
    BTypeList types;                // 4 + n * 4
};

struct BTRec {
    bsym *names;                    // 8
    BTypeList types;                // 4 + n * 4
};

struct BTSeq {
    btype tElem;                    // 4
};

struct BTMap {
    btype tKey;                     // 4
    btype tValue;                   // 4
};

struct BTFunc {
    btype tRet;                     // 4
    btype tFn;                      // 4
    bsym *names;                  // 8
    BTypeList *argtypes;            // 4 + n * 4
};

enum bexclusioncat {
    btnone = 0,
    btmemory = 1,
};


// ---------------------------------------------------------------------------------------------------------------------
// SType - StructuralType
// ---------------------------------------------------------------------------------------------------------------------
// the following is a reduced physical description of structs / tuples and c arrays for tracing and copying - can be
// generated for ctypes and btypes - there will be less stypes than btypes / ctypes but don't know how many


enum fieldtype : char {
    ptrToShallow = 1,       // offset to ptr to an object that contains no pointers
    ptrToDeep = 2,          // offset to ptr to object with pointers
    ptrToVariable = 3,      // offset to ptr to variable size object
    ptrToUnknown = 4,       // offset to void* (not managed)
    firstShallowElement = 5,
    firstPtrToShallowElement = 6,
    firstPtrToDeepElement = 7,
    firstPtrToVariableElement = 8,
    firstPtrToUnknownElement = 9,
};

struct fielddesc {
    unsigned short offset;      // 2
    enum fieldtype type;            // 1 + 1 padding
};

enum countType : char {
    given = 0,
    m8 = 1,
    m16 = 2,
    m32 = 3,
    m64 = 4,
};

struct fields {
    unsigned short numfields;           // 2 + 2 padding
    struct fielddesc fielddescs[];      // numfields * 4
};

struct SType {
    unsigned short basicSizeOf;         // 2 - size of the object without the array
    unsigned short elementSize;         // 2 - size of each array element
    unsigned short numElementsOffset;   // 2 - offset to the element count or the actual count
    enum countType numElementsType;     // 1
    char isDeep;                        // 1
    struct fields *fields;              // 8 - OPEN: could make this an id into an array reducing to 12 bytes
};                                      // 16

// object size is layout.basicSizeOf + &(layout.numElementsOffset) * layout.elementSize

// passing pointers to heap objects means the dispatch can work as can get type from pre area
// tagged unions can't be pass in registers but only in boxes that are at least 16 bytes for a double
// pass pointers to temporaries?


struct SType* stypes[0xFFFF];
typedef unsigned short stype;


// managed mode
enum managedmode : char {
    none = 0,           // nothing (0 bytes) is placed before aligned object
    moving = 2,         // stype (2 bytes) is placed before aligned object
    multi = 4,          // box by placing the btype (4 bytes) before aligned object
};

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

// HT_STRUCT2(name, slot_t, extravars)
HT_STRUCT2(TM_BTYPE_ID_HT, BTYPE_ID_T, struct TM* tm;)
HT_STRUCT2(TM_TL_ID_HT, TM_TL_ID_T, struct TM* tm;)

struct TM {
    struct MM *mm;
    struct SM *sm;
    struct btsummary *sumByBTypeId;         // array of struct btsummary indexed by btypeId
    SM_SYM_ID_T *symIdByBTypeId;            // array of symId indexed by btypeId
    ht_struct(TM_BTYPE_ID_HT) *btypeId_ht;  // hash table of btypeId
    TM_TL_ID_T *tls;                        // VM buffer of length prefixed ordered list of btypeIds
    RP *tlRpById;                           // array of type list RP indexed by typeListId
    ht_struct(TM_TL_ID_T) *symIdByName;     // 8 - hash table for type list lookup
    RP next_tl_rp;
    RP max_tl_rp;                           // 4GB of VM
    unsigned int nextTlId;
    unsigned int symIdByBTypeIdSize;
    unsigned int nextBTypeId;
    unsigned int nextIntId;
    unsigned int nextUniId;
    unsigned int nextTupId;
    unsigned int nextStrId;
    unsigned int nextRecId;
    unsigned int nextSeqId;
    unsigned int nextMapId;
    unsigned int nextFncId;
    unsigned int nextSvrId;
};

pub struct TM * TM_create(struct MM *, struct SM *);
pub int TM_trash(struct TM *);
pub BTYPE_ID_T tm_id(struct TM *, char const * const);
pub char * tm_name(struct TM *, BTYPE_ID_T);
pub BTYPE_ID_T tm_nominal(struct TM *, char const * const);

#endif // __BK_BM_H