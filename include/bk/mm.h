#ifndef API_BK_MM_H
#define API_BK_MM_H "bk/mm.h"

#include "bk.h"

struct MM {
    void *(* malloc)(size_t);
    void (* free)(void *);
    void *(* realloc)(void *, size_t);
};

// Page Allocator - reserves VM, allocate integer number of pages, keeps track of dropped ones, protecting, reusing or
// returning then to os when desired, and reserves extra VM as necessary. can't move but could coalesce adjacent pages

//struct PA {
//
//};


pub struct MM * MM_create();
pub int MM_trash(struct MM *);

#endif // API_BK_MM_H
