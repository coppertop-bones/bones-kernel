#ifndef API_BK_BUCKETS_H
#define API_BK_BUCKETS_H "bk/buckets.h"

#include "bk.h"

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


#endif // API_BK_BUCKETS_H
