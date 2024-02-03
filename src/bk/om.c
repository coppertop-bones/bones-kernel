#ifndef __BK_OM_C
#define __BK_OM_C "bk/om.c"

#include <setjmp.h>
#include <string.h>

#include "../../include/bk/bk.h"
#include "../../include/bk/om.h"

pub OM * OM_create(BK_MM *mm, BK_TM *tm) {
    OM *om = mm->malloc(sizeof(OM));
    om->mm = mm;
    om->tm = tm;
    om->start = mm->malloc(MM_BLOCK_SIZE);
    om->endOf = om->start + MM_BLOCK_SIZE;
    om->alloctor = mm->malloc(sizeof(struct OMAlloctor));
    om->alloctor->next = (OMSlot *) om->start + 1;
    om->alloctor->endOf = (OMSlot *) om->endOf;
    return om;
}

pub void OM_trash(OM *om) {

}

// agents could have destructors called? but this means every pointer would have to have a check
// instead of auto calling destructors in the VM the language must provide for it



// create objects with ref count = 1 which means there is an object here
// objects with only one ref can be mutated
// objects whose ref goes from 1 to 0 effectively die

// to pass a value to a function we have to inc it's count before passing if we don't want it to change, and dec it's count after
// an object with count of 1 may be mutated by the function / callee

// lines
// lines are marked as used or not (no count, count would need to be 4 bits to count, or 16 bits to know which slots in a line were used)
// if just marked line marks are cleared on trace and marked as used. if ref count lines then we can reuse a line potentially without a trace

// lines count is set on trace not on creation
// one allocator per block


// or on object creation inc line rc, dec on drop - but we want them to be born dead -> have a bunch of blocks awaiting tracing that we can't allocate into

// object pointer will be modulo 16 and its corresponding type pointer will modulo 16 minus 4

pub void * om_alloc(OM *om, btypeid_t btypeid) {
    void *answer;  int numslots;
    // answers a pointer to a newly allocated object
    // if this fails we have a serious problem - so longjmp to failure handler

    // 1) get size from type
    int sz = tm_size(om->tm, btypeid);
    numslots = 1 + sz / MM_SLOT_SIZE;

    // 2) allocate in block (if large alloc elsewhere)
    answer = om->alloctor->next;
    om->alloctor->next += numslots;

    // 3) prefix type, OPEN: sort flags, rc should be 0
    memcpy((btypeid_t *) answer - 1, &btypeid, sizeof(btypeid_t));

    return answer;
}

pub void * om_valloc(OM *om, btypeid_t bt, size sz) {
    // answers a pointer to a newly allocated variable size object
    // if this fails we have a serious problem - use longjmp to failure handler
    return 0;
}

pub void * om_revalloc(OM *om, btypeid_t bt, size sz) {
    // answers a pointer to a newly allocated object
    // if this fails we have a serious problem - use longjmp to failure handler
    return 0;
}


// moveToGlobal - copies to another block, compacts redoes rc, set's line usage
// markAsGlobal - leaves in place, sets line usage

// GC - clear line usage, traverse objects, set line usag



// born dead? if we are doing CoW then we can't coalesse
// need the cow count, e.g.

// a: ((1,2), (3,4))    o3.rc == 1, o1.rc == 1, o2.rc == 1
// b: a                 o3.rc == 2, o1.rc == 1, o2.rc == 1
// a[1][1] = 5          o4.rc == 1, o5.rc == 1, o3.rc == 1, o1.rc == 1, o2.rc == 2
// drop a               o3.rc == 1, o1.rc == 1, o2.rc == 1, o4.rc == 0, o5.rc == 0
// return b             o3.rc == 1, o1.rc == 1, o2.rc == 1


// o1: row1 (1,2)
// o2: row2 (3,4)
// o3: matrix(o1, o2)
// o4: row1 (5, 2)
// o5: matrix (o4, o2)



// mark

// get new roots - as born dead
// visit each
//   mark line as used
//   inc rc
// visit conservative roots
//   check isObject (rc > 0)
//   mark temporarily pinned (remember?)
// visit known roots (global kv store)



pub int om_count(OM *om, void *p) {
    return 0;
}

pub int om_inc(OM *om, void *p) {
    return 0;
}

pub int om_dec(OM *om, void *p) {
    return 0;
}

pub int om_btypeid(OM *om, void *p) {
    return 0;
}

pub int om_mark_line(OM *om, void *p) {
    return 0;
}

pub int om_is(OM *om, void *p) {
    // answer true if p points to the start of an object
    if (p < om->start || om->endOf <= p) return 0;
    // OPEN: check the slot table
    return 1;
}

pub int om_is_within(OM *om, void *p) {
    if (p < om->start || om->endOf <= p) return 0;
    // OPEN: implment
    // if this line == 0 and this line - 1 == 0 return 0;
    // scan back up to OM_LARGE_OBJECT_SIZE if no slot return 0;
    // get size from type if p > size return 0;
    return 1;
}

#endif      // __BK_OM_C


