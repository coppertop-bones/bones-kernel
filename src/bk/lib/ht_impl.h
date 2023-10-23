#ifndef __BK_HT_IMPL_H
#define __BK_HT_IMPL_H


#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "../../../include/bk/ht.h"



// 1 u32 flag per 16 slots (2 bits per slot) - 0b10 -> empty and not a tombstone
#define __ht_fsize(m) ((m) < 16? 1 : (m)>>4)
#define __ht_is_tomb(flags, idx) ( (flags[idx>>4]>> ((idx&0xfU)<<1) ) & 1 )
#define __ht_is_empty(flags, idx) ( (flags[idx>>4]>>((idx&0xfU)<<1)) & 2 )
#define __ht_is_occupied(flags, idx) ( !((flags[idx>>4]>>((idx&0xfU)<<1)) & 3) )
#define __ht_set_not_empty(flags, idx) (flags[idx>>4]&=~(2ul<<((idx&0xfU)<<1)))
#define __ht_set_occupied(flags, idx) (flags[idx>>4]&=~(3ul<<((idx&0xfU)<<1)))
#define __ht_set_tomb(flags, idx) (flags[idx>>4]|=1ul<<((idx&0xfU)<<1))


#ifndef kroundup32
#define kroundup32(x) (--(x), (x)|=(x)>>1, (x)|=(x)>>2, (x)|=(x)>>4, (x)|=(x)>>8, (x)|=(x)>>16, ++(x))
#endif


static const double __ht_HASH_UPPER = 0.77;

#define RESIZE_FAILED -1
#define RESIZED_OK 0
#define RESIZE_NOT_NEEDED 0


// u32 __hash_fn(key_t key);
// bool __found_fn(ht_struct(name)* h, slot_t entry, key_t key)
// key_t __key_from_entry_fn(ht_struct(name)* h, slot_t entry)


#define __HT_IMPL(name, SCOPE, slot_t, key_t, __hash_fn, __found_fn, __key_from_entry_fn)                               \
                                                                                                                        \
    SCOPE struct ht_##name *ht_create_##name(void) {                                                                    \
        return (struct ht_##name*) calloc(1, sizeof(struct ht_##name));                                                 \
    }                                                                                                                   \
                                                                                                                        \
    SCOPE void ht_trash_##name(struct ht_##name *h) {                                                                   \
        if (h) { free((void *)h->slots); free(h->flags); free(h); }                                                     \
    }                                                                                                                   \
                                                                                                                        \
    SCOPE void ht_clear_##name(struct ht_##name *h) {                                                                   \
        if (h && h->flags) {                                                                                            \
            memset(h->flags, 0xaa, __ht_fsize(h->n_slots) * sizeof(u32));                                               \
            h->n_entries = h->n_slots_used = 0;                                                                         \
        }                                                                                                               \
    }                                                                                                                   \
                                                                                                                        \
    SCOPE u32 ht_get_idx_##name(struct ht_##name *h, key_t key) {                                                       \
        u32 k, idx, last, mask, step = 0;                                                                               \
        if (!h->n_slots) return 0;                                                                                      \
        mask = h->n_slots - 1;                                                                                          \
        k = __hash_fn(key);                                                                                             \
        idx = k & mask;                                                                                                 \
        last = idx;                                                                                                     \
        while (!__ht_is_empty(h->flags, idx) && (__ht_is_tomb(h->flags, idx) || !__found_fn(h, h->slots[idx], key))) {  \
            idx = (idx + (++step)) & mask;                                                                              \
            if (idx == last) return h->n_slots;                                                                         \
        }                                                                                                               \
        return __ht_is_occupied(h->flags, idx) ? idx : h->n_slots;                                                      \
    }                                                                                                                   \
                                                                                                                        \
    SCOPE int ht_resize_##name(struct ht_##name *h, u32 new_n_slots) {                                                  \
        /* Uses 0.25*n_slots bytes of working space. */                                                                 \
        u32 *new_flags;                                                                                                 \
        kroundup32(new_n_slots);                                                                                        \
        if (new_n_slots < 4) new_n_slots = 4;                                                                           \
        if (h->n_entries >= (u32)(new_n_slots * __ht_HASH_UPPER + 0.5)) return RESIZE_NOT_NEEDED;                       \
        new_flags = (u32*)malloc(__ht_fsize(new_n_slots) * sizeof(u32));                                                \
        if (!new_flags) return RESIZE_FAILED;                                                                           \
        memset(new_flags, 0xaa, __ht_fsize(new_n_slots) * sizeof(u32));                                                 \
        if (h->n_slots < new_n_slots) { /* expand */                                                                    \
            slot_t *new_slots = (slot_t*)realloc((void *)h->slots, new_n_slots * sizeof(slot_t));                       \
            if (!new_slots) { free(new_flags); return RESIZE_FAILED; }                                                  \
            h->slots = new_slots;                                                                                       \
        }                                                                                                               \
        else { /* shrink */                                                                                             \
        }                                                                                                               \
        /* rehash */                                                                                                    \
        for (u32 j = 0; j != h->n_slots; ++j) {                                                                         \
            if (__ht_is_occupied(h->flags, j)) {                                                                        \
                slot_t entry = h->slots[j];                                                                             \
                u32 new_mask;                                                                                           \
                new_mask = new_n_slots - 1;                                                                             \
                __ht_set_tomb(h->flags, j);                                                                             \
                while (1) { /* kick-out process; sort of like in Cuckoo hashing */                                      \
                    u32 k, i, step = 0;                                                                                 \
                    k = __hash_fn(__key_from_entry_fn(h, entry));                                                       \
                    i = k & new_mask;                                                                                   \
                    while (!__ht_is_empty(new_flags, i)) i = (i + (++step)) & new_mask;                                 \
                    __ht_set_not_empty(new_flags, i);                                                                   \
                    if (i < h->n_slots && __ht_is_occupied(h->flags, i)) {                                              \
                        /* kick out the existing element */                                                             \
                        { slot_t tmp = h->slots[i]; h->slots[i] = entry; entry = tmp; }                                 \
                        __ht_set_tomb(h->flags, i); /* mark as deleted in the old hash table */                         \
                    }                                                                                                   \
                    else { /* write the element and jump out of the loop */                                             \
                        h->slots[i] = entry;                                                                            \
                        break;                                                                                          \
                    }                                                                                                   \
                }                                                                                                       \
            }                                                                                                           \
        }                                                                                                               \
        if (h->n_slots > new_n_slots) { /* shrink hash table */                                                         \
            h->slots = (slot_t *) realloc((void *)h->slots, new_n_slots * sizeof(slot_t));                              \
        }                                                                                                               \
        free(h->flags);                                                                                                 \
        h->flags = new_flags;                                                                                           \
        h->n_slots = new_n_slots;                                                                                       \
        h->n_slots_used = h->n_entries;                                                                                 \
        h->n_slots_used_threshold = (u32)(h->n_slots * __ht_HASH_UPPER + 0.5);                                          \
        return RESIZED_OK;                                                                                              \
    }                                                                                                                   \
                                                                                                                        \
    SCOPE u32 ht_put_idx_##name(struct ht_##name *h, key_t key, int *ret) {                                             \
        u32 idx;                                                                                                        \
        if (h->n_slots_used >= h->n_slots_used_threshold) { /* update the hash table */                                 \
            if (h->n_slots > (h->n_entries<<1)) {                                                                       \
                if (ht_resize_##name(h, h->n_slots - 1) == RESIZE_FAILED) { /* clear "deleted" elements */              \
                    *ret = RESIZE_FAILED; return h->n_slots;                                                            \
                }                                                                                                       \
            } else if (ht_resize_##name(h, h->n_slots + 1) == RESIZE_FAILED) { /* expand the hash table */              \
                *ret = RESIZE_FAILED; return h->n_slots;                                                                \
            }                                                                                                           \
        } /* TODO: to implement automatically shrinking; resize() already support shrinking */                          \
        {                                                                                                               \
            u32 k, i, site, last, mask = h->n_slots - 1, step = 0;                                                      \
            idx = site = h->n_slots; k = __hash_fn(key); i = k & mask;                                                  \
            if (__ht_is_empty(h->flags, i)) idx = i; /* for speed up */                                                 \
            else {                                                                                                      \
                last = i;                                                                                               \
                while (!__ht_is_empty(h->flags, i) && (__ht_is_tomb(h->flags, i) || !__found_fn(h, h->slots[i], key))) {\
                    if (__ht_is_tomb(h->flags, i)) site = i;                                                            \
                    i = (i + (++step)) & mask;                                                                          \
                    if (i == last) { idx = site; break; }                                                               \
                }                                                                                                       \
                if (idx == h->n_slots) {                                                                                \
                    if (__ht_is_empty(h->flags, i) && site != h->n_slots) idx = site;                                   \
                    else idx = i;                                                                                       \
                }                                                                                                       \
            }                                                                                                           \
        }                                                                                                               \
        if (__ht_is_empty(h->flags, idx)) *ret = HT_EMPTY;                                                              \
        else if (__ht_is_tomb(h->flags, idx)) *ret = HT_TOMBSTONE;                                                      \
        else *ret = HT_EXISTS;                                                                                          \
        return idx;                                                                                                     \
    }                                                                                                                   \
                                                                                                                        \
    SCOPE void ht_replace_empty_##name(struct ht_##name *h, u32 idx, slot_t entry) {                                    \
        h->slots[idx] = entry;                                                                                          \
        __ht_set_occupied(h->flags, idx);                                                                               \
        ++h->n_entries;                                                                                                 \
        ++h->n_slots_used;                                                                                              \
    }                                                                                                                   \
                                                                                                                        \
    SCOPE void ht_replace_tombstone_##name(struct ht_##name *h, u32 idx, slot_t entry) {                                \
        h->slots[idx] = entry;                                                                                          \
        __ht_set_occupied(h->flags, idx);                                                                               \
        ++h->n_entries;                                                                                                 \
    }                                                                                                                   \
                                                                                                                        \
    SCOPE void ht_replace_existing_##name(struct ht_##name *h, u32 idx, slot_t entry) {                                 \
        h->slots[idx] = entry;                                                                                          \
    }                                                                                                                   \
                                                                                                                        \
    SCOPE void ht_del_##name(struct ht_##name *h, u32 idx) {                                                            \
        if (idx != h->n_slots && __ht_is_occupied(h->flags, idx)) {                                                     \
            __ht_set_tomb(h->flags, idx);                                                                               \
            --h->n_entries;                                                                                             \
        }                                                                                                               \
    }
    
#define HT_IMPL(name, slot_t, key_t, __hash_fn, __found_fn, __key_from_entry_fn)                                        \
    __HT_IMPL(name, static bk_inline bk_unused, slot_t, key_t, __hash_fn, __found_fn, __key_from_entry_fn)


// ---------------------------------------------------------------------------------------------------------------------
// HASH FUNCTIONS
// ---------------------------------------------------------------------------------------------------------------------

/*! @function
  @abstract     Integer hash function
  @param  key   The integer [u32 or i32]
  @return       The hash value [u32]
 */
#define ht_int32_hash(key) (u32)(key)

/*! @function
  @abstract     64-bit integer hash function
  @param  key   The integer [khint64_t]
  @return       The hash value [u32]
 */
#define ht_int64_hash(h, key) (u32)((key)>>33^(key)^(key)<<11)

/*! @function
  @abstract     char* hash function
  @param  key   Pointer to a null terminated string
  @return       The hash value
 */
static bk_inline u32 __ht_X31_hash_string(char * key) {
    u32 hash = (u32)*key;
    if (hash) for (++key ; *key; ++key) hash = (hash << 5) - hash + (u32)*key;
    return hash;
}

/*! @function
  @abstract     Another interface to char * hash function
  @param  key   Pointer to a null terminated string [char *]
  @return       The hash value [u32]
 */
#define ht_str_hash(key) __ht_X31_hash_string(key)

static bk_inline u32 __ht_Wang_hash(u32 key) {
    key += ~(key << 15);
    key ^=  (key >> 10);
    key +=  (key << 3);
    key ^=  (key >> 6);
    key += ~(key << 11);
    key ^=  (key >> 16);
    return key;
}
#define ht_int32_wang_hash(h, key) __ht_Wang_hash((u32) key)


#endif // __BK_HT_IMPL_H