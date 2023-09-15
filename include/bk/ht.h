#ifndef __BK_HT_H
#define __BK_HT_H

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "bk.h"



#define __HT_STRUCT(name, object_t, extravars)                                                                          \
    struct ht_##name {                                                                                                  \
        object_t *slots;                                                                                                \
        u32 *flags;                                                                                                     \
        u32 n_objects;                                                                                                  \
        u32 n_slots;                                                                                                    \
        u32 n_slots_used;                                                                                               \
        u32 n_slots_used_threshold;                                                                                     \
        extravars                                                                                                       \
    };                                                                                                                  \

#define HT_STRUCT(name, object_t) __HT_STRUCT(name, object_t, )
#define HT_STRUCT2(name, object_t, extravars) __HT_STRUCT(name, object_t, extravars)

#define HT_PROTOTYPES(name, object_t, key_t)                                                                            \
    extern struct ht_##name *ht_create_##name(void);                                                                    \
    extern void ht_trash_##name(struct ht_##name *);                                                                    \
    extern void ht_clear_##name(struct ht_##name *);                                                                    \
    extern u32 ht_get_it_##name(const struct ht_##name *, key_t);                                                       \
    extern int ht_resize_##name(struct ht_##name *, u32 num_slots);                                                     \
    extern u32 ht_put_idx_##name(struct ht_##name *, key_t, int *ret);                                                  \
    extern void ht_replace_empty_##name(struct ht_##name *, u32 it, object_t);                                          \
    extern void ht_replace_tombstone_##name(struct ht_##name *, u32 it, object_t);                                      \
    extern void ht_replace_existing_##name(struct ht_##name *, u32 it, object_t);                                       \
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
                EXISTS if the key is present in the hash table;
                EMPTY if the bucket is empty (never used);
                TOMBSTONE if the element in the bucket has been deleted [int*]
  @return       Iterator of the put location [u32]
 */
#define ht_put_idx(name, h, k, r) ht_put_idx_##name(h, k, r)

/*! @function
  @abstract     Retrieve a key from the hash table.
  @param  name  Name of the hash table [symbol]
  @param  h     Pointer to the hash table [ht_struct(name)*]
  @param  k     Key [key_t]
  @return       Iterator to the found element, or ht_it_end(h) if the element is absent [u32]
 */
#define ht_get_it(name, h, k) ht_get_it_##name(h, k)

/*! @function
  @abstract     Place object at empty slot idx.
  @param  name  Name of the hash table [symbol]
  @param  h     Pointer to the hash table [ht_struct(name)*]
  @param  it    Idx of slot [u32]
  @param  o     Object [object_t]
 */
#define ht_replace_empty(name, h, it, o) ht_replace_empty_##name(h, it, o)

/*! @function
  @abstract     Place object at tombstoned slot idx.
  @param  name  Name of the hash table [symbol]
  @param  h     Pointer to the hash table [ht_struct(name)*]
  @param  it    Idx of slot [u32]
  @param  o     Object [object_t]
 */
#define ht_replace_tombstone(name, h, it, o) ht_replace_tombstone_##name(h, it, o)

/*! @function
  @abstract     Replace an object at slot idx.
  @param  name  Name of the hash table [symbol]
  @param  h     Pointer to the hash table [ht_struct(name)*]
  @param  it    Idx of slot [u32]
  @param  o     Object [object_t]
 */
#define ht_replace_existing(name, h, it, o) ht_replace_existing_##name(h, it, o)

/*! @function
  @abstract     Remove a key from the hash table.
  @param  name  Name of the hash table [symbol]
  @param  h     Pointer to the hash table [ht_struct(name)*]
  @param  it    Iterator to the element to be deleted [u32]
 */
#define ht_del(name, h, it) ht_del_##name(h, it)

/*! @function
  @abstract     Test whether a bucket contains data.
  @param  h     Pointer to the hash table [ht_struct(name)*]
  @param  it    Iterator to the bucket [u32]
  @return       1 if containing data; 0 otherwise [int]
 */
#define ht_exist(h, it) (!__ac_iseither((h)->flags, (it)))

/*! @function
  @abstract     Get key given an iterator
  @param  h     Pointer to the hash table [ht_struct(name)*]
  @param  it    Iterator to the bucket [u32]
  @return       Object [object_t]
 */
#define ht_object(h, it) ((h)->slots[it])

/*! @function
  @abstract     Get the start iterator
  @param  h     Pointer to the hash table [ht_struct(name)*]
  @return       The start iterator [u32]
 */
#define ht_it_start(h) (u32)(0)

/*! @function
  @abstract     Get the end iterator
  @param  h     Pointer to the hash table [ht_struct(name)*]
  @return       The end iterator [u32]
 */
#define ht_it_end(h) ((h)->n_slots)

/*! @function
  @abstract     Get the number of elements in the hash table
  @param  h     Pointer to the hash table [ht_struct(name)*]
  @return       Number of elements in the hash table [u32]
 */
#define ht_size(h) ((h)->n_objects)

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
#define ht_foreach(h, ovar, code) { u32 __i;                                                                            \
    for (__i = ht_it_start(h); __i != ht_it_end(h); ++__i) {                                                            \
        if (!ht_exist(h,__i)) continue;                                                                                 \
        (ovar) = ht_object(h,__i);                                                                                      \
        code;                                                                                                           \
    } }


#endif // __BK_HT_H
