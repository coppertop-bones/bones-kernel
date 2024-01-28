#ifndef __BK_HT_IMPL_H
#define __BK_HT_IMPL_H


#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "../../../include/bk/ht.h"



// 1 u32 flag per 16 entries (2 bits per entry) - 0b10 -> empty and not a tombstone
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


// u32 __hash_fn(hashable_t h);
// bool __found_fn(ht_struct(name)* ht, entry_t entry, hashable_t h)
// hashable_t __hashable_from_entry_fn(ht_struct(name)* ht, entry_t entry)


#define __HT_IMPL(name, SCOPE, entry_t, hashable_t, __hash_fn, __found_fn, __hashable_from_entry_fn)                    \
                                                                                                                        \
    SCOPE struct ht_##name *ht_create_##name(void) {                                                                    \
        return (struct ht_##name*) calloc(1, sizeof(struct ht_##name));                                                 \
    }                                                                                                                   \
                                                                                                                        \
    SCOPE void ht_trash_##name(struct ht_##name *ht) {                                                                  \
        if (ht) { free((void *)ht->entries); free(ht->flags); free(ht); }                                               \
    }                                                                                                                   \
                                                                                                                        \
    SCOPE void ht_clear_##name(struct ht_##name *ht) {                                                                  \
        if (ht && ht->flags) {                                                                                          \
            memset(ht->flags, 0xaa, __ht_fsize(ht->sz) * sizeof(u32));                                                  \
            ht->n_live = ht->n_used = 0;                                                                                \
        }                                                                                                               \
    }                                                                                                                   \
                                                                                                                        \
    SCOPE u32 ht_get_idx_##name(struct ht_##name *ht, hashable_t h) {                                                   \
        u32 hash, idx, last, mask, step = 0;                                                                            \
        if (!ht->sz) return 0;                                                                                          \
        mask = ht->sz - 1;                                                                                              \
        hash = __hash_fn(h);                                                                                            \
        idx = hash & mask;                                                                                              \
        last = idx;                                                                                                     \
        while (!__ht_is_empty(ht->flags, idx) && (__ht_is_tomb(ht->flags, idx) || !__found_fn(ht, ht->entries[idx], h))) {\
            idx = (idx + (++step)) & mask;                                                                              \
            if (idx == last) return ht->sz;                                                                             \
        }                                                                                                               \
        return __ht_is_occupied(ht->flags, idx) ? idx : ht->sz;                                                         \
    }                                                                                                                   \
                                                                                                                        \
    SCOPE int ht_resize_##name(struct ht_##name *ht, u32 new_sz) {                                                      \
        /* Uses 0.25*sz bytes of working space. */                                                                      \
        u32 *new_flags;                                                                                                 \
        kroundup32(new_sz);                                                                                             \
        if (new_sz < 4) new_sz = 4;                                                                                     \
        if (ht->n_live >= (u32)(new_sz * __ht_HASH_UPPER + 0.5)) return RESIZE_NOT_NEEDED;                              \
        new_flags = (u32*)malloc(__ht_fsize(new_sz) * sizeof(u32));                                                     \
        if (!new_flags) return RESIZE_FAILED;                                                                           \
        memset(new_flags, 0xaa, __ht_fsize(new_sz) * sizeof(u32));                                                      \
        if (ht->sz < new_sz) { /* expand */                                                                             \
            entry_t *new_slots = (entry_t*)realloc((void *)ht->entries, new_sz * sizeof(entry_t));                      \
            if (!new_slots) { free(new_flags); return RESIZE_FAILED; }                                                  \
            ht->entries = new_slots;                                                                                    \
        }                                                                                                               \
        else { /* shrink */                                                                                             \
        }                                                                                                               \
        /* rehash */                                                                                                    \
        for (u32 j = 0; j != ht->sz; ++j) {                                                                             \
            if (__ht_is_occupied(ht->flags, j)) {                                                                       \
                entry_t entry = ht->entries[j];                                                                         \
                u32 new_mask;                                                                                           \
                new_mask = new_sz - 1;                                                                                  \
                __ht_set_tomb(ht->flags, j);                                                                            \
                while (1) { /* kick-out process; sort of like in Cuckoo hashing */                                      \
                    u32 hash, i, step = 0;                                                                              \
                    hash = __hash_fn(__hashable_from_entry_fn(ht, entry));                                              \
                    i = hash & new_mask;                                                                                \
                    while (!__ht_is_empty(new_flags, i)) i = (i + (++step)) & new_mask;                                 \
                    __ht_set_not_empty(new_flags, i);                                                                   \
                    if (i < ht->sz && __ht_is_occupied(ht->flags, i)) {                                                 \
                        /* kick out the existing element */                                                             \
                        { entry_t tmp = ht->entries[i]; ht->entries[i] = entry; entry = tmp; }                          \
                        __ht_set_tomb(ht->flags, i); /* mark as deleted in the old hash table */                        \
                    }                                                                                                   \
                    else { /* write the element and jump out of the loop */                                             \
                        ht->entries[i] = entry;                                                                         \
                        break;                                                                                          \
                    }                                                                                                   \
                }                                                                                                       \
            }                                                                                                           \
        }                                                                                                               \
        if (ht->sz > new_sz) { /* shrink hash table */                                                                  \
            ht->entries = (entry_t *) realloc((void *)ht->entries, new_sz * sizeof(entry_t));                           \
        }                                                                                                               \
        free(ht->flags);                                                                                                \
        ht->flags = new_flags;                                                                                          \
        ht->sz = new_sz;                                                                                                \
        ht->n_used = ht->n_live;                                                                                        \
        ht->n_used_threshold = (u32)(ht->sz * __ht_HASH_UPPER + 0.5);                                                   \
        return RESIZED_OK;                                                                                              \
    }                                                                                                                   \
                                                                                                                        \
    SCOPE u32 ht_put_idx_##name(struct ht_##name *ht, hashable_t h, int *outcome) {                                     \
        u32 idx;                                                                                                        \
        if (ht->n_used >= ht->n_used_threshold) { /* update the hash table */                                           \
            if (ht->sz > (ht->n_live<<1)) {                                                                             \
                if (ht_resize_##name(ht, ht->sz - 1) == RESIZE_FAILED) { /* clear "deleted" elements */                 \
                    *outcome = RESIZE_FAILED; return ht->sz;                                                            \
                }                                                                                                       \
            } else if (ht_resize_##name(ht, ht->sz + 1) == RESIZE_FAILED) { /* expand the hash table */                 \
                *outcome = RESIZE_FAILED; return ht->sz;                                                                \
            }                                                                                                           \
        } /* TODO: to implement automatically shrinking; resize() already support shrinking */                          \
        {                                                                                                               \
            u32 hash, i, site, last, mask = ht->sz - 1, step = 0;                                                       \
            idx = site = ht->sz; hash = __hash_fn(h); i = hash & mask;                                                  \
            if (__ht_is_empty(ht->flags, i)) idx = i; /* for speed up */                                                \
            else {                                                                                                      \
                last = i;                                                                                               \
                while (!__ht_is_empty(ht->flags, i) && (__ht_is_tomb(ht->flags, i) || !__found_fn(ht, ht->entries[i], h))) {\
                    if (__ht_is_tomb(ht->flags, i)) site = i;                                                           \
                    i = (i + (++step)) & mask;                                                                          \
                    if (i == last) { idx = site; break; }                                                               \
                }                                                                                                       \
                if (idx == ht->sz) {                                                                                    \
                    if (__ht_is_empty(ht->flags, i) && site != ht->sz) idx = site;                                      \
                    else idx = i;                                                                                       \
                }                                                                                                       \
            }                                                                                                           \
        }                                                                                                               \
        if (__ht_is_empty(ht->flags, idx)) *outcome = HT_EMPTY;                                                         \
        else if (__ht_is_tomb(ht->flags, idx)) *outcome = HT_TOMBSTONE;                                                 \
        else *outcome = HT_LIVE;                                                                                        \
        return idx;                                                                                                     \
    }                                                                                                                   \
                                                                                                                        \
    SCOPE void ht_replace_empty_##name(struct ht_##name *ht, u32 idx, entry_t entry) {                                  \
        ht->entries[idx] = entry;                                                                                       \
        __ht_set_occupied(ht->flags, idx);                                                                              \
        ++ht->n_live;                                                                                                   \
        ++ht->n_used;                                                                                                   \
    }                                                                                                                   \
                                                                                                                        \
    SCOPE void ht_replace_tombstone_##name(struct ht_##name *ht, u32 idx, entry_t entry) {                              \
        ht->entries[idx] = entry;                                                                                       \
        __ht_set_occupied(ht->flags, idx);                                                                              \
        ++ht->n_live;                                                                                                   \
    }                                                                                                                   \
                                                                                                                        \
    SCOPE void ht_replace_entry_##name(struct ht_##name *ht, u32 idx, entry_t entry) {                                  \
        ht->entries[idx] = entry;                                                                                       \
    }                                                                                                                   \
                                                                                                                        \
    SCOPE void ht_del_##name(struct ht_##name *ht, u32 idx) {                                                           \
        if (idx != ht->sz && __ht_is_occupied(ht->flags, idx)) {                                                        \
            __ht_set_tomb(ht->flags, idx);                                                                              \
            --ht->n_live;                                                                                               \
        }                                                                                                               \
    }
    
#define HT_IMPL(name, entry_t, hashable_t, __hash_fn, __found_fn, __hashable_from_entry_fn)                             \
    __HT_IMPL(name, static bk_inline bk_unused, entry_t, hashable_t, __hash_fn, __found_fn, __hashable_from_entry_fn)


// ---------------------------------------------------------------------------------------------------------------------
// HASH FUNCTIONS
// ---------------------------------------------------------------------------------------------------------------------

/*! @function
  @abstract     32-bit hash function
  @param  h     The hashable integer [u32 or i32]
  @return       The hash value [u32]
 */
#define ht_int32_hash(h) (u32)(h)

/*! @function
  @abstract     64-bit integer hash function
  @param  h     The hashable 64 bit integer [u64 or i64]
  @return       The hash value [u32]
 */
#define ht_int64_hash(h) (u32)((h)>>33^(h)^(h)<<11)

/*! @function
  @abstract     char* hash function
  @param  h     Pointer to the hashable null terminated string
  @return       The hash value [u32]
 */
static bk_inline u32 __ht_X31_hash_string(char * h) {
    u32 answer = (u32)*h;
    if (answer) for (++h ; *h; ++h) answer = (answer << 5) - answer + (u32)*h;
    return answer;
}

/*! @function
  @abstract     Another interface to char * hash function
  @param  h     Pointer to a hashable null terminated string [char *]
  @return       The hash value [u32]
 */
#define ht_str_hash(h) __ht_X31_hash_string(h)

static bk_inline u32 __ht_Wang_hash(u32 h) {
    h += ~(h << 15);
    h ^=  (h >> 10);
    h +=  (h << 3);
    h ^=  (h >> 6);
    h += ~(h << 11);
    h ^=  (h >> 16);
    return h;
}
#define ht_int32_wang_hash(h) __ht_Wang_hash((u32) h)


#endif // __BK_HT_IMPL_H