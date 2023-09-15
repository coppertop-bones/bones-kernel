// ---------------------------------------------------------------------------------------------------------------------
// EXAMPLE
// ---------------------------------------------------------------------------------------------------------------------

/*
#include "bk/hash.h"
KHASH_MAP_STRUCT(u32_u8, khint32_t, char)
KHASH_IMPL(u32_u8, khint32_t, char, KHASH_MAP, ht_int_hash_func, ht_int_hash_equal)
int main() {
    int res, is_missing;
    u32 o;
    ht_struct(u32_u8) *hm = ht_create(u32_u8);
    o = ht_put_idx(u32_u8, hm, 5, &res);
    if (o && res == EMPTY)
    o = ht_get_it(u32_u8, hm, 10);
    is_missing = (o == ht_it_end(hm));
    o = ht_get_it(u32_u8, hm, 5);
    ht_del(u32_u8, hm, o);
    for (o = ht_it_start(hm); o != ht_it_end(hm); ++o)
        if (ht_exist(hm, o)) ht_value(hm, o) = 1;
    ht_trash(u32_u8, hm);
    return 0;
}
*/


#ifndef __BK_HT_IMPL_H
#define __BK_HT_IMPL_H


#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "../../include/bk/ht.h"


#define __ac_isdel(flags, o) ((flags[o>>4]>>((o&0xfU)<<1))&1)
#define __ac_isempty(flags, o) ((flags[o>>4]>>((o&0xfU)<<1))&2)
#define __ac_iseither(flags, o) ((flags[o>>4]>>((o&0xfU)<<1))&3)
//#define __ac_set_isdel_false(flags, o) (flags[o>>4]&=~(1ul<<((o&0xfU)<<1)))
#define __ac_set_isempty_false(flags, o) (flags[o>>4]&=~(2ul<<((o&0xfU)<<1)))
#define __ac_set_isboth_false(flags, o) (flags[o>>4]&=~(3ul<<((o&0xfU)<<1)))
#define __ac_set_isdel_true(flags, o) (flags[o>>4]|=1ul<<((o&0xfU)<<1))

#define __ac_fsize(m) ((m) < 16? 1 : (m)>>4)

#ifndef kroundup32
#define kroundup32(x) (--(x), (x)|=(x)>>1, (x)|=(x)>>2, (x)|=(x)>>4, (x)|=(x)>>8, (x)|=(x)>>16, ++(x))
#endif

#ifndef kcalloc
#define kcalloc(N,Z) calloc(N,Z)
#endif
#ifndef kmalloc
#define kmalloc(Z) malloc(Z)
#endif
#ifndef krealloc
#define krealloc(P,Z) realloc(P,Z)
#endif
#ifndef kfree
#define kfree(P) free(P)
#endif


static const double __ac_HASH_UPPER = 0.77;

#define RESIZE_FAILED -1
#define OK 0
#define NO_RESIZE 0 

#define EXISTS 0
#define EMPTY 1
#define TOMBSTONE 2


//u32 __hash_fn(key_t key);
// key_t __key_from_object_fn(ht_struct(name)* h, object_t obj)


#define __BK_H_IMPL(name, SCOPE, object_t, key_t, __hash_fn, __found_fn, __key_from_object_fn)                          \
                                                                                                                        \
    SCOPE struct ht_##name *ht_create_##name(void) {                                                                    \
        return (struct ht_##name*) kcalloc(1, sizeof(struct ht_##name));                                                \
    }                                                                                                                   \
                                                                                                                        \
    SCOPE void ht_trash_##name(struct ht_##name *h) {                                                                   \
        if (h) { kfree((void *)h->slots); kfree(h->flags); kfree(h); }                                                  \
    }                                                                                                                   \
                                                                                                                        \
    SCOPE void ht_clear_##name(struct ht_##name *h) {                                                                   \
        if (h && h->flags) {                                                                                            \
            memset(h->flags, 0xaa, __ac_fsize(h->n_slots) * sizeof(u32));                                               \
            h->n_objects = h->n_slots_used = 0;                                                                         \
        }                                                                                                               \
    }                                                                                                                   \
                                                                                                                        \
    SCOPE u32 ht_get_it_##name(struct ht_##name const * const h, key_t key) {                                           \
        u32 k, o, last, mask, step = 0;                                                                                 \
        if (!h->n_slots) return 0;                                                                                      \
        mask = h->n_slots - 1;                                                                                          \
        k = __hash_fn(key);                                                                                             \
        o = k & mask;                                                                                                   \
        last = o;                                                                                                       \
        while (!__ac_isempty(h->flags, o) && (__ac_isdel(h->flags, o) || !__found_fn(h, h->slots[o], key))) {           \
            o = (o + (++step)) & mask;                                                                                  \
            if (o == last) return h->n_slots;                                                                           \
        }                                                                                                               \
        return __ac_iseither(h->flags, o) ? h->n_slots : o;                                                             \
    }                                                                                                                   \
                                                                                                                        \
    SCOPE int ht_resize_##name(struct ht_##name *h, u32 new_n_slots) {                                                  \
        /* Uses 0.25*n_slots bytes of working space. */                                                                 \
        u32 *new_flags;                                                                                                 \
        kroundup32(new_n_slots);                                                                                        \
        if (new_n_slots < 4) new_n_slots = 4;                                                                           \
        if (h->n_objects >= (u32)(new_n_slots * __ac_HASH_UPPER + 0.5)) return NO_RESIZE;                               \
        new_flags = (u32*)kmalloc(__ac_fsize(new_n_slots) * sizeof(u32));                                               \
        if (!new_flags) return RESIZE_FAILED;                                                                           \
        memset(new_flags, 0xaa, __ac_fsize(new_n_slots) * sizeof(u32));                                                 \
        if (h->n_slots < new_n_slots) { /* expand */                                                                    \
            object_t *new_slots = (object_t*)krealloc((void *)h->slots, new_n_slots * sizeof(object_t));                \
            if (!new_slots) { kfree(new_flags); return RESIZE_FAILED; }                                                 \
            h->slots = new_slots;                                                                                       \
        }                                                                                                               \
        else { /* shrink */                                                                                             \
        }                                                                                                               \
        /* rehash */                                                                                                    \
        for (u32 j = 0; j != h->n_slots; ++j) {                                                                         \
            if (__ac_iseither(h->flags, j) == 0) {                                                                      \
                object_t obj = h->slots[j];                                                                             \
                u32 new_mask;                                                                                           \
                new_mask = new_n_slots - 1;                                                                             \
                __ac_set_isdel_true(h->flags, j);                                                                       \
                while (1) { /* kick-out process; sort of like in Cuckoo hashing */                                      \
                    u32 k, i, step = 0;                                                                                 \
                    k = __hash_fn(__key_from_object_fn(h, obj));                                                        \
                    i = k & new_mask;                                                                                   \
                    while (!__ac_isempty(new_flags, i)) i = (i + (++step)) & new_mask;                                  \
                    __ac_set_isempty_false(new_flags, i);                                                               \
                    if (i < h->n_slots && __ac_iseither(h->flags, i) == 0) {                                            \
                        /* kick out the existing element */                                                             \
                        { object_t tmp = h->slots[i]; h->slots[i] = obj; obj = tmp; }                                   \
                        __ac_set_isdel_true(h->flags, i); /* mark as deleted in the old hash table */                   \
                    }                                                                                                   \
                    else { /* write the element and jump out of the loop */                                             \
                        h->slots[i] = obj;                                                                              \
                        break;                                                                                          \
                    }                                                                                                   \
                }                                                                                                       \
            }                                                                                                           \
        }                                                                                                               \
        if (h->n_slots > new_n_slots) { /* shrink hash table */                                                         \
            h->slots = (object_t *) krealloc((void *)h->slots, new_n_slots * sizeof(object_t));                         \
        }                                                                                                               \
        kfree(h->flags);                                                                                                \
        h->flags = new_flags;                                                                                           \
        h->n_slots = new_n_slots;                                                                                       \
        h->n_slots_used = h->n_objects;                                                                                 \
        h->n_slots_used_threshold = (u32)(h->n_slots * __ac_HASH_UPPER + 0.5);                                          \
        return OK;                                                                                                      \
    }                                                                                                                   \
                                                                                                                        \
    SCOPE u32 ht_put_idx_##name(struct ht_##name *h, key_t key, int *ret) {                                             \
        u32 o;                                                                                                          \
        if (h->n_slots_used >= h->n_slots_used_threshold) { /* update the hash table */                                 \
            if (h->n_slots > (h->n_objects<<1)) {                                                                       \
                if (ht_resize_##name(h, h->n_slots - 1) == RESIZE_FAILED) { /* clear "deleted" elements */              \
                    *ret = RESIZE_FAILED; return h->n_slots;                                                            \
                }                                                                                                       \
            } else if (ht_resize_##name(h, h->n_slots + 1) == RESIZE_FAILED) { /* expand the hash table */              \
                *ret = RESIZE_FAILED; return h->n_slots;                                                                \
            }                                                                                                           \
        } /* TODO: to implement automatically shrinking; resize() already support shrinking */                          \
        {                                                                                                               \
            u32 k, i, site, last, mask = h->n_slots - 1, step = 0;                                                      \
            o = site = h->n_slots; k = __hash_fn(key); i = k & mask;                                                    \
            if (__ac_isempty(h->flags, i)) o = i; /* for speed up */                                                    \
            else {                                                                                                      \
                last = i;                                                                                               \
                while (!__ac_isempty(h->flags, i) && (__ac_isdel(h->flags, i) || !__found_fn(h, h->slots[i], key))) {   \
                    if (__ac_isdel(h->flags, i)) site = i;                                                              \
                    i = (i + (++step)) & mask;                                                                          \
                    if (i == last) { o = site; break; }                                                                 \
                }                                                                                                       \
                if (o == h->n_slots) {                                                                                  \
                    if (__ac_isempty(h->flags, i) && site != h->n_slots) o = site;                                      \
                    else o = i;                                                                                         \
                }                                                                                                       \
            }                                                                                                           \
        }                                                                                                               \
        if (__ac_isempty(h->flags, o)) *ret = EMPTY;                                                                    \
        else if (__ac_isdel(h->flags, o)) *ret = TOMBSTONE;                                                             \
        else *ret = EXISTS;                                                                                             \
        return o;                                                                                                       \
    }                                                                                                                   \
                                                                                                                        \
    SCOPE void ht_replace_empty_##name(struct ht_##name *h, u32 o, object_t obj) {                                      \
        h->slots[o] = obj;                                                                                              \
        __ac_set_isboth_false(h->flags, o);                                                                             \
        ++h->n_objects;                                                                                                 \
        ++h->n_slots_used;                                                                                              \
    }                                                                                                                   \
                                                                                                                        \
    SCOPE void ht_replace_tombstone_##name(struct ht_##name *h, u32 o, object_t obj) {                                  \
        h->slots[o] = obj;                                                                                              \
        __ac_set_isboth_false(h->flags, o);                                                                             \
        ++h->n_objects;                                                                                                 \
    }                                                                                                                   \
                                                                                                                        \
    SCOPE void ht_replace_existing_##name(struct ht_##name *h, u32 o, object_t obj) {                                   \
        h->slots[o] = obj;                                                                                              \
    }                                                                                                                   \
                                                                                                                        \
    SCOPE void ht_del_##name(struct ht_##name *h, u32 o) {                                                              \
        if (o != h->n_slots && !__ac_iseither(h->flags, o)) {                                                           \
            __ac_set_isdel_true(h->flags, o);                                                                           \
            --h->n_objects;                                                                                             \
        }                                                                                                               \
    }
    
#define BK_H_IMPL(name, object_t, key_t, __hash_fn, __found_fn, __key_from_object_fn)                                   \
    __BK_H_IMPL(name, static bk_inline bk_unused, object_t, key_t, __hash_fn, __found_fn, __key_from_object_fn)


// ---------------------------------------------------------------------------------------------------------------------
// HASH FUNCTIONS
// ---------------------------------------------------------------------------------------------------------------------

/*! @function
  @abstract     Integer hash function
  @param  key   The integer [u32 or i32]
  @return       The hash value [u32 or i32]
 */
#define ht_int_hash(key) (u32)(key)

/*! @function
  @abstract     64-bit integer hash function
  @param  key   The integer [khint64_t]
  @return       The hash value [u32]
 */
#define ht_int64_hash(h, key) (u32)((key)>>33^(key)^(key)<<11)

/*! @function
  @abstract     const char* hash function
  @param  key   Pointer to a null terminated string
  @return       The hash value
 */
static bk_inline u32 __ac_X31_hash_string(const char * key) {
    u32 hash = (u32)*key;
    if (hash) for (++key ; *key; ++key) hash = (hash << 5) - hash + (u32)*key;
    return hash;
}

/*! @function
  @abstract     Another interface to const char* hash function
  @param  key   Pointer to a null terminated string [const char*]
  @return       The hash value [u32]
 */
#define ht_str_hash_func(key) __ac_X31_hash_string(key)

static bk_inline u32 __ac_Wang_hash(u32 key) {
    key += ~(key << 15);
    key ^=  (key >> 10);
    key +=  (key << 3);
    key ^=  (key >> 6);
    key += ~(key << 11);
    key ^=  (key >> 16);
    return key;
}
#define ht_int_hash_func2(h, key) __ac_Wang_hash((u32) key)


#endif // __BK_HT_IMPL_H