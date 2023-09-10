
each overload knows the number of args

signatures lookup is hit count sorted list of signature - could be a hash map in future i.e. probe, if missing then hash and probe


we only keep hits in the cache as cache misses mean "not found" which initiates a fitsWithin search, and then either
a type error is raised or the new hit is added to the cache
we want to count the number of hits so we can sort according to hits -> which hopefully will be faster overall

assuming that L1 & L2 cache misses are important for the dispatch we compact the signatures
a type can be BTL or BTU so the number of slots is 1 byte for header + 2 * num_args - BTL is 1 short, BTU is 2 shorts
we have 48 bits in a 1 arg signature however we only can rely on there being space in the first and last short
so we have 32 bits to play with


the following schema encodes types into a u16 array and can handle up to a type space of size 256k

type_nums[] is u16 aligned

N -> max size of sig in multiples of u16 -1 -> 5 bits up to 32 shorts (16 args without enforcing shorter type nums)
P -> payload (i.e. the function slot for this overload) - 16 bits so 64k possible functions
T -> btype (18 bits -> 256k types)
    BTL -> lower 15 bits
    BTU -> upper 3 bits
U -> is BTU type (1 bit)
H -> hit count (8 bits -> 256 hot hits before incrementing cold hit)



 PL / HC (payload / hit count)
 | PPPP PHHH HHHH H--- |

 PL / SS (payload / sig size)
 | PPPP PPPP PPPN NNNN |

 BTL
 | UTTT TTTT TTTT TTTT |

 BTU
 | 00000 000 0000 0TTT |


 e.g. 1 arg overloads
 |         MSB         |         ...         |         LSB         |

 |  PL / HC / A1-BTU   |       A1-BTL        |       PL / SS       |
 | PPPP PHHH HHHH HTTT | 1TTT TTTT TTTT TTTT | PPPP PPPP PPP0 0001 |

 |       PL / HC       |       A1-BTL        |       PL / SS       |
 | PPPP PHHH HHHH H000 | 0TTT TTTT TTTT TTTT | PPPP PPPP PPP0 0000 |


 e.g. 2 arg overloads
 |         MSB         |         ...         |         ...         |         ...         |         LSB         |

 |  PL / HC / A2-BTU   |       A2-BTL        |       A1-BTU        |       A1-BTL        |       PL / SS       |
 | PPPP PHHH HHHH HTTT | 1TTT TTTT TTTT TTTT | 00000 000 0000 0TTT | 1TTT TTTT TTTT TTTT | PPPP PPPP PPP0 0011 |

 |       PL / HC       |       A2-BTL        |       A1-BTU        |       A1-BTL        |       PL / SS       |
 | PPPP PHHH HHHH H000 | 0TTT TTTT TTTT TTTT | 00000 000 0000 0TTT | 1TTT TTTT TTTT TTTT | PPPP PPPP PPP0 0010 |

 |       PL / HC       |       A2-BTU        |       A2-BTL        |       A1-BTL        |       PL / SS       |
 | PPPP PHHH HHHH H000 | 00000 000 0000 0TTT | 1TTT TTTT TTTT TTTT | 0TTT TTTT TTTT TTTT | PPPP PPPP PPP0 0010 |

 |       PL / HC       |                     |       A2-BTL        |       A1-BTL        |       PL / SS       |
 | PPPP PHHH HHHH H000 | 0000 0000 0000 0000 | 0TTT TTTT TTTT TTTT | 0TTT TTTT TTTT TTTT | PPPP PPPP PPP0 0001 |


 OPEN: could reverse order of arguments in overload as last arg is likely to bre more type volatile than first arg?


 pSig vs replicating sig
 pSig is 8bytes - sig1 is 4 to 6 bytes, sig2 is 6 to 10 bytes, sig3 is 8 to 14 bytes
 fnId can be encoded in pSig and sig - fnId = 2 ^ 11 (2048 fns per overload), pSig could encode more
 *pSig may be a cache miss (+200 cycles), but sig[] is less likely to be a miss
 functions with many args are not likely to have lots of overloads so the space saved by pSig in real terms may be
 minimal compared to the locality gained

 encode function number in sigheader - in bits 15 to 5 - allowing a max of 2048 overloads, with 16 arguments





 changes to reflect into code
 numExtraArgs - 4 bits, 0 to 31 extra args
 probeHashEncoding - 4 bits, 0=>1, 1=>2, 2=>4, 3=>8, 4=>16, 5=>32, 6=>64,
 numExtra is additional args   - sig width (U16 count) is 1 + 2 * numArgs (min 1 arg)

u8 + u8 = 2
(3 * u16) * n = 6 * n
n is 1, 2, 4, 8

8 * 6 is 48
2 args 1 overload would be 5 * u16 + u8 + u8 = 12 bytes so fits in 1 object slot


typedef struct {
    unsigned char slot_width;                         // in count of TypeNum - 3 of a 1 arg, 5 for a 2 arg, ..., 33 for a 16 arg
    unsigned char num_slots;                          // number of slots in the array (we also have a scratch slot for the query)
    unsigned short hash_n_slots;                      // at 50% this can hold 32k functions (should be enough!!!???)
    //    unsigned short count_buf_size
    //    unsigned short count_buf_next
    //    TypeNum *count_buf
    //    func *call_back
    TypeNum type_nums[];
    //    TypeNum query[1][slot_width];      // a buffer of the right size to copy the TypeNums from the call site
    // OPEN: is this needed in bones or just Python
    //    TypeNum sig_array[num_slots][slot_width];
    //    TypeNum sig_hash[hash_n_slots][slot_width];// a hash table of signatures - we'll borrow techniques from else where to organise
} SelectorCache;


