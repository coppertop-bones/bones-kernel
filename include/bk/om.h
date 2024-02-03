#ifndef INC_BK_OM_H
#define INC_BK_OM_H "bk/om.h"

#include "bk.h"
#include "mm.h"
#include "tm.h"



// PLAN
// bones needs pointers (*), C needs **?
// structs (or tuples)
// struct (and tuple) layout with padding, need records for compact layout
// sequences (to test reallocvar)
// Python interface will make this usable for Cluedo thus making it more interesting


// M1
// high-performance cores
//      192 KB of L1 instruction cache
//      128 KB of L1 data cache
//      12 MB L2 cache
// energy-efficient cores
//      128 KB L1 instruction cache
//      64 KB L1 data cache
//      4 MB L2 cache


// GOAL reuse as much memory as possible between traces - the trace is as slow as we get


// DESIGN
// STW for grey disambiguation - needs to be conservative
// slot map in total marking the starting point of allocated objects - 2048 slots per block - 1 bit per object 256 bytes per block
// contiguous for whole of small and medium object heap - overallocate vm for heap so memory arithmatic is simple
// line map per block - 128 lines per block - 16 bytes - in per block metadata - consider cache locality

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

// object header - 32 bits
// isFlat - contains no indirection (pointers or relative pointers)
// isMedium - 256 (-4) < size <= 8k
// isVariable - type knows how to calculate size, else type knows size
// isTombstone (used only when evacuating)
// isData - e.g. double*, data maybe shared but size is stored in a containing structure
// leaving 27 bits for ref count (e.g 4 bits) and btypeid (e.g. 23 bits)

// block header - first 12 bytes in block cannot be used nor last 4 bytes for object storage
// greyStartLine, greyEndLine
// lastFreeLines - how many lines are available to allocate
// totalFreeLines
// fragmentation stats - blackWhite count (0 is completely free, 1 indicates free space after solid space, etc)

// tombstones can only be removed when the full trace is complete?

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


// NOMENCLATURE
// run - the count of contiguous free lines
// slot, line, block, small, medium, large, object map
// type a label of the static things we can say pre-runtime



// immix2017 - "Immix: A Mark-Region Garbage Collector with Space Efficiency, Fast Collection, and Mutator Performance"

// cache line size is 128 bytes on macos

// line size is 256 bytes (16 objects)
// object size is 16 bytes, align 16
// max size is 8K
// block size is 32K (4 x max object size) - "bounding worst-case block-level fragmentation to about 25%.", section "Block and Line Size", p5, immix2017
// btypeid_t is 4 bytes 3 bytes would be 16M types - 8 bits meta

// header flags
// isPinned
// small - 0 is small, 1 is medium
// forwarded


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





//slot - 16 bytes for object storage - 4 bytes meta before ptr (so can store a 12 byte object + 16 * n)
//line contains slots - e.g. 256 byte line (2 cache lines on macos holds 16 slots)
//
//small object fits in a line
//
//allocator has a block (or an allocator belongs to a block?)
//slot * next
//slot * endOf


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
