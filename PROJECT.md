NEXT
- raise error if symid is out of bounds
- nominal types (aka Atoms)
- research fastest algo for merging small sorted list into larger sorted list (e.g. sort new syms first) better than:
  - if can realloc
    - do so
    - starting at back do a stream merge in place
  - else
    - alloc
    - starting at either end do a stream merge copying to new memory
- quick sort
- counting sort
- intersection types


RADAR
- fix OPEN raise an error to set python error before returning 0
- cross platform VM lib
- allocate 4GB of VM for names - protect the whole lot and unprotect as needed (general bucket and typename bucket)
- use special values for tombstones rather than flags
- create a no delete hash table for syms and enums (doesn't even need tombstones)
- le for syms
- research fastest algo for sorting small lists


MID
- unions, tuples, structs, seq, map, fn, overload, family, schemavars
- make above work with Python fitsWithin
- fitsWithin with cache in C
- rework minc types to use bk instead


DREAMLAND
- implement other probes for hashtable
