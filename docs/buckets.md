buckets are supposed to be fast on allocation and deallocation
implemented as a singly linked list of buckets, state can be saved and reset thus deallocated en-mass
buckets can be cleaned for security
the last allocation can be resized to be bigger or smaller useful when required size is unknown upfront
Buckets can fit into a cache line e.g. 64 bytes
Buckets can be used to get a tmp buffer
void *buf = allocInBuckets(all_strings, n:1000, align:1);
...
reallocInBuckets(all_strings, buf, 0, align:1);

