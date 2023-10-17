// ---------------------------------------------------------------------------------------------------------------------
// keep constants, global structs, type defs here
// ---------------------------------------------------------------------------------------------------------------------

#ifndef __BK_BK_H
#define __BK_BK_H "bk/bk.h"


#include "../all.cfg"

#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdbool.h>


#define _4K 4096
#define _16K 16384
#define _64K 65536
#define _1M 1048576
#define _1GB 1073741824
#define _2GB 2147483648
#define _4GB 4294967296
#define _1TB 1099511627776


#ifndef bk_inline
#ifdef _MSC_VER
#define bk_inline __inline
#else
#define bk_inline inline
#endif
#endif /* bk_inline */


#ifndef bk_unused
#if (defined __clang__ && __clang_major__ >= 3) || (defined __GNUC__ && __GNUC__ >= 3)
#define bk_unused __attribute__ ((__unused__))
#else
#define bk_unused
#endif
#endif /* bk_unused */


typedef unsigned int RP;                // relative pointer
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long u64;


typedef unsigned int BTYPEID_T;        /* currently ls 18 bits for 256k types */
typedef unsigned char BMETATYPE_ID_T;   /* currently 4 bits for 16 metatypes */

#define NA_BTYPE 0
#define SYM_ID_T unsigned int
#define SM_NA_SYM 0


// error codes
typedef char* err;
#define ok 0


#endif   // __BK_BK_H