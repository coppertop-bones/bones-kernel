// ---------------------------------------------------------------------------------------------------------------------
// Copyright 2025 David Briant, https://github.com/coppertop-bones. Licensed under the Apache License, Version 2.0
//
// HI - HASH INDEX
// based on https://github.com/attractivechaos/klib/blob/master/khash.h
// https://attractivechaos.wordpress.com/2019/12/28/deletion-from-hash-tables-without-tombstones/
// ---------------------------------------------------------------------------------------------------------------------

#ifndef INC_BK_HI_H
#define INC_BK_HI_H

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "../bk.h"

// TODO
//  implement special values for unused (0) and tombstoned (-1) tokens, e.g. for type manager
//  add size limit for linear probing
//  investigate hashing functions and other probing styles


// A hash index is essentially a sparce array of tokens (with size of power of 2), each token may live, tombstoned
// or empty. The token is used to find the actual entry (and for the simplest uses, e.g. a set of integers, may be the
// entry itself)

#define __HI_STRUCT(name, token_t, extravars)                                                                           \
    struct hi_##name {                                                                                                  \
        token_t *tokens;                                                                                                \
        u32 *flags;                                                                                                     \
        u32 sz;                                                                                                         \
        u32 n_live;             /* number of live tokens */                                                             \
        u32 n_used;             /* number of live and tombstoned tokens */                                              \
        u32 n_used_threshold;                                                                                           \
        extravars               /* extra variables used by the hash functions e.g. to implement a hash map */           \
    };                                                                                                                  \

#define HI_STRUCT(name, token_t) __HI_STRUCT(name, token_t, )
#define HI_STRUCT_WITH(name, token_t, extravars) __HI_STRUCT(name, token_t, extravars)


#define HI_LIVE 0
#define HI_EMPTY 1
#define HI_TOMBSTONE 2


#define HI_PROTOTYPES(name, token_t, hashable_t)                                                                        \
    extern struct hi_##name *hi_create_##name(void);                                                                    \
    extern void hi_trash_##name(struct hi_##name *);                                                                    \
    extern void hi_clear_##name(struct hi_##name *);                                                                    \
    extern u32 hi_get_idx_##name(struct hi_##name *, hashable_t);                                                       \
    extern u32 hi_put_idx_##name(struct hi_##name *, hashable_t, int *ret);                                             \
    extern void hi_replace_empty_##name(struct hi_##name *, u32 idx, token_t);                                          \
    extern void hi_replace_tombstone_##name(struct hi_##name *, u32 idx, token_t);                                      \
    extern void hi_replace_token_##name(struct hi_##name *, u32 idx, token_t);                                          \
    extern int hi_resize_##name(struct hi_##name *, u32 sz);                                                            \
    extern void hi_drop_##name(struct hi_##name *, u32 idx);                                                            \


// ---------------------------------------------------------------------------------------------------------------------
// API
// ---------------------------------------------------------------------------------------------------------------------

/*!
  @abstract Type of the hash index.
  @param  name  Name of the hash index
 */
#define hi_struct(name) struct hi_##name

/*! @function
  @abstract     Answer a pointer to a new hash index.
  @param  name  Name of the hash index
  @return       Pointer to the hash index [hi_struct(name)*]
 */
#define hi_create(name) hi_create_##name()

/*! @function
  @abstract     Trashes a hash index.
  @param  name  Name of the hash index
  @param  hi    Pointer to the hash index [hi_struct(name)*]
 */
#define hi_trash(name, hi) hi_trash_##name(hi)

/*! @function
  @abstract     Reset a hash index without deallocating memory.
  @param  name  Name of the hash index
  @param  hi    Pointer to the hash index [hi_struct(name)*]
 */
#define hi_clear(name, hi) hi_clear_##name(hi)

/*! @function
  @abstract     Resize a hash index.
  @param  name  Name of the hash index
  @param  hi    Pointer to the hash index [hi_struct(name)*]
  @param  sz    New size [u32]
 */
#define hi_resize(name, hi, sz) hi_resize_##name(hi, sz)

/*! @function
  @abstract     Answer the idx, corresponding to the hashable, to put a token, resizing if necessary
  @param  name  Name of the hash index
  @param  hi    Pointer to the hash index [hi_struct(name)*]
  @param  h     Hashable [hashable_t]
  @param  o     Outcome (out param):
                RESIZE_FAILED;
                HI_LIVE if the location is currently live;
                HI_EMPTY if the location is currently empty (never used);
                HI_TOMBSTONE if the location is currently tombstoned [int*]
  @return       Idx of the put location [u32]
 */
#define hi_put_idx(name, hi, h, o) hi_put_idx_##name(hi, h, o)

/*! @function
  @abstract     Answer the live token idx corresponding to the hashable or end if it can't be found
  @param  name  Name of the hash index
  @param  hi    Pointer to the hash index [hi_struct(name)*]
  @param  h     Hashable [hashable_t]
  @return       Idx of the token or hi_end(hi) if absent [u32]
 */
#define hi_get_idx(name, hi, h) hi_get_idx_##name(hi, h)

/*! @function
  @abstract     Put a token at a currently empty location.
  @param  name  Name of the hash index [symbol]
  @param  hi    Pointer to the hash index [hi_struct(name)*]
  @param  idx   Idx of slot [u32]
  @param  token Token [token_t]
 */
#define hi_replace_empty(name, hi, idx, token) hi_replace_empty_##name(hi, idx, token)

/*! @function
  @abstract     Put a token at a currently tombstoned location.
  @param  name  Name of the hash index [symbol]
  @param  hi    Pointer to the hash index [hi_struct(name)*]
  @param  idx   Idx of slot [u32]
  @param  token Token [token_t]
 */
#define hi_replace_tombstone(name, hi, idx, token) hi_replace_tombstone_##name(hi, idx, token)

/*! @function
  @abstract     Put a token at a currently live location.
  @param  name  Name of the hash index [symbol]
  @param  hi    Pointer to the hash index [hi_struct(name)*]
  @param  idx   Idx of slot [u32]
  @param  token Token [token_t]
 */
#define hi_replace_live(name, hi, idx, token) hi_replace_token_##name(hi, idx, token)

/*! @function
  @abstract     Tombstone a token in the hash index.
  @param  name  Name of the hash index [symbol]
  @param  hi    Pointer to the hash index [hi_struct(name)*]
  @param  idx   Idx to the token to be deleted [u32]
 */
#define hi_drop(name, hi, idx) hi_drop_##name(hi, idx)

/*! @function
  @abstract     Answers if the token at idx is live.
  @param  hi    Pointer to the hash index [hi_struct(name)*]
  @param  idx   Idx [u32]
  @return       1 if containing data; 0 otherwise [int]
 */
#define hi_is_live(hi, idx) (__hi_is_live((hi)->flags, (idx)))

/*! @function
  @abstract     Answer the token at idx
  @param  hi    Pointer to the hash index [hi_struct(name)*]
  @param  idx   Idx [u32]
  @return       Token [token_t]
 */
#define hi_token(hi, idx) ((hi)->tokens[idx])

/*! @function
  @abstract     Answer the first idx
  @param  hi    Pointer to the hash index [hi_struct(name)*]
  @return       The start idx [u32]
 */
#define hi_start(hi) (u32)(0)

/*! @function
  @abstract     Answer the end idx, i.e. one after the last indexable
  @param  hi    Pointer to the hash index [hi_struct(name)*]
  @return       The end idx [u32]
 */
#define hi_end(hi) ((hi)->sz)

/*! @function
  @abstract     Get the number of live tokens in the hash index
  @param  hi    Pointer to the hash index [hi_struct(name)*]
  @return       Number of live tokens [u32]
 */
#define hi_n_live(hi) ((hi)->n_live)

/*! @function
  @abstract     Get the total size of the hash index
  @param  hi    Pointer to the hash index [hi_struct(name)*]
  @return       Total number of tokens [u32]
 */
#define hi_sz(hi) ((hi)->sz)

/*! @function
  @abstract     Iterate over the token in the hash index
  @param  hi    Pointer to the hash index [hi_struct(name)*]
  @param  var   Variable to which token will be assigned
  @param  code  Block of code to execute
 */
#define hi_foreach(hi, var, code) { u32 __i;                                                                            \
        for (__i = hi_start(hi); __i < hi_end(hi); ++__i) {                                                             \
            if (!hi_is_live(hi, __i)) continue;                                                                         \
            (var) = hi_token(hi, __i);                                                                                  \
            code;                                                                                                       \
        }                                                                                                               \
    }


#endif // INC_BK_HI_H
