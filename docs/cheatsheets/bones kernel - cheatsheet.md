# BONES KERNEL


## SYMBOL MANAGER


## ENUM MANAGER


## MEMORY MANAGER

### CONTEXT

- non-career programmer language - memory management needs to be hideable from user
- global key-value store - naturally defines long lived objects
- no closures in bones nor C
- in bones blocks can be passed to control functions
- in bones immutable with CoW and a counting ref counting (count of one means we can mutate)
- in bones it is not possible to create cycles in data structures (all data is a DAG, i.e. diamonds are possible)
- need to be able to consume all the memory from the os and release it back instantly - experienced 105GB query on a 128GB machine
- strongly typed so can be precise on heap
- C ABI for funcion calls - unless can demonstrate worth implementing own internal ABI
- anticipating using Itanium exceptions
- single thread model presented to user (possible with an awareness that some operations are done in parallel)

### ASSUMPTIONS / BELIEFS
- object pools (e.g. Boehm-Demers-Weiser BDW) are non-cache local - slows mutator performance
- bump allocation is fast and local
- object moving helps free memory and increase locality
- in analytic interactive mode there is plenty of opportunity to clear up / reorganise memory
- in service mode (e.g. minecraft mod) not so sure
- rc style immix (LXR in particular) seems a good fit
- avoid branch misprediction as much as possible

### DECISIONS
- conservative on stack (precise on stack sounds hard)
- piggy back off the CoW write barrier
- avoid read barriers

### PROPOSED DESIGN
- VM buffer - over allocate huge buffer in VM and allow OS to allocate physical memory as and when required
- 32k blocks
- 256 byte lines - 16 x 16 byte slots, 16 byte aligned
- 8096 byte large object threshold - large objects managed separately
- per object metadata - prefix 4 bytes prior to c pointer
  - isFlat - contains no indirection (pointers or relative pointers)
  - isMedium - 256 (-4) < size <= 8k
  - isVariable - type knows how to calculate size, else type knows size
  - isTombstone - used when evacuating, overwrites object start with forwarding pointer
  - isData - e.g. double*, data maybe shared but size is stored in a containing structure
  - leaving 27 bits for ref count (e.g 4 bits) and btypeid (e.g. 23 bits -> 8M, 22 bits -> 4M)
- thus min size is 12 bytes and then in increments of 16 bytes
- medium object spans more than one line
- born dead optimisation
- block level metadate
  - object start bitmap - 2048 slots per block - 1 bit per object is 256 bytes per block - needed to implement 
    conservative on stack pointer identification (which may point into middle of object)
  - line used bitmap - 128 lines per block - 16 bytes - used to avoid allocating on top of existing objects when reusing blocks
- contiguous VM for whole of small and medium object heap - overallocate vm for heap so memory arithmatic is simple
- type can reserve an additional slot to hold actual length so flat opaque objects can be moved, non-flat opaque 
  objects have to be pinned
- block header - first 12 bytes in block cannot be used nor last 4 bytes for object storage - we can use those for our 
  purposes

- tombstones can only be removed when the full trace is complete?

arenas are slightly more efficient but results need evacuating before rolling back to a checkpoint
although bump allocation into a used block requires logic to avoid the live lines it may be more cache local than 
arenas - something to investigate

16 byte alignment leaves 20 bits free is we wanted pointer boxing, i.e.
`| XXXX XXXX XXXX XXXX | PPPP PPPP PPPP PPPP | PPPP PPPP PPPP PPPP | PPPP PPPP PPPP XXXX |`


### TERMS / NOMENCLATURE
- run - the count of contiguous free lines
- slot, line, block, small, medium, large, object map, line map
- type - a label of the static things we can say pre-runtime


### REFERENCES
immix2017 - "Immix: A Mark-Region Garbage Collector with Space Efficiency, Fast Collection, and Mutator Performance"


## TYPE MANAGER


## TEXT PAD
