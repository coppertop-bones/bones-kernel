// ---------------------------------------------------------------------------------------------------------------------
// Copyright 2025 David Briant, https://github.com/coppertop-bones. Licensed under the Apache License, Version 2.0
//
// MM - MEMORY MANAGER
// ---------------------------------------------------------------------------------------------------------------------


#ifndef INC_BK_TM_BTYPE_T_ENUM_H
#define INC_BK_TM_BTYPE_T_ENUM_H "bk/tm_btypeid_t_enum.h"

#include "tm.h"

#define TM_FIRST_VALID_BTYPEID 2

#if defined _APPLE_ || defined __MACH__
typedef enum : BTYPEID_T_TYPE {
#else
    typedef enum {
#endif
    B_NAT = 0,       // not-a-type - i.e. an error code, the uninhabitable set
    B_NEW = 1,       // new type - dummy value to tell type manager to allocate a new id
    B_NULL,          // empty set - not the same as not-a-type
    B_VOID,
    B_MEM,
    B_M8,
    B_M16,
    B_M32,
    B_M64,
    B_CHAR,         // implementation defined (poss with compiler flags)
    B_U8,
    B_U16,
    B_U32,
    B_U64,
    B_I8,
    B_I16,
    B_I32,
    B_I64,
    B_F32,
    B_F64,
    B_P,
    B_PP,
    B_PPP,
    B_RP8,
    B_RP16,
    B_RP32,
    B_RP64,
    B_LITINT,
    B_LITDEC,
    B_LITTXT,

    B_EXTERN,
    B_STATIC,
    B_AUTO,
    B_REGISTER,
    B_VARARGS,
    B_CONST,
    B_CONST_P,
    B_RESTRICT,
    B_VOLATILE,
    B_FN,
    B_CHAR_STAR,
    B_CHAR_CONST_STAR,
    B_CHAR_CONST_STAR_CONST,    // almost implies an alias onto / into some structure that cannot mutuate that structure
    B_VOID_STAR,
    B_FN_PTR,
    B_EXTERN_FN,
    B_EXTERN_FN_PTR,
    B_PTR,
    B_PTR_PTR,
    B_PTR_PTR_PTR,

    B_T,
    B_T1,       // aka B_TA
    B_T2,       // aka B_TB
    B_T3,       // aka B_TC
    B_T4,       // aka B_TD
    B_T5,       // aka B_TE
    B_T6,       // aka B_TF
    B_T7,       // aka B_TG
    B_T8,       // aka B_TH
    B_T9,       // aka B_TI
    B_T10,      // aka B_TJ
    B_T11,      // aka B_TK
    B_T12,      // aka B_TL
    B_T13,      // aka B_TM
    B_T14,      // aka B_TN
    B_T15,      // aka B_TO
    B_T16,      // aka B_TP
    B_T17,      // aka B_TQ
    B_T18,      // aka B_TR
    B_T19,      // aka B_TS
    B_T20,      // aka B_TT

    // we don't need to distinguish N from M or any other index so can just alias N as M, I, J, K etc

    B_N,
    B_N1,
    B_N2,
    B_N3,
    B_N4,
    B_N5,
    B_N6,
    B_N7,
    B_N8,
    B_N9,

    B_FIRST_UNRESERVED_TYPEID,

} btypeid_t2;

typedef BTYPEID_T_TYPE btypeid_t;

#define B_TA B_T1
#define B_TB B_T2
#define B_TC B_T3
#define B_TD B_T4
#define B_TE B_T5
#define B_TF B_T6
#define B_TG B_T7
#define B_TH B_T8
#define B_TI B_T9
#define B_TJ B_T10
#define B_TK B_T11
#define B_TL B_T12
#define B_TM B_T13
#define B_TN B_T14
#define B_TO B_T15
#define B_TP B_T16
#define B_TQ B_T17
#define B_TR B_T18
#define B_TS B_T19
#define B_TT B_T20

#endif  // INC_BK_TM_BTYPE_T_ENUM_H