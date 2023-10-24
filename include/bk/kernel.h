#ifndef API_BK_KERNEL_H
#define API_BK_KERNEL_H "bk/kernel.h"

#include "mm.h"
#include "sm.h"
#include "em.h"
#include "tm.h"
#include "om.h"
#include "tm.h"

struct K {
    struct MM *mm;
    struct SM *sm;
    struct EM *em;
    struct TM *tm;
    struct TPM *tp;
//    struct OM om;  // plural? or does an object manager have allocators?
};

pub struct K * K_create(struct MM *mm);
pub int K_trash(struct K *k);

#endif // API_BK_KERNEL_H

