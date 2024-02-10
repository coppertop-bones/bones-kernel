#ifndef INC_BK_OM_H
#define INC_BK_OM_H "bk/om.h"

#include "bk.h"
#include "mm.h"
#include "tm.h"


// GOAL reuse as much memory as possible between traces - the trace is as slow as we get


// DESIGN
// STW for grey disambiguation - needs to be conservative

// LXR (because it is not conservative can reuse slot refcounts as line refcounts) - 4 bits per slot is starting to add overhead
// so rather than refcount the lines (which needs 5 bits per line) + the object refcount (header)

// object start map and size info in type allow conservative start and interior pointers

// if ref count line we need to inc / dec two numbers - one in header, one in block.

// 3 mechanisms to track refcounts and conservatively categorise potential pointers
// a) 4 bits per slot - 3 bit object refcount + 1 bit to identify object start, medium objects need handling too on allocation / deallocation (expensive?)
// b) 1 bit per slot + 1 byte per line as ref count + header ref count - ref counts need updating in two places
// c) 1 bit per slot + 1 bit per line + header ref count + separate process to mark lines post allocation (which allows deadish objects)

// going with c) - more memory (and cache) efficient, allows a degree of coalescing (assuming born dead although we
// do need to queue decrefs into a ring buffer and process them before marking a grey slot as white or black - due to our CoW needs)
// so allocate matrix, multiple by x, take diagonal, return

// CoW - inc, dec, queueDec, processDecs

// header ref counts are less likely to get stuck

// given our CoW mechanism we can't delay ref counting in the same way

// hot heap maybe better for temps than cold stack?

// greyStartLine, greyEndLine
// lastFreeLines - how many lines are available to allocate
// totalFreeLines
// fragmentation stats - blackWhite count (0 is completely free, 1 indicates free space after solid space, etc)

// small allocator - can heavily reuse partially filled blocks
// medium allocator - only consumes free blocks, unless out of memory, in which case can do an expensive scan for large enough run
// can over allocvar by bumping more than needed but slots are not marked as used :) reallocvar can then check

// scratchTrace - cooperative or just when block is full?
// scratchMove

// memory state - white (free), to check - grey (reserved / unknown), marked - black (used) and white (free)
// quick gc - uses ref counts to mark reserved memory as free or used
// clears line map (actually already clear) - from greyStart to greyEnd, for each object still alive, get size and mark lines as used
// killing an object updates greyStart and greyEnd
// allocate updates greyEnd

// when object is copied forwarding point is left - all slots except first can be marked as free
// gc when hot should be faster than gc when cold


// block size is 32K (4 x max object size) - "bounding worst-case block-level fragmentation to about 25%.", section "Block and Line Size", p5, immix2017


// to relocate an object - must be done in a STW pause
// copy object - count = 1
// set old header to forwarded
// overwrite 1st 8 bytes of old object location with point to new location


// ref count 2 bits per object -> 32 bits per line,
// pin the line?


// combine the object map and the refcount here
// object map is 3 bits per 16 bytes - 0, 1, 2, 3

// "To work with the Immix heap organization, it counts live objects on each line"
// section "RC Immix", p6, Fast Conservative Garbage Collection

// "When processing ambiguous references, the collector consults the object map, discarding any reference that does
// not point to the start of a potentially live object."
// section "Object Map Filtering", p6, Fast Conservative Garbage Collection

// small object < 1 line
// section "Conservative Marking", p5, immix2017

// when placing an object in the global key value stored we can move it, or just mark it


// may want to hint the size
// scratch_alloc(om, btypeid) - create dead        amount alloc'd is figured from the type
// scratch_valloc(om, btypeid, count, hint) - create dead     amount alloc'd is figured from the type
// kvs_alloc(om, btypeid) - create live

// scratch_malloc(om, size, align)



// scenario mmul - A.B + (C.D + E.F) inplace as much as possible.   alloc space for A.B, scratch, alloc space for C.D, inplace add
// reuse space
// A.B + ((C.D) + (E.F))
// alloc for A.B
// alloc for C.D
// alloc for E.F



// write allocator - rc meta, btypeid, allocate
// conservative roots

typedef struct {
    u8 bytes[MM_SLOT_SIZE];
} OMSlot;

typedef struct {
    u8 bytes[MM_LINE_SIZE];
} OMLine;

struct OMBlock {

};

struct OMAlloctor {
    OMSlot *next;
    OMSlot *endOf;
};

typedef struct {
    BK_MM *mm;
    BK_TM *tm;
    char *rcs;      // ref counts
    void *start;
    void *endOf;
    struct OMAlloctor *alloctor;
} OM;

pub OM * OM_create(BK_MM *, BK_TM *);
pub void OM_trash(OM *);

pub void * om_alloc(OM *, btypeid_t);
pub void * om_valloc(OM *, btypeid_t, size);
pub void * om_revalloc(OM *, btypeid_t, size);
pub int om_count(OM *, void *);
pub int om_inc(OM *, void *);
pub int om_dec(OM *, void *);
pub int om_btypeid(OM *, void *);
pub int om_is(OM *, void *);



#define OM_BLOCKS_TOTAL_4GB _4GB
#define OM_SLOTS_TOTAL_4GB _64M
#define OM_LINES_TOTAL_4GB _2M

#define OM_BLOCKS_TOTAL_8GB _8GB
#define OM_SLOTS_TOTAL_8GB _128M
#define OM_LINES_TOTAL_8GB _4M

#define OM_BLOCKS_TOTAL_16GB _16GB
#define OM_SLOTS_TOTAL_16GB _256M
#define OM_LINES_TOTAL_16GB _8M

#define OM_BLOCKS_TOTAL_32GB _32GB
#define OM_SLOTS_TOTAL_32GB _512M
#define OM_LINES_TOTAL_32GB _16M

#define OM_BLOCKS_TOTAL_64GB _64GB
#define OM_SLOTS_TOTAL_64GB _1GB
#define OM_LINES_TOTAL_64GB _32M

#define OM_BLOCKS_TOTAL_128GB _128GB
#define OM_SLOTS_TOTAL_128GB _2GB
#define OM_LINES_TOTAL_128GB _64M

#define OM_BLOCKS_TOTAL_256GB _256GB
#define OM_SLOTS_TOTAL_256GB _4GB
#define OM_LINES_TOTAL_256GB _128M


#endif // INC_BK_OM_H
