#include "../bk/pp.c"
#include "../lib/ht_impl.c"

pvt void die_(char *preamble, char *msg, va_list args) {
    fprintf(stderr, "%s", preamble);
    vfprintf(stderr, msg, args);
    exit(1);
}

HT_STRUCT(ht_i32, int)

pvt bool matchesKey(ht_struct(ht_i32) *h, int entry, int key) {
    return entry == key;
}

pvt inline int keyFromEntry(ht_struct(ht_i32) *h, int entry) {
    return entry;
}

// HT_IMPL(name, entry_t, hashable_t, __hash_fn, __found_fn, __key_from_entry_fn)
HT_IMPL(ht_i32, int, int, ht_int32_hash, matchesKey, keyFromEntry)


int main() {
    int res, is_missing, idx, sum, obj, key;

    ht_struct(ht_i32) *h = ht_create(ht_i32);
    check(h->sz == 0, "sz == %i (should be 0)", h->sz);

    idx = ht_put_idx(ht_i32, h, 5, &res);
    check(h->sz == 4, "sz == %i (should be 4)", h->sz);
    check(idx == 1, "sz == %i (should be 1)", h->sz);
    check(res == HT_EMPTY, "res != HT_EMPTY");

    idx = ht_put_idx(ht_i32, h, 6, &res);
    check(h->sz == 4, "sz == %i (should be 4)", h->sz);
    check(idx == 2, "sz == %i (should be 2)", h->sz);
    check(res == HT_EMPTY, "res != HT_EMPTY");

    idx = ht_put_idx(ht_i32, h, 7, &res);
    check(h->sz == 4, "sz == %i (should be 4)", h->sz);
    check(idx == 3, "sz == %i (should be 3)", h->sz);
    check(res == HT_EMPTY, "res != HT_EMPTY");

    obj = 8;
    idx = ht_put_idx(ht_i32, h, obj, &res);
    check(h->sz == 4, "sz == %i (should be 4)", h->sz);
    check(idx == 0, "sz == %i (should be 0)", h->sz);
    check(res == HT_EMPTY, "res != HT_EMPTY");

    check(h->slots[idx] == 0, "pre put == %i (should be 0)", h->slots[idx]);
    ht_replace_empty_ht_i32(h, idx, obj);
    check(h->slots[idx] == 8, "post put == %i (should be 8)", h->slots[idx]);

    key = 10;
    idx = ht_get_idx(ht_i32, h, key);
    check(ht_get_idx(ht_i32, h, key) == ht_eot(h), "post get == %i (should be %i)", ht_get_idx(ht_i32, h, key), ht_eot(h));

    key = 8;
    idx = ht_get_idx(ht_i32, h, key);
    check(idx == 0, "post get == %i (should be 0)", idx);

    sum = 0;
    ht_foreach(h, obj, {
        sum += obj;
    })
    check(sum == 8, "sum == %i (should be %i)", sum, 8);

    check(h->flags[idx] == (0xAAAAAAAA & 0xFFFFFFFC), "pre del flags[idx] == %i (should be %i)", h->flags[idx], (0xAAAAAAAA & 0xFFFFFFFC));
    ht_del(ht_i32, h, idx);
    check(h->flags[idx] == (0xAAAAAAA9), "post del flags[idx] == %i (should be %i)", h->flags[idx], (0xAAAAAAA9));   // 0b1001

    ht_trash(ht_i32, h);

    PP(info, "passed");
    return 0;
}
