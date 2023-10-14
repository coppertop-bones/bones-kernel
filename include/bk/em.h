#ifndef __BK_EM_H
#define __BK_EM_H "bk/em.h"

#include "bk.h"
#include "mm.h"
#include "sm.h"

struct EM {
    struct MM *mm;
    struct SM *sm;
    int something;
};

pub struct EM * EM_create(struct MM *, struct SM*);
pub int EM_trash(struct EM *);

// sort order stuff

#endif // __BK_EM_H
