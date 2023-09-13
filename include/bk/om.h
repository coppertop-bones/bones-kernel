#ifndef __BK_OM_H
#define __BK_OM_H "bk/om.h"

#include "bk.h"
#include "buckets.h"

struct OM {
    int something;
};

pub struct OM * om_create();
pub void om_trash(struct OM *);

#endif // __BK_OM_H
