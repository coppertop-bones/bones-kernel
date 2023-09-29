#ifndef __BK_SM_H
#define __BK_SM_H "bk/sm.h"

#include "bk.h"
#include "ht.h"


// this is more for documentation than usage to make clear that a sym is length prefixed
struct sym {
    unsigned short n;
    char buf[];             // <<<< RP points here
};


// following names are easy to find in auto complete as they all start with SM_
#define SM_SYM_ID_T u32
#define SM_NA_SYM 0
#define SM_MAX_NAME_LEN 0xFF                                    /* DTM: symbols can be up to 255 bytes of utf8 inc null termination - can be increased */
#define SM_MAX_NAME_STORAGE 0xFFFFFFFF                          /* DTM: 4GB is max addressable by SM_SYM_ID_T and vm space is cheap */
#define SM_ID_ARRAY_INC_SIZE (0x4000 / sizeof(SM_SYM_ID_T))     /* DTM: i.e. 1 page on macos M1, 4 pages on windows intel */
#define SM_SYMS_NOT_SORTED 0
#define SM_SYMS_SORTED 1

#define SM_ERR_NAME_TOO_LONG 1
#define SM_ERR_NAME_TOO_SHORT 2
#define SM_ERR_OUT_OF_NAME_STORAGE 3


// HT_STRUCT2(name, slot_t, extravars)
HT_STRUCT2(symIdByName, u32, struct SM* sm;)


struct SM {
    char *names;                            // 8 - VM buffer of u16 length prefixed, null terminated utf8 strings for type name and sym interning
    RP *nameRpById;                         // 8 - array of name RP indexed by id
    u32 *sortOrderById;                     // 8 - array of sort_order indexed by id - slot0 is 1 if sorted, 0 if not sorted
    ht_struct(symIdByName) *symIdByName;    // 8 - hash table for name lookup
    unsigned int nameRpByIdSize;            // 4
    SM_SYM_ID_T next_sym_id;                // 4
    RP next_name_rp;                        // 4
    RP max_rp;                              // 4
};


pub struct SM * sm_create();
pub void sm_trash(struct SM *);
pub u32 sm_id(struct SM *, char const * const);
pub char * sm_name(struct SM *, RP);
pub bool sm_id_le(struct SM *, SM_SYM_ID_T a, SM_SYM_ID_T b);
pub inline RP sm_id_2_RP(struct SM *sm, SM_SYM_ID_T id);

#endif // __BK_SM_H
