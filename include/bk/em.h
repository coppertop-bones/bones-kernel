// ---------------------------------------------------------------------------------------------------------------------
// Copyright 2025 David Briant, https://github.com/coppertop-bones. Licensed under the Apache License, Version 2.0
//
// EM - ENUM MANAGER
// ---------------------------------------------------------------------------------------------------------------------

#ifndef INC_BK_EM_H
#define INC_BK_EM_H "bk/em.h"

#include "bk.h"
#include "mm.h"
#include "sm.h"

typedef struct {
    BK_MM *mm;
    BK_SM *sm;
    int something;
} BK_EM;

pub BK_EM * EM_create(BK_MM *, BK_SM*);
pub int EM_trash(BK_EM *);

// sort order stuff

#endif // INC_BK_EM_H
