NEXT
- research fastest algo for merging small sorted list into larger sorted list (e.g. sort new syms first) better than:
  - if can realloc
    - do so
    - starting at back do a stream merge in place
  - else
    - alloc
    - starting at either end do a stream merge copying to new memory
- quick sort
- counting sort
- dictionary sort syms
- research fastest algo for sorting small lists of integers (for type lists)
- intersection types


RADAR
- cross platform VM lib
- use special values for tombstones rather than flags
- create a no delete hash table for syms and enums (doesn't even need tombstones)
- le for syms
- exclusions (do we need enums?)


MID
- unions, tuples, structs, seq, map, fn, overload, family, schemavars, recursion
- make above work with Python fitsWithin
- fitsWithin with cache in C
- rework minc types to use bk instead


DREAMLAND
- implement other probes for hashtable


DONE
- raise error if symid is out of bounds in Python
- nominal types
- fix OPEN raise an error to set python error before returning 0
- allocate 4GB of VM for names - protect the whole lot and unprotect as needed (general bucket and typename bucket)
