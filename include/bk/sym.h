#ifndef __BK_SYM_H
#define __BK_SYM_H "bk/sym.h"

#include "bk.h"

struct SM {
    char *istrings;         // VM buffer of u16 length prefixed, null terminated utf8 strings for type name and sym interning
    RP hash_size;
    RP *istring_rp_hash;    // hash to get the rp for a string
};

pub RP sym(char const * const name);
// sort order stuff

#endif // __BK_SYM_H
