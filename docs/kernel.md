## kernel

syms \
enums \
mm \
tm

VM buffer - over allocate buffer in VM and allow OS to allocate physical memory as and when required \
ID is an index into an array (fixed size elements) \
RP is a u32 offset into a buffer (variable size elements) - Relative Pointer


### syms
istrings - VM buffer of u16 length prefixed, null terminated utf8 strings for type name and sym interning - RP indexed \
istring_rp_hash - hash to get the rp for a string \
OPEN: in the future we should split type names from other syms? \
OPEN: read up on how other systems do interning


### enums
map syms to enums (linear probe to start with, maybe hash later) \
maintain local id and sort order


### bk object algo

PLAN
1. buckets - untracked memory
   1.1 initialise blocks and buckets + meta data + linked list of unused blocks
   1.2 bucket_alloc - grabs enough contiguous blocks to fulfill request, grab_blocks, return_blocks, zero_blocks, release_blocks
   1.3 test that the string gen still works (and maybe minc)
   1.4 complete buckets or at min mark all open issues
2. scratch - user initiated tracking
    2.1 also init object start bit map and line map
    2.2 snap_scratch (ansswers a checkpoint), scratch_alloc
        2.2.1 add size to type
        2.2.2 add structs or tuples so can trace precise
        2.2.3 need object header, might as well to rc fns too
    2.3 save_these(roots), reset_to_snap (so not to disrupt caller's memory
    
managed - user puts ptrs into kv store

on OOM - stop the world and figure gaps


we use region based memory management
32k block divided into 256 byte lines divided into 16 byte slots
a small object may cover up to 16 slots or 1 line
a medium object may cover up to 8k
we keep an object (start) bitmap (1 bit for each slot)
    - we need this to implement conservative on stack pointer identification
    - 1 bit * (32k / 16 bytea) of memory per block for the object start map (256 bytes)
we keep a line used bitmap (1 bit for each line that has object data in it)
    - this is used to avoid allocating on top of existing objects when reusing blocks

we bump allocate small objects into fresh or ready blocks (user can choose fresh for locality)
we bump allocate medium objects into fresh blocks (rather than searching for space)
large objects use a separate allocator - details to be decided - compacting? prob just a very big amount of vm
we bump allocate user sized objects into an arena - object map and line maps do not need to be maintained

object header
objects are 16 byte aligned and prefixed with the header (i.e 4 bytes preceding the object start)
prefixing is chosen for better c-abi compatability and to help keep bkheader private (to minimise breaking changes)
12 bytes is largest 1 slot object, 28 bytes is largest 2 slot object, etc.
we can reuse / reserve the first 12 bytes, and last 4 bytes of a block for kernel usage

16 byte alignment leaves 20 bits free is we wanted pointer boxing, i.e. 
`| XXXX XXXX XXXX XXXX | PPPP PPPP PPPP PPPP | PPPP PPPP PPPP PPPP | PPPP PPPP PPPP XXXX |`

created untracked
object starts must always be marked, lines are not marked on allocation (i.e. they will be made live / tracked later on)
before a fresh block can be reused its lines must be marked - happens on trace (either stw or user initiated)

user managing scratch
allocates objects - has a number of fresh blocks and wants to reuse them, traces roots and resets allocators to make them ready
next wave of allocations will avoid live objects
when should we evacuate

object dealloc (rc -> 0)
    - can't change line map for small objects as that line may be use by other small objects
    - for medium objects can't zero first and last lines as maybe used by other objects but can zero lines between

zeroing

collection
tracing
    - mark all lines in block as free
    - traverse roots marking lines with small object starts and medium object extents (thus the need for 1 line space)
    - reset RC to 1 when marking starts of precisely known objects
    - can also reset object counts when tracing

conservative 

object header
type
refcount
isPinned - 1 bit
isConservativelyPinned ? hard to reset
size (small, medium or large) - 2 bits
isAgg (i.e. an exponential type) - 1 bit

OUTSTANDING - how to reset RC - could apply mask to all headers in a block


block header
hasPinned


terms
allocate - to create an object
evacuate - to copy an object into fresh block, marking the old place as a tombstone and overwriting the first 8 bits 
    with a forwarding pointer
compact (we don't do this) - moving objects within a block

fresh block - a block that hasn't been recycled - thus we know the remaining memory is unused and contiguous
ready block - we bump allocate into a block that has live / tracked objects in it avoiding lines with those live objects
dead block - has no live objects but has not been zeroed
clean block - has no live objects and has been zeroed

conservative start - start only is checked
conservative within - start and length are checked - ptr within vm region extent, find block, is ptr an object start, 
    scan back to prior object start, get size, is ptr within object extent

we evacuate:
    - on a stop the world trace (exact heap roots provided and conservatively assumed roots from stack)
    - user initiated trace (exact roots provided)






buckets are supposed to be fast on allocation and deallocation
implemented as a singly linked list of buckets, state can be saved and reset thus deallocated en-mass
buckets can be cleaned for security
the last allocation can be resized to be bigger or smaller useful when required size is unknown upfront
Buckets can fit into a cache line e.g. 64 bytes
Buckets can be used to get a tmp buffer
void *buf = allocInBuckets(all_strings, n:1000, align:1);
...
reallocInBuckets(all_strings, buf, 0, align:1);



// how do we move a double*? -  mm must keep size or we have to push responsibility for moving onto containing stuct
// struct matrix {int N; int M; data double*;} but to create a view we want to share the double* so the double*
// must be ref counted and sized

//    8k medium object max is 8192 - 512 * 16 slots - 9 bits

