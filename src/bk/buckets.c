#ifndef __BK_BUCKETS_C
#define __BK_BUCKETS_C "bk/buckets.c"


#include "../../include/all.cfg"
#include <stdlib.h>
#include "../../include/bk/buckets.h"
#include "../lib/os.c"
#include "pp.c"

pvt unsigned int PAGE_SIZE = 0;

tdd void *_nextBucket(Buckets *a, unsigned int n, unsigned int align);
tdd void *_allocBucket(size_t size);


pub void * initBuckets(Buckets *a, unsigned long chunkSize) {
    if (PAGE_SIZE == 0) {PAGE_SIZE = os_page_size();}
    a->first_bucket = 0;
    a->current_bucket = 0;
    a->next = 0;
    a->eoc = 0;
    a->nPages = chunkSize / PAGE_SIZE + (chunkSize % PAGE_SIZE > 0);
    return _nextBucket(a, 0, 1);
}

pub void * allocInBuckets(Buckets *a, unsigned int n, unsigned int align) {
    void *p;
    p = a->next + (align - ((unsigned long)a->next % align));
    if ((p + n) > a->eoc) {
        p = _nextBucket(a, n, align);
        if (!p) return 0;
        p = a->next + (align - ((unsigned long)a->next % align));
    }
    a->last_alloc = p;
    a->next = p + n;
    return p;
}

pub void * reallocInBuckets(Buckets *a, void* p, unsigned int n, unsigned int align) {
    if (!p  || p != a->last_alloc) return allocInBuckets(a, n, align);
    if ((p + n) > a->eoc) {
        void *chunk = _nextBucket(a, n, align);
        if (!chunk) return 0;
        p = a->next + (align - ((unsigned long)a->next % align));
        a->last_alloc = p;
    }
    a->next = p + n;
    return p;
}

tdd void * _nextBucket(Buckets *a, unsigned int n, unsigned int align) {
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
    a->next = p + sizeof(BucketHeader);
    a->eoc = ((BucketHeader *)p)->eoc;
    return p;
}

tdd void * _allocBucket(size_t size) {
    void *p;  BucketHeader *ch;
    p = malloc(size);                              // OPEN: cache, page and set alignment options
    if (!p) return 0;
    ch = (BucketHeader *)p;
    ch->next_chunk = 0;
    ch->eoc = p + size - 1;
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

pub void freeBuckets(void *first_bucket) {
    void *current, *next;
    current = first_bucket;
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


#endif  // __BK_BUCKETS_C