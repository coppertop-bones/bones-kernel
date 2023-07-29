// ---------------------------------------------------------------------------------------------------------------------
// Copyright 2025 David Briant, https://github.com/coppertop-bones. Licensed under the Apache License, Version 2.0
//
// BK - BONES KERNEL
//
// OVERVIEW
// Bones is a language inspired by q/kdb and Smalltalk. It is intended to be used by non-career programmers for
// high performance statistical operations and so includes support for immutable data structures, composable apis and
// multi-dispatch. It has a strong static type system that happily is amenable to inference and a memory manager that
// is conservative on stack, precise on heap and encourages differentiation between object generations.
//
// It is highly compatible with C, using the C-ABI internally (for the moment) and allows c structs to be used directly.
//
// Bones has not been designed to support agent style programming and some of the design decisions may thus hinder
// writing applications in it.
//
// FEATURES
// - ref counting for CoW immutability and GC
// - tracing GC, with opportunistic evacuation, object pinning, destructor callbacks
// - bones shares a simple AST (aka Reduced Syntax Tree) with a C compiler
// - function selection for multi-dispatch is done statically where possible
// - automatic region based memory management and user apis to initiate earlier collection (including unmanaged arenas)
//
// FUTURE DIRECTIONS
// - tooling such as interactive debugging, profiling, type inference, code gen etc
// - support itanium exception handling
//
// COMPONENTS
// EM - ENUM MANAGER - sets of symbols
// HT - HASH TABLE - utils for building hash tables, i.e. maps, sets etc
// K - KERNEL - global singleton
// MM - MEMORY MANAGER - automatic and manual management
// OS - OPERATING SYSTEM APIs
// QBE - RST to QBE generator
// RST - REDUCED SYNTAX TREE
// SM - SYMBOL MANAGER - i.e. interned strings with fast sorting
// TM - TYPE MANAGER
// TP - TEXT PAD - composable api for string manipulation
//
// DEPRECATED
// OM - OBJECT MANAGER - moving to MM
//
// GLOBAL DETAILS
// - mm cannot detect / handle encoded pointers
// - types (i.e. btypeid_t) are encoded into 18 bits
// - meta-types are encoded in 4 bits
// - slots are 16 bytes, objects are 32 bit meta data prefixed, thus a single slot can hold up to a 12 byte object
//
// NAMING
// register types are lower case
// agents are BK_PascalCase
// structs are either 'struct lowercase' or PascalCase
// ---------------------------------------------------------------------------------------------------------------------

#ifndef INC_BK_BK_H
#define INC_BK_BK_H "bk/bk.h"


#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>


#define _4K 0x1000          /* 4096 */
#define _8K 0x2000
#define _16K 0x4000         /* 16384 */
#define _32K 0x8000
#define _64K 0x10000       /* 65536 */
#define _128K 0x20000
#define _256K 0x40000
#define _512K 0x80000
#define _1M 0x100000       /* 1048576 */
#define _2M 0x200000
#define _4M 0x400000
#define _8M 0x800000
#define _16M 0x1000000
#define _32M 0x2000000
#define _64M 0x4000000
#define _128M 0x8000000
#define _256M 0x10000000
#define _512M 0x20000000
#define _1GB 0x40000000     /* 1073741824 */
#define _2GB 0x80000000     /* 2147483648 */
#define _4GB 0x100000000    /* 4294967296 */
#define _8GB 0x200000000
#define _16GB 0x400000000
#define _32GB 0x800000000
#define _64GB 0x1000000000
#define _128GB 0x2000000000
#define _256GB 0x4000000000
#define _512GB 0x8000000000
#define _1TB 0x10000000000    /* 1099511627776 */

#define MACOS_M1_PAGE_SIZE _16K
#define MACOS_M1_CACHE_LINE_SIZE 128

#define MACOS_X64_PAGE_SIZE _4K
#define MACOS_X64_CACHE_LINE_SIZE 64

#define WIN_X64_PAGE_SIZE _4K
#define WIN_X64_CACHE_LINE_SIZE 128


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


/* __PRETTY_FUNCTION__, __FILE__, __LINE__, __FUNCTION__, __func__ */
#define FN_NAME (char *)(__FUNCTION__)


typedef unsigned int    RP;             // relative pointer
//typedef unsigned char   u8;
typedef uint8_t         u8;
typedef char            m8;             // "byte" doesn't work in windows 10
typedef char *          mem;
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
typedef int             size32;
typedef size_t          usize;
typedef unsigned int    usize32;
//typedef uintptr_t       uptr;



#define sizeof(x)    (ptrdiff_t)sizeof(x)
#define bk_alignof(x)   (ptrdiff_t)_Alignof(x)
#define countof(a)   (sizeof(a) / sizeof(*(a)))
#define lengthof(cs)  (countof(cs) - 1)


#define s8(cs) (S8){(lengthof(cs)), (void *)cs}

typedef struct {
    size sz;
    char *cs;               // pointer to null terminated utf8
} S8;

typedef struct {
    unsigned long vtsz;     // 2 bits of meta (S8, slice, or sequence) followed by 62 bits of text length
    void *p;                // either pointer to utf8 (optionally null terminated) or a length prefixed list of TPN *
} TPN;

#define s8_sz(s) (size)(s.sz)

typedef u32 symid_t;
#define SM_NA_SYM 0


// error codes
typedef char* err;
#define BK_NO_ERROR 0


// the following control visibility of functions
#define pvt static
#define pub
#ifdef BK_EXPOSE_TDD
#define tdd
#else
#define tdd static
#endif


#endif   // INC_BK_BK_H