// symbol encoding
// size prefixed, utf8 sequence, from 0 to 255 bytes so 0 is effectively the null symbol - we "waste" one byte for
// null termination so standard c string functions can work - size prefix allows for slightly faster comparison as
// we check size first


#ifndef API_BK_SM_H
#define API_BK_SM_H "bk/sm.h"

#include "bk.h"
#include "ht.h"


// this is more for documentation than usage to make clear that a symname is length prefixed
struct symname {
    unsigned short n;
    char buf[];             // <<<< RP points here
};


// following names are easy to find in auto complete as they all start with SM_
#define SM_MAX_NAME_LEN 0xFF                                    /* DTM: symbols can be up to 255 bytes of utf8 inc null termination - can be increased */
#define SM_MAX_NAME_STORAGE 0xFFFFFFFF                          /* DTM: 4GB is max addressable by SYM_ID_T and vm space is cheap */
#define SM_MAX_SYM_ID_INC_SIZE (0x4000 / sizeof(RP))              /* DTM: i.e. 1 page on macos M1, 4 pages on windows intel */
#define SM_SYMS_NOT_SORTED 0
#define SM_SYMS_SORTED 1

#define SM_ERR_NAME_TOO_LONG 1
#define SM_ERR_NAME_TOO_SHORT 2
#define SM_ERR_OUT_OF_NAME_STORAGE 3


// HT_STRUCT2(name, slot_t, extravars)
HT_STRUCT2(SM_SYMID_BY_NAMEHASH, u32, struct SM* sm;)


struct SM {
    char *symname_buf;                      // 8 - VM buffer of u16 length prefixed, null terminated utf8 strings for type name and sym interning
    RP *rp_by_symid;                        // 8 - array of name RP indexed by id
    u32 *sortorder_by_symid;                // 8 - array of sort_order indexed by id - slot0 is 1 if sorted, 0 if not sorted
    ht_struct(SM_SYMID_BY_NAMEHASH) *symid_by_namehash;    // 8 - hash table for name lookup
    struct MM *mm;                          // 8 - memory manager to use
    SYM_ID_T max_symid;                     // 4
    SYM_ID_T next_symid;                    // 4
    RP next_rp;                             // 4
    RP max_rp;                              // 4
};


pub struct SM * SM_create(struct MM*);
pub int SM_trash(struct SM *);
pub SYM_ID_T sm_id(struct SM *, char *);
pub char * sm_name(struct SM *, RP);
pub bool sm_id_le(struct SM *, SYM_ID_T a, SYM_ID_T b);
pub inline RP sm_id_2_RP(struct SM *, SYM_ID_T);

#endif // API_BK_SM_H
