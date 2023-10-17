## NEXT
- exclusions
- answer same PyBType for same id
- "t1", "t2" etc for unnamed types (dynamically generated for now)
- expose setNominalTo and setEnumTo (and maybe setSymTo?)
- add BTypeError - a subclass of TypeError. multidispatch means CantFindError rather than TypeError, which means BTypeError 
  indicates an error constructing the btype. BTypeError is nicer than BMetaTypeError. 
- add BEnumError


## RADAR


## MID
- unions, tuples, structs, seq, map, fn, overload, family, schemavars, recursion
- make above work with Python fitsWithin
- fitsWithin with cache in C
- rework minc types to use bk instead
- enums (e.g. useful for exclusions)
- le for syms  / dictionary sort syms with fast merge
- windows VM fns


## DREAMLAND
- implement other probes for hashtable
- use special values for tombstones rather than flags
- create a no delete hash table for syms and enums (doesn't even need tombstones)


# THINKING
## enums
- do we need temporary enums for groupby on an unclassified set of syms?


## DONE
- raise error if symid is out of bounds in Python
- nominal types
- fix OPEN raise an error to set python error before returning 0
- allocate 4GB of VM for names - protect the whole lot and unprotect as needed (general bucket and typename bucket)
- research fastest algo for sorting small lists of integers (for type lists) - AC's radix uses insertion sort for small arrays
- quick sort et al
- counting sort
- fast median
- research fastest algo for merging small sorted list into larger sorted list (e.g. sort new syms first) better than:
  - if can realloc
    - do so
    - starting at back do a stream merge in place
  - else
    - alloc
    - starting at either end do a stream merge copying to new memory
- typelists (uses sorting)
- intersection types
- revisit tdd tests
