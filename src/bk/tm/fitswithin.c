// ---------------------------------------------------------------------------------------------------------------------
// Copyright 2025 David Briant, https://github.com/coppertop-bones. Licensed under the Apache License, Version 2.0
//
// FITSWITHIN IMPLEMENTATION
// KEEPER REQUISITES: core
// ---------------------------------------------------------------------------------------------------------------------

#ifndef SRC_BK_TM_FITSWITHIN_C
#define SRC_BK_TM_FITSWITHIN_C "bk/tm/fitswithin.c"


#include "core.c"



int tm_fitsWithin(BK_TM *tm, btypeid_t a, btypeid_t b) {
    // should answer a tuple {cacheID, doesFit, tByT, distance}
    // tByT can just be a T sorted list (not worth doing a hash)
    if (a == b) return 1;
    if ((b == B_EXTERN) && (a & B_EXTERN)) return 1;
    if ((b == B_FN) && ((a & 0x7f) == B_FN)) return 1;
    if ((b == B_EXTERN_FN_PTR) && ((a & 0xffff) == B_EXTERN_FN_PTR)) return 1;
    if ((b == B_FN_PTR) && ((a & 0xff7f) == B_FN_PTR)) return 1;
    if ((b == B_CHAR_STAR) && ((a & 0xff7f) == B_CHAR_STAR)) return 1;
    if ((b == B_VOID_STAR) && ((a & 0xff7f) == B_VOID_STAR)) return 1;
    if ((a & 0x000000FF) == b) return 1;
    return 0;
}

#endif  // SRC_BK_TM_FITSWITHIN_C