#ifndef __BK_SM_H
#define __BK_SM_H "bk/sm.h"

#include "bk.h"
#include "ht.h"

struct sym {
    unsigned short n;
    char buf[];             // <<<< RP points here
};


// HT_STRUCT2(name, object_t, extravars)
HT_STRUCT2(symIdByName, u32, struct SM* sm;)


#define NA_SYM 0
#define SM_MAX_SYM_STORAGE 0x10000  /* 64k */


struct SM {
    char *names;                            // 8 - VM buffer of u16 length prefixed, null terminated utf8 strings for type name and sym interning
    RP *nameRpById;                         // 8 - array of name RP indexed by id
    u32 *sortOrderById;                     // 8 - array of sort_order indexed by id - slot0 is 1 if sorted, 0 if not sorted
    ht_struct(symIdByName) *symIdByName;    // 8
    unsigned int nameRpByIdSize;            // 4
    u32 next_sym_id;                        // 4
    RP next_name_rp;                        // 4
};


pub struct SM * sm_create();
pub void sm_trash(struct SM *);
pub u32 sm_id(struct SM *, char const * const);
pub char * sm_buf(struct SM *, RP);
pub bool sm_id_le(struct SM *, u32 a, u32 b);
pub inline RP sm_id_2_RP(struct SM *sm, u32 id);

#endif // __BK_SM_H
