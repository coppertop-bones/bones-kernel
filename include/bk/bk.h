// ---------------------------------------------------------------------------------------------------------------------
// keep constants, global structs, type defs here
// ---------------------------------------------------------------------------------------------------------------------

#ifndef __BK_BK_H
#define __BK_BK_H "bk/bk.h"


#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdbool.h>
#include "../all.cfg"


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


typedef unsigned int RP;        // relative pointer
typedef unsigned int u32;
typedef unsigned short u16;
#define BTYPE_TYPE unsigned int // currently ls 18 bits for 256k types


typedef char* err;
#define ok 0


// symbol encoding
// size prefixed, utf8 sequence, from 0 to 255 bytes so 0 is effectively the null symbol - we "waste" one byte for
// null termination so standard c string functions can work - size prefix allows for slightly faster comparison as
// we check size first



#define SIGNAL(msg) return (err) join_txts(3, __FUNCTION__, ": ", msg);       /* __PRETTY_FUNCTION__, __FILE__, __LINE__, __FUNCTION__, __func__ */

//#define newtxtbuf(numbytes) (malloc((numbytes)+1))


// Quote Originally Posted by C99 Section 6.2.5 Paragraph 15 and note 35
// The three types char, signed char, and unsigned char are collectively called the character types. The
// implementation shall define char to have the same range, representation, and behavior as either signed
// char or unsigned char.
//
// CHAR_MIN, defined in <limits.h>, will have one of the values 0 or SCHAR_MIN, and this can be used to
// distinguish the two options. Irrespective of the choice made, char is a separate type from the other
// two and is not compatible with either.
//
// consider:
// strcmp - e.g. https://codebrowser.dev/glibc/glibc/string/strcmp.c.html


// size_t
//   - the type which is used to represent the size of objects in bytes, and returned from sizeof
//   - for example https://www.embedded.com/further-insights-into-size_t/


// https://embeddedgurus.com/stack-overflow/2008/06/efficient-c-tips-1-choosing-the-correct-integer-size/
typedef uint_fast8_t fu8;
typedef uint_fast16_t fu16;
typedef uint_fast32_t fu32;
typedef uint_fast64_t fu64;

typedef uint_least8_t lu8;
typedef uint_least16_t lu16;
typedef uint_least32_t lu32;
typedef uint_least64_t lu64;


//
//char* concatMsg(const char* str1, const char* str2){
//    char* result;
//    asprintf(&result, "%s%s", str1, str2);
//    return result;
//}



// # - single hash in macro puts the argument in quotes, ## creates a new symbol
// https://gcc.gnu.org/onlinedocs/cpp/Concatenation.html#Concatenation

// X macro - https://www.digitalmars.com/articles/b51.html, https://en.wikipedia.org/wiki/X_Macro

//#define IFNDEF \#ifndef
//#define DEFINE \#define
//#define INCLUDE \#include
//#define ENDIF \#endif
//
//
//#define J_INCLUDE(file_locator, guard_symbol) \
//    IFNDEF guard_symbol \
//    INCLUDE file_locator \
//    DEFINE guard_symbol \
//    ENDIF


#endif   // __BK_BK_H