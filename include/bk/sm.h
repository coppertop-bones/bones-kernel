#ifndef __BK_SM_H
#define __BK_SM_H "bk/sm.h"

#include "bk.h"

struct SM {
    char *istrings;         // VM buffer of u16 length prefixed, null terminated utf8 strings for type name and sym interning
    RP next;                // ensure is u16 aligned
    RP hash_size;
    RP *istring_rp_hash;    // hash to get the rp for a string
    bool sorted;
};

pub struct SM * sm_create();
pub void sm_trash(struct SM *);
pub RP sm_sym(struct SM *, char const * const name);
pub char * sm_get(struct SM *, RP symRp);
pub bool sm_le(struct SM *, RP a, RP b);

#endif // __BK_SM_H
