#ifndef __BK_EM_H
#define __BK_EM_H "bk/em.h"

#include "bk.h"
#include "sm.h"

struct EM {
    int something;
};

pub struct EM * em_create();
pub void em_trash(struct EM *);

// sort order stuff

#endif // __BK_EM_H
