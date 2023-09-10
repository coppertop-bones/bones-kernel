#ifndef __BK_KERNEL_H
#define __BK_KERNEL_H "bk/kernel.h"

#include "sm.h"
#include "em.h"
#include "tm.h"
#include "mm.h"

struct K {
    struct SM sm;
    struct EM em;
    struct TM tm;
    struct MM mm;
};

pub int K_init(struct K *k);
pub int K_shutdown(struct K *k);

#endif // __BK_KERNEL_H
