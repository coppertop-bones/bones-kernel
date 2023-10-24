#ifndef API_BK_OM_H
#define API_BK_OM_H "bk/om.h"

#include "bk.h"
#include "buckets.h"
#include "mm.h"

struct OM {
    int something;
};

pub struct OM * OM_create(struct MM *);
pub void OM_trash(struct OM *);

#endif // API_BK_OM_H
