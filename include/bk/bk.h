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


typedef unsigned int    RP;                // relative pointer
//typedef unsigned char   u8;
typedef uint8_t         u8;
typedef char            byte;
typedef unsigned short  u16;
//typedef char16_t      c16;
//typedef unsigned int   u32;
typedef uint32_t        u32;
typedef int32_t         i32;
typedef int32_t         b32;
//typedef unsigned long   u64;
typedef uint64_t        u64;
typedef float           f32;
typedef double          f64;
typedef ptrdiff_t       size;
typedef size_t          usize;
//typedef uintptr_t       uptr;



#define sizeof(x)    (size)sizeof(x)
#define alignof(x)   (size)_Alignof(x)
#define countof(a)   (sizeof(a) / sizeof(*(a)))
#define lengthof(s)  (countof(s) - 1)



typedef unsigned int BTYPEID_T;        /* currently ls 18 bits for 256k types */
typedef unsigned char BMETATYPE_ID_T;   /* currently 4 bits for 16 metatypes */

#define NA_BTYPE 0
#define SYM_ID_T unsigned int
#define SM_NA_SYM 0


// error codes
typedef char* err;
#define ok 0


#endif   // __BK_BK_H