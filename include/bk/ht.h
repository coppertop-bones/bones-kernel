#ifndef __BK_HT_H
#define __BK_HT_H

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "bk.h"

// hash table is an array of slots (with size of power of 2), each slot may contain an entry, a tombstone or be empty

#define __HT_STRUCT(name, slot_t, extravars)                                                                            \
    struct ht_##name {                                                                                                  \
        slot_t *slots;                                                                                                  \
        u32 *flags;                                                                                                     \
        u32 n_entries;                                                                                                  \
        u32 n_slots;                                                                                                    \
        u32 n_slots_used;                                                                                               \
        u32 n_slots_used_threshold;                                                                                     \
        extravars                                                                                                       \
    };                                                                                                                  \

#define HT_STRUCT(name, slot_t) __HT_STRUCT(name, slot_t, )
#define HT_STRUCT2(name, slot_t, extravars) __HT_STRUCT(name, slot_t, extravars)


#define HT_EXISTS 0
#define HT_EMPTY 1
#define HT_TOMBSTONE 2


#define HT_PROTOTYPES(name, slot_t, key_t)                                                                              \
    extern struct ht_##name *ht_create_##name(void);                                                                    \
    extern void ht_trash_##name(struct ht_##name *);                                                                    \
    extern void ht_clear_##name(struct ht_##name *);                                                                    \
    extern u32 ht_get_idx_##name(struct ht_##name *, key_t);                                                      \
    extern int ht_resize_##name(struct ht_##name *, u32 num_slots);                                                     \
    extern u32 ht_put_idx_##name(struct ht_##name *, key_t, int *ret);                                                  \
    extern void ht_replace_empty_##name(struct ht_##name *, u32 idx, slot_t);                                           \
    extern void ht_replace_tombstone_##name(struct ht_##name *, u32 idx, slot_t);                                       \
    extern void ht_replace_existing_##name(struct ht_##name *, u32 idx, slot_t);                                        \
    extern void ht_del_##name(struct ht_##name *, u32 idx);                                                             \


// ---------------------------------------------------------------------------------------------------------------------
// API
// ---------------------------------------------------------------------------------------------------------------------

/*!
  @abstract Type of the hash table.
  @param  name  Name of the hash table [symbol]
 */
#define ht_struct(name) struct ht_##name

/*! @function
  @abstract     Answer a pointer to a new hash table.
  @param  name  Name of the hash table [symbol]
  @return       Pointer to the hash table [ht_struct(name)*]
 */
#define ht_create(name) ht_create_##name()

/*! @function
  @abstract     Trashes a hash table.
  @param  name  Name of the hash table [symbol]
  @param  h     Pointer to the hash table [ht_struct(name)*]
 */
#define ht_trash(name, h) ht_trash_##name(h)

/*! @function
  @abstract     Reset a hash table without deallocating memory.
  @param  name  Name of the hash table [symbol]
  @param  h     Pointer to the hash table [ht_struct(name)*]
 */
#define ht_clear(name, h) ht_clear_##name(h)

/*! @function
  @abstract     Resize a hash table.
  @param  name  Name of the hash table [symbol]
  @param  h     Pointer to the hash table [ht_struct(name)*]
  @param  s     New size [u32]
 */
#define ht_resize(name, h, s) ht_resize_##name(h, s)

/*! @function
  @abstract     Insert a key to the hash table.
  @param  name  Name of the hash table [symbol]
  @param  h     Pointer to the hash table [ht_struct(name)*]
  @param  k     Key [key_t]
  @param  r     Extra return code:
                RESIZE_FAILED if the operation failed;
                HT_EXISTS if the key is present in the hash table;
                HT_EMPTY if the bucket is empty (never used);
                HT_TOMBSTONE if the element in the bucket has been deleted [int*]
  @return       Iterator of the put location [u32]
 */
#define ht_put_idx(name, h, k, r) ht_put_idx_##name(h, k, r)

/*! @function
  @abstract     Retrieve a key from the hash table.
  @param  name  Name of the hash table [symbol]
  @param  h     Pointer to the hash table [ht_struct(name)*]
  @param  k     Key [key_t]
  @return       Iterator to the found element, or ht_eot(h) if the element is absent [u32]
 */
#define ht_get_idx(name, h, k) ht_get_idx_##name(h, k)

/*! @function
  @abstract     Put an entry at empty slot idx.
  @param  name  Name of the hash table [symbol]
  @param  h     Pointer to the hash table [ht_struct(name)*]
  @param  idx   Idx of slot [u32]
  @param  entry     Entry [slot_t]
 */
#define ht_replace_empty(name, h, idx, entry) ht_replace_empty_##name(h, idx, entry)

/*! @function
  @abstract     Place entry at tombstoned slot idx.
  @param  name  Name of the hash table [symbol]
  @param  h     Pointer to the hash table [ht_struct(name)*]
  @param  idx   Idx of slot [u32]
  @param  entry     Entry [slot_t]
 */
#define ht_replace_tombstone(name, h, idx, entry) ht_replace_tombstone_##name(h, idx, entry)

/*! @function
  @abstract     Replace an entry at slot idx.
  @param  name  Name of the hash table [symbol]
  @param  h     Pointer to the hash table [ht_struct(name)*]
  @param  idx   Idx of slot [u32]
  @param  entry     Entry [slot_t]
 */
#define ht_replace_existing(name, h, idx, entry) ht_replace_existing_##name(h, idx, entry)

/*! @function
  @abstract     Remove a key from the hash table.
  @param  name  Name of the hash table [symbol]
  @param  h     Pointer to the hash table [ht_struct(name)*]
  @param  idx   Idx to the element to be deleted [u32]
 */
#define ht_del(name, h, idx) ht_del_##name(h, idx)

/*! @function
  @abstract     Test whether a bucket contains data.
  @param  h     Pointer to the hash table [ht_struct(name)*]
  @param  idx   Idx to the bucket [u32]
  @return       1 if containing data; 0 otherwise [int]
 */
#define ht_exist(h, idx) (__ht_is_occupied((h)->flags, (idx)))

/*! @function
  @abstract     Get entry given an idx
  @param  h     Pointer to the hash table [ht_struct(name)*]
  @param  idx   Idx to the bucket [u32]
  @return       Entry [slot_t]
 */
#define ht_entry(h, idx) ((h)->slots[idx])

/*! @function
  @abstract     Get the start idx
  @param  h     Pointer to the hash table [ht_struct(name)*]
  @return       The start idx [u32]
 */
#define ht_start(h) (u32)(0)

/*! @function
  @abstract     Get the end of table idx
  @param  h     Pointer to the hash table [ht_struct(name)*]
  @return       The end idx [u32]
 */
#define ht_eot(h) ((h)->n_slots)

/*! @function
  @abstract     Get the number of elements in the hash table
  @param  h     Pointer to the hash table [ht_struct(name)*]
  @return       Number of elements in the hash table [u32]
 */
#define ht_n_entries(h) ((h)->n_entries)

/*! @function
  @abstract     Get the number of slots in the hash table
  @param  h     Pointer to the hash table [ht_struct(name)*]
  @return       Number of slots in the hash table [u32]
 */
#define ht_n_slots(h) ((h)->n_slots)

/*! @function
  @abstract     Iterate over the entries in the hash table
  @param  h     Pointer to the hash table [ht_struct(name)*]
  @param  kvar  Variable to which key will be assigned
  @param  code  Block of code to execute
 */
#define ht_foreach(h, entryvar, code) { u32 __i;                                                                        \
        for (__i = ht_start(h); __i != ht_eot(h); ++__i) {                                                              \
            if (!ht_exist(h, __i)) continue;                                                                            \
            (entryvar) = ht_entry(h, __i);                                                                              \
            code;                                                                                                       \
        }                                                                                                               \
    }


#endif // __BK_HT_H
