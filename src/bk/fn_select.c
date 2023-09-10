#ifndef __BK_FN_SELECT_C
#define __BK_FN_SELECT_C "bk/fn_select.c"


#include "../../include/all.cfg"
#include "../../include/bk/bk.h"
#include "../../include/bk/kernel.h"
#include "utils.c"




//pointers
//| ---- ---- PPPP PPPP | PPPP PPPP PPPP P--- |
//
//11 spare bits
//
//IEEE Doubles - NaN boxing - 52 spare bits
//
//S[Exponent-][Mantissa------------------------------------------]
//| SEEE EEEE EEEE MMMM | MMMM MMMM MMMM MMMM | MMMM MMMM MMMM MMMM | MMMM MMMM MMMM MMMM |
//| S111 1111 1111 0000 | 0000 0000 0000 0000 | 0000 0000 0000 0000 | 0000 0000 0000 0000 |   +- infinity
//| -111 1111 1111 1--- | ---- ---- ---- ---- | ---- ---- ---- ---- | ---- ---- ---- ---- |   non-signalling NaN
//| -111 1111 1111 0--- | ---- ---- ---- ---- | ---- ---- ---- ---- | ---- ---- ---- ---- |   signalling NaN but mantissa > 0
//
//if double + pStr - 7 spare bits
//
//any struct > 2 things is going to be a pointer - so no real overhead in a 64bit box?
//symbols
//unboxed types
//symbols (length prefixed utf8), strings (utf8), numbers u8, i8, u64, i64, f64, bool, small structs
//
//stack
//scratch
//heap objects - prefix with a 32 bit meta field (which includes the type) - this  might get padded to 64bits for
//    doubles? or we can align the payload?, e.g. to cache lines or possibly set lines?


typedef unsigned short PAYLOAD;

// masks for embedding the code
#define SIZE_MASK 0x001F            // 0000 0000 0001 1111

#define LOWER_PAYLOAD_MASK 0x001F   // 0000 0000 0001 1111
#define UPPER_PAYLOAD_MASK 0xFFE0   // 1111 1111 1110 0000
#define LOWER_PAYLOAD_SHIFT 3

#define HAS_UPPER_MASK 0x8000       // 1000 0000 0000 0000
#define UPPER_TYPE_SHIFT 15
#define LOWER_TYPE_MASK 0x00007FFF  // 0000 0000 0000 0000 0111 1111 1111 1111
#define UPPER_TYPE_MASK 0xFFFF8000  // 1111 1111 1111 1111 1000 0000 0000 0000
#define MAX_UPPER_TYPE 0            // when have done upper make it 7


// pSig vs replicating sig
// pSig is 8bytes - sig1 is 4 to 6 bytes, sig2 is 6 to 10 bytes, sig3 is 8 to 14 bytes
// fnId can be encoded in pSig and sig - fnId = 2 ^ 11 (2048 fns per overload), pSig could encode more
// *pSig may be a cache miss (+200 cycles), but sig[] is less likely to be a miss
// functions with many args are not likely to have lots of overloads so the space saved by pSig in real terms may be
// minimal compared to the locality gained

// encode function number in sigheader - in bits 15 to 5 - allowing a max of 2048 overloads, with 16 arguments

struct SelectorCache {
    unsigned char slot_width;                         // in count of u16
    unsigned char num_slots;                          // number of slots in the array (we also have a scratch slot for the query)
    unsigned short hash_n_slots;                      // at 50% this can hold 32k functions (should be enough!!!???)
//    unsigned short count_buf_size
//    unsigned short count_buf_next
//    unsigned short *count_buf
//    func *call_back
    unsigned short type_nums[];
//    unsigned short query[1][slot_width];      // a buffer of the right size to copy the bt_ids from the call site
                                            // OPEN: is this needed in bones or just Python
//    unsigned short sig_array[num_slots][slot_width];
//    unsigned short sig_hash[hash_n_slots][slot_width];// a hash table of signatures - we'll borrow techniques from else where to organise
};

// OPEN: maybe add query count so can sort by it - for mo just track in Python

#define P_QUERY(sc) (&(sc)->type_nums[0])
#define P_SIG_ARRAY(sc) (&(sc)->type_nums[1 * (sc)->slot_width])
#define P_SIG_HASH(sc) (&(sc)->type_nums[(1 + (sc)->num_slots) * (sc)->slot_width])
#define SLOT_WIDTH_FROM_NUM_ARGS(num_args) (1 + 2 * (num_args))
#define NUM_ARGS_FROM_SLOT_WIDTH(slot_width) ((slot_width - 1) / 2)

// sig array has a variable length encoding
// SigHeader header - last 5 bits are length in u16 so 11111 = 31 which is taken as 32 (as it makes no sense to 
// dispatch on 0 args)

// sig arrays must be stored sparsely in the hash part of the cache but could be stored consecutively in the array
// part of the cache - for the moment only do sparse - only advantage of compact is avoiding cache misses

pvt void SC_at_array_put(struct SelectorCache *sc, int index, unsigned short sig[], PAYLOAD payload) {
    // index is one based, sig is size prefixed array of T1|T2
    unsigned short *dest = P_SIG_ARRAY(sc) + (index - 1) * sc->slot_width;
    unsigned short size = sig[0] & SIZE_MASK;
    dest[0] = (payload & UPPER_PAYLOAD_MASK) | size;
    for (fu8 o=1; o < size + 2; o++) dest[o] = sig[o];
    unsigned short *pad_array = dest + size + 2;
    size_t num_to_pad = sc->slot_width - (size + 1);
    for (fu8 o=0; o < num_to_pad; o++) pad_array[o] = 0;
    fu8 o_last = sc->slot_width - 1;
    dest[o_last] = dest[o_last] | ((payload & LOWER_PAYLOAD_MASK) << LOWER_PAYLOAD_SHIFT);
}

pvt unsigned char SC_next_free_array_index(struct SelectorCache *sc) {
    fu16 num_slots = sc->num_slots;
    fu16 slot_width = sc->slot_width;
    unsigned short *array = P_SIG_ARRAY(sc);
    for (fu16 o=0; o < num_slots; o++) if ((array + o * slot_width)[0] == 0x0000) return o + 1;
    return 0;
}

//static void SC_at_hash_put() {}

// printf("query[o]: %#02x, sig[o]: %#02x\n", query[o], sig[o]);

pvt inline fu16 fast_compare_sig(unsigned short query[], unsigned short sig[], fu8 slot_width) {
    fu16 N = query[0];
    if (N != (sig[0] & LOWER_PAYLOAD_MASK)) return 0;                                       // check count
    for (fu8 o = 1; o <= N; o++) {
        if (query[o] != sig[o]) return 0;                                        // check compressed bt_ids
//        if (query[o] == 0) return (sig[0] & UPPER_PAYLOAD_MASK) | ((sig[o_last] >> LOWER_PAYLOAD_SHIFT) & LOWER_PAYLOAD_MASK);   // check null terminal
    }
//    if (query[o_last] != (sig[o_last] & UPPER_PAYLOAD_MASK)) return 0;                             // check last
    return (sig[0] & UPPER_PAYLOAD_MASK) | ((sig[slot_width - 1] >> LOWER_PAYLOAD_SHIFT) & LOWER_PAYLOAD_MASK);
}

// the client will likely probe array first, compute a hash if missing, then probe from hash start
pvt fu16 fast_probe_sigs(unsigned short query[], unsigned short sigs[], fu8 slot_width, fu16 num_slots) {
    for (fu32 o = 0; o < num_slots; o++) {
        if (*(sigs + o * slot_width) == 0) return 0;
        fu16 v = fast_compare_sig(query, sigs + o * slot_width, slot_width);
        if (v) return v;
    }
    return 0;
}

pvt size_t SC_new_size(unsigned char num_args, unsigned char num_slots) {
    // OPEN check range and return err (like in SC_init)
    unsigned char slot_width = SLOT_WIDTH_FROM_NUM_ARGS(num_args);
    return sizeof(struct SelectorCache) + sizeof(unsigned short) * ((size_t)num_slots + 1) * (size_t)slot_width;
}

pvt err SC_init(struct SelectorCache *sc, unsigned char num_args, unsigned char num_slots) {
    unsigned char slot_width = SLOT_WIDTH_FROM_NUM_ARGS(num_args);
    if (!(1 <= num_args && num_args <=16)) SIGNAL("num_args is not within {1, 16}");         // OPEN add num_args value to msg
    if (!(1 <= num_slots && num_slots <=128)) SIGNAL("num_slots is not within {1, 128}");

    sc -> slot_width = slot_width;
    sc -> num_slots = num_slots;
    sc -> hash_n_slots = 0x0000;
    unsigned short *query = P_QUERY(sc);
    for (int i=0; i < (int)slot_width; i++) query[i] = 0x0000;
    unsigned short *array = P_SIG_ARRAY(sc);
    for (int i=0; i < (int)slot_width * (int)num_slots; i++) array[i] = 0x0000;
    return ok;
}

pvt void SC_drop(struct SelectorCache *sc) {
}


#endif  // __BK_FB_SELECT_C