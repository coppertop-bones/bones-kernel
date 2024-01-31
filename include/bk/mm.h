// ---------------------------------------------------------------------------------------------------------------------
// MM - MEMORY MANAGER
//
// Supports:
//  Untracked arenas - client is responsible for evacuation and reset
//
// Region based management. Blocks (32KB) may contain small (<= 1 line sized, 1 to 16 slots) and medium (<= 8KB)
// objects. Although we use the term object we don't mean it in the sense of OO.
//
// Memory may be used in several modes. Untracked and tracked. Arena (bump) style allocation. Tracing, evacuation
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


#ifndef INC_BK_MM_H
#define INC_BK_MM_H "bk/mm.h"

#include "bk.h"


#define MM_SLOT_SIZE 16
#define MM_LINE_SIZE 256
#define MM_SMALL_OBJ_SIZE MM_LINE_SIZE
#define MM_LARGE_OBJ_SIZE _8K
#define MM_BLOCK_SIZE _32K


// Page Allocator - reserves VM, allocate integer number of pages, keeps track of dropped ones, protecting, reusing or
// returning then to os when desired, and reserves extra VM as necessary. can't move but could coalesce adjacent pages

// BM - Block Manager
typedef struct {
    int fred;
} BK_BM;


typedef struct {
    void *(* malloc)(size_t);
    void (* free)(void *);
    void *(* realloc)(void *, size_t);
} BK_MM;


struct Buckets {
    void *first_bucket;     // 8
    void *current_bucket;   // 8
    void *next;             // 8
    void *eoc;              // 8
    void *last_alloc;       // 8
    unsigned short nPages;  // 2
};

struct BucketsCheckpoint {
    void *current_bucket;   // 8
    void *next;             // 8
    void *eoc;              // 8
    void *last_alloc;       // 8
};

struct BucketHeader {
    void *next_chunk;       // 8
    void *eoc;              // 8
};

typedef struct Buckets Buckets;
typedef struct BucketsCheckpoint BucketsCheckpoint;
typedef struct BucketHeader BucketHeader;

tdd void * initBuckets(Buckets *a, size chunkSize);
tdd void * allocInBuckets(Buckets *a, size n, size align);
tdd void * reallocInBuckets(Buckets *a, void* p, size n, size align);
tdd void checkpointBuckets(Buckets *a, BucketsCheckpoint *s);
tdd void resetToCheckpoint(Buckets *a, BucketsCheckpoint *s);
tdd void cleanBuckets(void *first_bucket);
tdd void freeBuckets(void *first_bucket);
tdd unsigned long numBuckets(BucketHeader *first_bucket);
tdd int inBuckets(Buckets *a, void *p);
tdd int isAlive(Buckets *a, void *p);
tdd int isDead(Buckets *a, void *p);


pub BK_MM * MM_create();
pub int MM_trash(BK_MM *);

#endif // INC_BK_MM_H
