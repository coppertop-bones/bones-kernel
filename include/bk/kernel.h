#ifndef __BK_KERNEL_H
#define __BK_KERNEL_H "bk/kernel.h"

#include "sym.h"
#include "em.h"
#include "tm.h"
#include "mm.h"

struct K {
    struct SM sm;
    struct EM em;
    struct TM tm;
    struct MM mm;
};


#endif // __BK_KERNEL_H
