// ---------------------------------------------------------------------------------------------------------------------
// MM - MEMORY MANAGER
//
// Supports:
//  Untracked arenas - client is responsible for evacuation and reset
//
// Region based management. Blocks (32KB) may contain small (<= 1 line sized, 1 to 16 slots) and medium (<= 8KB)
// objects. Although we use the term object we don't mean it in the sense of OO.
//
// Memory may be used in several modes. Untracked and tracked. Arena (bump) style allocation. Tracing, evaculatoin
//
// Features include:
//  - born dead optimisation
//  - ref counted CoW (to assist destructive updates)
//  - conservative on stack - pointer to start, pointer within
//  - pinning
//  - call backs on death
//  - generational
//
// Can't handle encoded pointers (but may be able to handle
// ---------------------------------------------------------------------------------------------------------------------


#ifndef API_BK_MM_H
#define API_BK_MM_H "bk/mm.h"

#include "bk.h"


#define MM_SLOT_SIZE 16
#define MM_LINE_SIZE 256
#define MM_SMALL_OBJ_SIZE MM_LINE_SIZE
#define MM_LARGE_OBJ_SIZE _8K
#define MM_BLOCK_SIZE _32K



typedef struct {
    void *(* malloc)(size_t);
    void (* free)(void *);
    void *(* realloc)(void *, size_t);
} BK_MM;

// Page Allocator - reserves VM, allocate integer number of pages, keeps track of dropped ones, protecting, reusing or
// returning then to os when desired, and reserves extra VM as necessary. can't move but could coalesce adjacent pages

//struct PA {
//
//};


pub BK_MM * MM_create();
pub int MM_trash(BK_MM *);

#endif // API_BK_MM_H
