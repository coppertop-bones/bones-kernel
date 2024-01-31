#include "../bk/pp.c"
#include "../lib/hi_impl.c"

pvt void die_(char *preamble, char *msg, va_list args) {
    fprintf(stderr, "%s", preamble);
    vfprintf(stderr, msg, args);
    exit(1);
}

HI_STRUCT(hi_i32, int)

pvt bool matchesKey(hi_struct(hi_i32) *h, int entry, int key) {
    return entry == key;
}

pvt inline int keyFromEntry(hi_struct(hi_i32) *h, int entry) {
    return entry;
}

// HI_IMPL(name, token_t, hashable_t, __hash_fn, __found_fn, __hashable_from_token_fn)
HI_IMPL(hi_i32, int, int, hi_int32_hash, matchesKey, keyFromEntry)


int main() {
    int res, is_missing, idx, sum, obj, key;

    hi_struct(hi_i32) *h = hi_create(hi_i32);
    check(h->sz == 0, "sz == %i (should be 0)", h->sz);

    idx = hi_put_idx(hi_i32, h, 5, &res);
    check(h->sz == 4, "sz == %i (should be 4)", h->sz);
    check(idx == 1, "sz == %i (should be 1)", h->sz);
    check(res == HI_EMPTY, "res != HI_EMPTY");

    idx = hi_put_idx(hi_i32, h, 6, &res);
    check(h->sz == 4, "sz == %i (should be 4)", h->sz);
    check(idx == 2, "sz == %i (should be 2)", h->sz);
    check(res == HI_EMPTY, "res != HI_EMPTY");

    idx = hi_put_idx(hi_i32, h, 7, &res);
    check(h->sz == 4, "sz == %i (should be 4)", h->sz);
    check(idx == 3, "sz == %i (should be 3)", h->sz);
    check(res == HI_EMPTY, "res != HI_EMPTY");

    obj = 8;
    idx = hi_put_idx(hi_i32, h, obj, &res);
    check(h->sz == 4, "sz == %i (should be 4)", h->sz);
    check(idx == 0, "sz == %i (should be 0)", h->sz);
    check(res == HI_EMPTY, "res != HI_EMPTY");

    check(h->slots[idx] == 0, "pre put == %i (should be 0)", h->slots[idx]);
    hi_replace_empty_hi_i32(h, idx, obj);
    check(h->slots[idx] == 8, "post put == %i (should be 8)", h->slots[idx]);

    key = 10;
    idx = hi_get_idx(hi_i32, h, key);
    check(hi_get_idx(hi_i32, h, key) == hi_end(h), "post get == %i (should be %i)", hi_get_idx(hi_i32, h, key), hi_end(h));

    key = 8;
    idx = hi_get_idx(hi_i32, h, key);
    check(idx == 0, "post get == %i (should be 0)", idx);

    sum = 0;
    hi_foreach(h, obj, {
        sum += obj;
    })
    check(sum == 8, "sum == %i (should be %i)", sum, 8);

    check(h->flags[idx] == (0xAAAAAAAA & 0xFFFFFFFC), "pre del flags[idx] == %i (should be %i)", h->flags[idx], (0xAAAAAAAA & 0xFFFFFFFC));
    hi_drop(hi_i32, h, idx);
    check(h->flags[idx] == (0xAAAAAAA9), "post del flags[idx] == %i (should be %i)", h->flags[idx], (0xAAAAAAA9));   // 0b1001

    hi_trash(hi_i32, h);

    PP(info, "passed");
    return 0;
}
