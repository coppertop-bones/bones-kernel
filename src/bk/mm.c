// ---------------------------------------------------------------------------------------------------------------------
// Copyright 2025 David Briant, https://github.com/coppertop-bones. Licensed under the Apache License, Version 2.0
//
// MM - MEMORY MANAGER
// ---------------------------------------------------------------------------------------------------------------------

// https://www.etalabs.net/overcommit.html - mmap - none, then mprotect read-write what you need
// mremap
// https://github.com/estraier/tkrzw/issues/11 - not on macos
// https://stackoverflow.com/questions/17197615/no-mremap-for-windows - use windows AWE


#ifndef __BK_MM_C
#define __BK_MM_C "bk/mm.c"

#include <stdlib.h>
#include "lib/os.c"
#include "../../include/bk/mm.h"
#include "pp.c"


pvt unsigned int PAGE_SIZE = 0;


pub BK_MM * MM_create() {
    BK_MM *mm = (BK_MM *) malloc(sizeof(BK_MM));
    mm->malloc = malloc;
    mm->realloc = realloc;
    mm->free = free;
    return mm;
}

pub int MM_trash(BK_MM *mm) {
    free(mm);
    return 0;
}

tdd void *_nextBucket(Buckets *a, size n, size align);
tdd void *_allocBucket(size size);


pub void * Buckets_init(Buckets *a, size chunkSize) {
    if (PAGE_SIZE == 0) {PAGE_SIZE = os_page_size();}
    a->first_bucket = 0;
    a->current_bucket = 0;
    a->next = 0;
    a->eoc = 0;
    a->nPages = (int)(chunkSize / PAGE_SIZE + (chunkSize % PAGE_SIZE > 0));
    return _nextBucket(a, 0, 1);
}

pub void * allocInBuckets(Buckets *a, size n, size align) {
    void* p;
    p = (mem)a->next + (align - ((size)a->next % align));
    if (((mem)p + n) > (mem)a->eoc) {
        p = _nextBucket(a, n, align);
        if (!p) return 0;
        p = (mem)a->next + (align - ((size)a->next % align));
    }
    a->last_alloc = p;
    a->next = (mem)p + n;
    return p;
}

pub void * reallocInBuckets(Buckets *a, void *p, size n, size align) {
    // OPEN: if we can't realloc the client should decide how much needs allocating
    if (!p  || p != a->last_alloc) return allocInBuckets(a, n, align);
    if (((mem)p + n) > (mem)a->eoc) {
        void *chunk = _nextBucket(a, n, align);
        if (!chunk) return 0;
        p = (mem)a->next + (align - ((size)a->next % align));
        a->last_alloc = p;
    }
    a->next = (mem)p + n;
    return p;
}

tdd void * _nextBucket(Buckets *a, size n, size align) {
    void *p;  BucketHeader *ch;
    if (!a->current_bucket) {
        // OPEN: allocate enough pages to hold size n aligned to align
        // which might mean fast forwarding to a big enough chunk
        p = a->first_bucket = a->current_bucket = _allocBucket(a -> nPages * PAGE_SIZE);
        if (!p) return 0;
    } else {
        ch = (BucketHeader *)a->current_bucket;
        p = ch->next_chunk;
        if (!p) {
            // OPEN: see above
            p = _allocBucket(a -> nPages * PAGE_SIZE);
            if (!p) return 0;
            ch->next_chunk = p;
        }
    }
    a->current_bucket = p;
    a->next = (mem) p + sizeof(BucketHeader);
    a->eoc = ((BucketHeader *)p)->eoc;
    return p;
}

tdd void * _allocBucket(size size) {
    void *p;  BucketHeader *ch;
    p = malloc(size);                              // OPEN: cache, page and set alignment options
    if (!p) return 0;
    ch = (BucketHeader *)p;
    ch->next_chunk = 0;
    ch->eoc = (mem)p + size - 1;
    return p;
}

pub void checkpointBuckets(Buckets *a, BucketsCheckpoint *s) {
    s->current_bucket = a->current_bucket;
    s->next = a->next;
    s->eoc = a->eoc;
    s->last_alloc = a->last_alloc;
}

pub void resetToCheckpoint(Buckets *a, BucketsCheckpoint *s) {
    a->current_bucket = s->current_bucket;
    a->next = s->next;
    a->eoc = s->eoc;
    a->last_alloc = s->last_alloc;
}

pub void cleanBuckets(void *first_bucket) {
    nyi("cleanBuckets");
}

pub void Buckets_finalise(Buckets *buckets) {
    void *current, *next;
    current = buckets->first_bucket;
    while (current) {
        next = *(void**)current;
        free(current);
        current = next;
    }
}

pub unsigned long numBuckets(BucketHeader *first_bucket) {
    if (!first_bucket) return 0;
    unsigned long n = 0;
    do {
        n++;
        first_bucket = (BucketHeader *)first_bucket->next_chunk;
    }
    while (first_bucket);
    return n;
}

pub int inBuckets(Buckets *a, void *p) {
    // answers true if p is in any bucket (dead or alive)
    nyi("inBuckets");
    return 0;
}

pub int isAlive(Buckets *a, void *p) {
    // answers true if p is alive in an owned bucket
    nyi("isAlive");
    return 0;
}

pub int isDead(Buckets *a, void *p) {
    // answers true if p is dead in am owned bucket
    nyi("isDead");
    return 0;
}


#endif // __BK_MM_C
