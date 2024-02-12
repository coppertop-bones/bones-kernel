// ---------------------------------------------------------------------------------------------------------------------
// MM - MEMORY MANAGER
// ---------------------------------------------------------------------------------------------------------------------


#ifndef INC_BK_MM_H
#define INC_BK_MM_H "bk/mm.h"

#include "bk.h"

// TODO
//   make buckets use block manager
//   review
//      review the matrix mul by mutation example
//      https://danluu.com/malloc-tutorial/
//      https://github.com/zyfjeff/C-HOW-TO/blob/master/c-malloc/Malloc_tutorial.pdf
//      https://manybutfinite.com/post/anatomy-of-a-program-in-memory/
//      https://goog-perftools.sourceforge.net/doc/tcmalloc.html
//      https://github.com/jemalloc/jemalloc
//      https://wiki-prog.infoprepa.epita.fr/images/0/04/Malloc_tutorial.pdf
//      https://github.com/sailfish009/malloc   doug lea's also https://sourceware.org/git/?p=glibc.git;a=blob;f=malloc/malloc.c
//      https://moss.cs.iit.edu/cs351/slides/slides-malloc.pdf
//      https://github.com/microsoft/mimalloc  microsoft's me malloc
//      https://news.ycombinator.com/item?id=20249743 mimalloc experience at clickhouse and more
//      https://github.com/spaskalev/buddy_alloc
//      https://www.gingerbill.org/series/memory-allocation-strategies/
//      https://softwareengineering.stackexchange.com/questions/303666/merits-of-copy-on-write-semantics
//      https://igoro.com/archive/gallery-of-processor-cache-effects/
//      https://sploitfun.wordpress.com/2015/02/10/understanding-glibc-malloc/
//      https://www.gingerbill.org/article/2020/06/21/the-ownership-semantics-flaw/
//      https://www.gingerbill.org/article/2020/05/17/relative-pointers/
//      https://www.gingerbill.org/article/2020/01/25/a-reply-to-lets-stop-copying-c/
//      https://github.com/olemorud/arena-allocator
//      https://nullprogram.com/blog/2023/09/27/    arenas s8 chap
//      https://nullprogram.com/blog/2023/10/08/
//      https://www.rfleury.com/p/untangling-lifetimes-the-arena-allocator
//      https://lobste.rs/s/sutuvh/arena_allocator_tips_tricks
//      https://www.hboehm.info/gc/
//      https://www.memorymanagement.org/
//      https://github.com/xoofx/gcix
//      https://www.youtube.com/watch?v=x-CBUQxp1vE&t=1203s rc immix talk
//   write api for arena and region mm
//   https://joeduffyblog.com/2016/02/07/the-error-model/



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



// objects may be agents or values
// meta:
// isAgent



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
