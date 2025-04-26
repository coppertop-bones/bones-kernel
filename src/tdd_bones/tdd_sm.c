// ---------------------------------------------------------------------------------------------------------------------
//
//                             Copyright (c) 2019-2025 David Briant. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance
// with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software distributed under the License is distributed
// on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for
// the specific language governing permissions and limitations under the License.
//
// ---------------------------------------------------------------------------------------------------------------------


// ---------------------------------------------------------------------------------------------------------------------
// TDD TESTS - SYM MANAGER
// ---------------------------------------------------------------------------------------------------------------------

#include "../bk/pp.c"
#include "../bk/mm.c"
#include "../bk/tp.c"
#include "../bk/k.c"

pvt void die_(char *preamble, char *msg, va_list args) {
    fprintf(stderr, "%s", preamble);
    vfprintf(stderr, msg, args);
    exit(1);
}

int main() {
    int id;
    BK_MM *mm = MM_create();
    Buckets *buckets = mm->malloc(sizeof(Buckets));
    Buckets_init(buckets, 64);
    BK_K *k = K_create(mm, buckets);
    PP(info, "kernel created");

    int off = 6;

    id = sm_id(k->sm, "fred");
    check(id == 1 + off, "id == %i (should be %i)", id, 1 + off);

    id = sm_id(k->sm, "fred");
    check(id == 1 + off, "id == %i (should be %i)", id, 1 + off);

    id = sm_id(k->sm, "joe");
    check(id == 2 + off, "id == %i (should be %i)", id, 2 + off);

    id = tm_lookup(k->tm, "joe");
    check(id == 0, "id == %i (should be %i)", id, 0);

    id = tm_lookup(k->tm, "sally");
    check(id == 0, "id == %i (should be %i)", id, 0);


    K_trash(k);
    Buckets_finalise(buckets);
    free(buckets);
    PP(info, "passed");
    return 0;
}
