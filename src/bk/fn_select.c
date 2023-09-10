#ifndef __BK_FN_SELECT_C
#define __BK_FN_SELECT_C "bk/fn_select.c"


#include "../../include/all.cfg"
#include "../../include/bk/bk.h"

#include "utils.c"


typedef unsigned short PAYLOAD;

// the following does not currently incorporate the hit count

// masks for embedding the code
#define SIZE_MASK 0x001F            // 0000 0000 0001 1111

#define LOWER_PAYLOAD_MASK 0x001F   // 0000 0000 0001 1111
#define UPPER_PAYLOAD_MASK 0xFFE0   // 1111 1111 1110 0000
#define LOWER_PAYLOAD_SHIFT 3

#define HAS_UPPER_TYPE_FLAG 0x8000  // 1000 0000 0000 0000
#define UPPER_TYPE_SHIFT 15
#define LOWER_TYPE_MASK 0x00007FFF  // 0000 0000 0000 0000 0111 1111 1111 1111
#define UPPER_TYPE_MASK 0xFFFF8000  // 1111 1111 1111 1111 1000 0000 0000 0000
#define MAX_UPPER_TYPE 0            // when have done upper make it 7

#define HC_MASK 0xFF00
#define NOT_HC_MASK 0x00FF
#define HC_INC  0x0100

struct SelectorCache {
    unsigned char slot_width;                         // in count of u16
    unsigned char num_slots;                          // number of slots in the array (plus scratch slot for the query)
    unsigned short buf[];
};

#define P_QUERY(sc) (&(sc)->buf[0])
#define P_SIG_ARRAY(sc) (&(sc)->buf[1 * (sc)->slot_width])
#define P_SIG_HASH(sc) (&(sc)->buf[(1 + (sc)->num_slots) * (sc)->slot_width])
#define SLOT_WIDTH_FROM_NUM_ARGS(num_args) (1 + 2 * (num_args))
#define NUM_ARGS_FROM_SLOT_WIDTH(slot_width) ((slot_width - 1) / 2)

// |       HC / LP       |         ...         |       UP / SS       |
// | HHHH HHHH PPPP P--- |         ...         | PPPP PPPP PPPN NNNN |
//
// |         UBT         |         LBT         |
// | 0000 0000 0000 0TTT | UTTT TTTT TTTT TTTT |

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
    unsigned short *query = P_QUERY(sc);
    for (int i=0; i < (int)slot_width; i++) query[i] = 0x0000;
    unsigned short *array = P_SIG_ARRAY(sc);
    for (int i=0; i < (int)slot_width * (int)num_slots; i++) array[i] = 0x0000;
    return ok;
}

pvt void SC_drop(struct SelectorCache *sc) {
}


#endif  // __BK_FN_SELECT_C