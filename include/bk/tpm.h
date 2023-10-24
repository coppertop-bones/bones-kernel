// tp - text pad
//
// why?
// we needed an easy way to construct string representations of objects, e.g. good error messages, type descriptions,
// etc
// we need a random access interface for the string DSL "{-1}: {-2} - exception {1} occurred", PP(Exception), FN_NAME, LINE_NO)
// for readability of the error reporting code - making it easy to write detailed error messages should lead to more
// 1) printf("%s", genErrorMessage(...)) leads to memory leaks
// 2) using malloc for dynamic buffers is slow and additionally error-prone as the responsibility for memory management
//    enmeshes the client with the service
// in the case of passing messages back to python the string is copied in the construction of a python string - this
// has already caused OPEN: issues in the code. e.g.
//         return PyErr_Format(PyExc_TypeError, "There are exclusion conflicts");  // OPEN: say which types are in conflict
//
// Arena style mm works well in this context - as seen in the minc code gen code

// Goals
// fast - zero copy where possible, minimise copying by using laziness
// easy -
// composable API - expressive and pithy

// analysis
// what are the pros / cons of
// txt (bones object, type prefixed, len prefixed, null-terminated utf-8) vs
// s8 {len, char*} pair

// s8 can be passed on the stack - as it is fixed len, txt* must be passed on stack
// both can be restricted to int max strings

// why not txt as
// {
//   {int len, int btypeid}
//   u8*
// }
// object allocator either allocated less 8 bytes to cover the possibility of len OR has to check the type of the next
// object!

// to construct a len prefixed you need to copy the string

// decisions
// use libc for rendering etc replacing as and when it makes sense
// allocator will be pvt to tp so om can be used later instead of buckets


#ifndef API_BK_TP_H
#define API_BK_TP_H "bk/tp.h"

#include "bk.h"
#include "mm.h"
#include "buckets.h"


struct TPM {
    Buckets *all_strings;
    BucketsCheckpoint cp;
    char *buf;
    size size;
    struct MM *mm;
};


pub struct TPM * TPM_create(struct MM *);
pub int TPM_trash(struct TPM *);

// OPEN replace FILE * with a handle
pub FILE * tpm_start(struct TPM *);
pub s8 tpm_finish(struct TPM *, FILE *);
pub void tpm_drop(struct TPM *);

//int tp_cmp(s8 a, s8 b);
pub s8 tpm_printf(struct TPM* , s8 format, ...);
pub void tpm_fprintf(struct TPM *, FILE *, s8 format, ...);


#endif // API_BK_TP_H



