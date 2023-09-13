#ifndef __BK_KERNEL_H
#define __BK_KERNEL_H "bk/kernel.h"

#include "sm.h"
#include "em.h"
#include "tm.h"
#include "om.h"

struct K {
    struct SM sm;
    struct EM em;
    struct TM tm;
    struct OM om;  // plural? or does an object manager have allocators?
};

pub struct K * k_create();
pub int k_trash(struct K *k);

#endif // __BK_KERNEL_H
