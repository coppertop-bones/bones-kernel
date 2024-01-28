// ---------------------------------------------------------------------------------------------------------------------
// HT - HASH TABLE
// based on https://github.com/attractivechaos/klib/blob/master/khash.h
// https://attractivechaos.wordpress.com/2019/12/28/deletion-from-hash-tables-without-tombstones/
// ---------------------------------------------------------------------------------------------------------------------

#ifndef API_BK_HT_H
#define API_BK_HT_H

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "bk.h"


// hash table is a sparce array (with size of power of 2), each entry may contain a value, a tombstone or be empty

#define __HT_STRUCT(name, entry_t, extravars)                                                                           \
    struct ht_##name {                                                                                                  \
        entry_t *entries;                                                                                               \
        u32 *flags;                                                                                                     \
        u32 sz;                                                                                                         \
        u32 n_live;             /* number of entries with live values */                                                \
        u32 n_used;             /* number of used entries - includes number of entries and number of tombstones */      \
        u32 n_used_threshold;                                                                                           \
        extravars               /* allows use to turn a hash table into a hash map */                                   \
    };                                                                                                                  \

#define HT_STRUCT(name, entry_t) __HT_STRUCT(name, entry_t, )                             // hash table constructor
#define HT_STRUCT_EXTRA(name, entry_t, extravars) __HT_STRUCT(name, entry_t, extravars)   // has map constructor


#define HT_LIVE 0
#define HT_EMPTY 1
#define HT_TOMBSTONE 2


#define HT_PROTOTYPES(name, entry_t, hashable_t)                                                                        \
    extern struct ht_##name *ht_create_##name(void);                                                                    \
    extern void ht_trash_##name(struct ht_##name *);                                                                    \
    extern void ht_clear_##name(struct ht_##name *);                                                                    \
    extern u32 ht_get_idx_##name(struct ht_##name *, hashable_t);                                                       \
    extern u32 ht_put_idx_##name(struct ht_##name *, hashable_t, int *ret);                                             \
    extern void ht_replace_empty_##name(struct ht_##name *, u32 idx, entry_t);                                          \
    extern void ht_replace_tombstone_##name(struct ht_##name *, u32 idx, entry_t);                                      \
    extern void ht_replace_value_##name(struct ht_##name *, u32 idx, entry_t);                                          \
    extern int ht_resize_##name(struct ht_##name *, u32 sz);                                                            \
    extern void ht_del_##name(struct ht_##name *, u32 idx);                                                             \


// ---------------------------------------------------------------------------------------------------------------------
// API
// ---------------------------------------------------------------------------------------------------------------------

/*!
  @abstract Type of the hash table.
  @param  name  Name of the hash table
 */
#define ht_struct(name) struct ht_##name

/*! @function
  @abstract     Answer a pointer to a new hash table.
  @param  name  Name of the hash table
  @return       Pointer to the hash table [ht_struct(name)*]
 */
#define ht_create(name) ht_create_##name()

/*! @function
  @abstract     Trashes a hash table.
  @param  name  Name of the hash table
  @param  ht    Pointer to the hash table [ht_struct(name)*]
 */
#define ht_trash(name, ht) ht_trash_##name(ht)

/*! @function
  @abstract     Reset a hash table without deallocating memory.
  @param  name  Name of the hash table
  @param  ht    Pointer to the hash table [ht_struct(name)*]
 */
#define ht_clear(name, ht) ht_clear_##name(ht)

/*! @function
  @abstract     Resize a hash table.
  @param  name  Name of the hash table
  @param  ht    Pointer to the hash table [ht_struct(name)*]
  @param  sz    New size [u32]
 */
#define ht_resize(name, ht, sz) ht_resize_##name(ht, sz)

/*! @function
  @abstract     Answer idx to put value
  @param  name  Name of the hash table
  @param  hh    Pointer to the hash table [ht_struct(name)*]
  @param  h     Hashable [hashable_t]
  @param  o     Outcome:
                RESIZE_FAILED;
                HT_LIVE if the entry is live;
                HT_EMPTY if the entry is empty (never used);
                HT_TOMBSTONE if the entry has been deleted [int*]
  @return       Idx of the put location [u32]
 */
#define ht_put_idx(name, ht, h, o) ht_put_idx_##name(ht, h, o)

/*! @function
  @abstract     Answer idx a live entry
  @param  name  Name of the hash table
  @param  ht    Pointer to the hash table [ht_struct(name)*]
  @param  h     Hashable [hashable_t]
  @return       Idx of the value, or ht_eot(ht) if the element is absent [u32]
 */
#define ht_get_idx(name, ht, h) ht_get_idx_##name(ht, h)

/*! @function
  @abstract     Put an value at empty slot idx.
  @param  name  Name of the hash table [symbol]
  @param  ht    Pointer to the hash table [ht_struct(name)*]
  @param  idx   Idx of slot [u32]
  @param  entry Entry [entry_t]
 */
#define ht_replace_empty(name, ht, idx, entry) ht_replace_empty_##name(ht, idx, entry)

/*! @function
  @abstract     Place value at tombstoned slot idx.
  @param  name  Name of the hash table [symbol]
  @param  ht    Pointer to the hash table [ht_struct(name)*]
  @param  idx   Idx of slot [u32]
  @param  entry Entry [entry_t]
 */
#define ht_replace_tombstone(name, ht, idx, entry) ht_replace_tombstone_##name(ht, idx, entry)

/*! @function
  @abstract     Replace an value at slot idx.
  @param  name  Name of the hash table [symbol]
  @param  ht    Pointer to the hash table [ht_struct(name)*]
  @param  idx   Idx of slot [u32]
  @param  entry Entry [entry_t]
 */
#define ht_replace_value(name, ht, idx, entry) ht_replace_value_##name(ht, idx, entry)

/*! @function
  @abstract     Delete an entry in the hash table.
  @param  name  Name of the hash table [symbol]
  @param  ht    Pointer to the hash table [ht_struct(name)*]
  @param  idx   Idx to the element to be deleted [u32]
 */
#define ht_del(name, ht, idx) ht_del_##name(ht, idx)

/*! @function
  @abstract     Test whether a bucket contains data.
  @param  ht    Pointer to the hash table [ht_struct(name)*]
  @param  idx   Idx to the bucket [u32]
  @return       1 if containing data; 0 otherwise [int]
 */
#define ht_exist(ht, idx) (__ht_is_occupied((ht)->flags, (idx)))

/*! @function
  @abstract     Get value given an idx
  @param  ht    Pointer to the hash table [ht_struct(name)*]
  @param  idx   Idx to the bucket [u32]
  @return       Entry [entry_t]
 */
#define ht_value(ht, idx) ((ht)->entries[idx])

/*! @function
  @abstract     Get the start idx
  @param  ht    Pointer to the hash table [ht_struct(name)*]
  @return       The start idx [u32]
 */
#define ht_start(ht) (u32)(0)

/*! @function
  @abstract     Get the end of table idx
  @param  ht    Pointer to the hash table [ht_struct(name)*]
  @return       The end idx [u32]
 */
#define ht_eot(ht) ((ht)->sz)

/*! @function
  @abstract     Get the number of elements in the hash table
  @param  ht    Pointer to the hash table [ht_struct(name)*]
  @return       Number of elements in the hash table [u32]
 */
#define ht_n_values(ht) ((ht)->n_live)

/*! @function
  @abstract     Get the total size of the hash table
  @param  ht    Pointer to the hash table [ht_struct(name)*]
  @return       Number of entries in the hash table [u32]
 */
#define ht_sz(ht) ((ht)->sz)

/*! @function
  @abstract     Iterate over the values in the hash table
  @param  ht    Pointer to the hash table [ht_struct(name)*]
  @param  var   Variable to which entry will be assigned
  @param  code  Block of code to execute
 */
#define ht_foreach(ht, var, code) { u32 __i;                                                                            \
        for (__i = ht_start(ht); __i != ht_eot(ht); ++__i) {                                                            \
            if (!ht_exist(ht, __i)) continue;                                                                           \
            (var) = ht_value(ht, __i);                                                                                  \
            code;                                                                                                       \
        }                                                                                                               \
    }


#endif // API_BK_HT_H
