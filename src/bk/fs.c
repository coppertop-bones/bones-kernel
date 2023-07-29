// ---------------------------------------------------------------------------------------------------------------------
// Copyright 2025 David Briant, https://github.com/coppertop-bones. Licensed under the Apache License, Version 2.0
//
// FS - FUNCTION SELECTION
// ---------------------------------------------------------------------------------------------------------------------

#ifndef SRC_BK_FS_C
#define SRC_BK_FS_C "bk/fs.c"


#include "../../include/bk/bk.h"
#include "bk.c"
#include "lib/txt.c"


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

typedef struct {
    unsigned char slot_width;       // in count of u16
    unsigned char num_slots;        // number of slots in the array (plus scratch slot for the query)
    unsigned short buf[];
} FS;

#define P_QUERY(fs) (&(fs)->buf[0])
#define P_SIG_ARRAY(fs) (&(fs)->buf[1 * (fs)->slot_width])
#define P_SIG_HASH(fs) (&(fs)->buf[(1 + (fs)->num_slots) * (fs)->slot_width])
#define SLOT_WIDTH_FROM_NUM_ARGS(num_args) (1 + 2 * (num_args))
#define NUM_ARGS_FROM_SLOT_WIDTH(slot_width) ((slot_width - 1) / 2)

// |       HC / LP       |         ...         |       UP / SS       |
// | HHHH HHHH PPPP P--- |         ...         | PPPP PPPP PPPN NNNN |
//
// |         UBT         |         LBT         |
// | 0000 0000 0000 0TTT | UTTT TTTT TTTT TTTT |

pvt void FS_at_array_put(FS *fs, int index, unsigned short sig[], PAYLOAD payload) {
    // index is one based, sig is size prefixed array of T1|T2
    unsigned short *dest = P_SIG_ARRAY(fs) + (index - 1) * fs->slot_width;
    int size = sig[0] & SIZE_MASK;
    dest[0] = (payload & UPPER_PAYLOAD_MASK) | size;
    for (int o=1; o < size + 2; o++) dest[o] = sig[o];
    unsigned short *pad_array = dest + size + 2;
    int num_to_pad = fs->slot_width - (size + 1);
    for (int o=0; o < num_to_pad; o++) pad_array[o] = 0;
    int o_last = fs->slot_width - 1;
    dest[o_last] = 0x0000 | ((payload & LOWER_PAYLOAD_MASK) << LOWER_PAYLOAD_SHIFT);
}

pvt int FS_next_free_array_index(FS *fs) {
    int num_slots = fs->num_slots;
    int slot_width = fs->slot_width;
    unsigned short *array = P_SIG_ARRAY(fs);
    for (int o=0; o < num_slots; o++) if ((array + o * slot_width)[0] == 0x0000) return o + 1;
    return 0;
}

pvt inline int fast_compare_sig(unsigned short query[], unsigned short sig[], int slot_width, unsigned short *HC) {
    int N = query[0];
    if (N != (sig[0] & LOWER_PAYLOAD_MASK)) return 0;                                   // check count
    for (int o = 1; o <= N; o++) {
        if (query[o] != sig[o]) return 0;                                               // check compressed bt_ids
//        if (query[o] == 0) return (sig[0] & UPPER_PAYLOAD_MASK) | ((sig[o_last] >> LOWER_PAYLOAD_SHIFT) & LOWER_PAYLOAD_MASK);   // check null terminal
    }
//    if (query[o_last] != (sig[o_last] & UPPER_PAYLOAD_MASK)) return 0;                // check last
    int o_last = slot_width - 1;
    int hc = ((sig[o_last] & HC_MASK) + HC_INC) & HC_MASK;
    if (hc == 0) *HC += 1;  // if wrapped inc off struct hit count
    sig[o_last] = hc | (sig[o_last] & NOT_HC_MASK);
    return (sig[0] & UPPER_PAYLOAD_MASK) | ((sig[o_last] >> LOWER_PAYLOAD_SHIFT) & LOWER_PAYLOAD_MASK);
}

// the client will likely probe array first, compute a hash if missing, then probe from hash start
pvt int fast_probe_sigs(unsigned short query[], unsigned short sigs[], int slot_width, int num_slots) {
    unsigned short *HCs = sigs + slot_width * num_slots;
    for (int o = 0; o < num_slots; o++) {
        if (*(sigs + o * slot_width) == 0) return 0;
        int v = fast_compare_sig(query, sigs + o * slot_width, slot_width, HCs + o);
        if (v) return v;
    }
    return 0;
}

//pvt size_t FS_required_size(int num_args, int num_slots) {
//    // OPEN check range and return err (like in FS_create)
//    int slot_width = SLOT_WIDTH_FROM_NUM_ARGS(num_args);
//    return sizeof(FS) + sizeof(unsigned short) * (num_slots + 1) * slot_width + sizeof(unsigned short) * num_slots;
//}

pvt err FS_required_size(int num_args, int num_slots, size_t *size) {
    if (!(1 <= num_args && num_args <=16)) SIGNAL("num_args is not within {1, 16}");         // OPEN add num_args value to msg
    if (!(1 <= num_slots && num_slots <=128)) SIGNAL("num_slots is not within {1, 128}");
    int slot_width = SLOT_WIDTH_FROM_NUM_ARGS(num_args);
    *size = sizeof(FS) + sizeof(unsigned short) * (num_slots + 1) * slot_width + sizeof(unsigned short) * num_slots;
    return BK_NO_ERROR;
}

pvt err FS_create(FS *fs, int num_args, int num_slots) {
    int i, iHC;
    int slot_width = SLOT_WIDTH_FROM_NUM_ARGS(num_args);
    if (!(1 <= num_args && num_args <=16)) SIGNAL("num_args is not within {1, 16}");         // OPEN add num_args value to msg
    if (!(1 <= num_slots && num_slots <=128)) SIGNAL("num_slots is not within {1, 128}");

    fs -> slot_width = slot_width;
    fs -> num_slots = num_slots;
    unsigned short *query = P_QUERY(fs);
    for (i=0; i < slot_width; i++) query[i] = 0x0000;
    unsigned short *array = P_SIG_ARRAY(fs);
    for (i=0; i < slot_width * num_slots; i++) array[i] = 0x0000;
    for (iHC=0; iHC < num_slots; iHC++) array[i+iHC] = 0x0000;
    return BK_NO_ERROR;
}

pvt void FS_trash(FS *fs) {
}


#endif  // SRC_BK_FS_C