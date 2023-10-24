#ifndef API_BK_MM_H
#define API_BK_MM_H "bk/mm.h"

#include "bk.h"

struct MM {
    void *(* malloc)(size_t);
    void (* free)(void *);
    void *(* realloc)(void *, size_t);
};

pub struct MM * MM_create();
pub int MM_trash(struct MM *);

#endif // API_BK_MM_H

