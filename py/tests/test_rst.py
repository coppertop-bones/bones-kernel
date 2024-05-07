from coppertop.pipe import *
from dm.core.types import pylist, pytuple
from dm.testing import check, raises, equals, gt, different
from bones import jones
from bones.core.errors import NotYetImplemented
import dm.pp
from dm.core import PP

import sys, itertools





def test_passingBlockToControlFn():
    pass

def test_deepAssign():
    # The


    # test
    # a: <N**i32>
    # a[1] typeOf == <:i32>

    sys._k = jones.Kernel()
    sys._k.initCommon()
    tm = sys._k.tm

    i32 = tm.fromName('i32')
    N_i32 = tm.seq(i32)

    # create a symtab and add to local scope
    scope = RstSymTab()
    scope.fns.addName('p', tN_i32)

    fnEquals = RstFn('==', scope)
    fnTypeOf =

    n1 = RstBind('p', RstLitInt(0, tN_i32))
    n2 = RstSeqAtOff(n1, 0)
    n3 = RstApply(RstFn(scope, '=='), RstApply(RstFn(scope, 'typeOf'), n2), RstType('i32'))

    n2._t >> check >> typeIs >> i32

    slot locators
    RstSeqAt
    RstSeqAtOff
    RstTupAt
    RstMapAt
    RstStructAt
    RstScopeAt

    content accessors
    RstGet
    RstPut

    a = ...
    RstApply(RstFn(scope, 'add'), RstGet(a), RstGet(b)) // in "pass by ref", e.g. Java, a is a pointer to an object
    RstApply(RstFn(scope, 'add'), a, b)                 // R actually passes the ast node? the slot in this case

    pass by ref would mean



    RstDefFn
    RstDefBlock

    ptr_t **pp = malloc(2);
    *pp++;

    ptr_t *p = *pp;

    a =

    slot navigation
    matchbox navigation

    a slot can be a memory address or a register


    contained reference

    a = (1,2)           // a is a reference to a slot heap, stack, register - compiler could use r1 and r2
    b = a               // b is ???
    b.1 = 2             // b.1 is what



    i32 *p = 0;
    i32 a =


    # create deref

    [] in C is just pointer arithmatic

    so a pointer is an exponential and we don't need ptr in the compiler


    N**i32 & range

a = 1

# if we provide a at and atPut for a ptr then we can implement d ranges in bones :(
#
# range[T1]: {p1:O**[T1]&ptr, p2:O**[T1]&ptr}
# isEmpty: {[r:range] r.p2 < r.p1}
# isNotEmpty: {[r:range] r.p2 >= r.p1}
# front: {[r:range[T1]] r.p1[0]}
# popFront: {[r:range] {p1:r.p1+1, p2:r.p2} <:range>}
# back: {[r:range[T1]] r.p1[0]}
# popBack: {[r:range] {p1:r.p1, p2:r.p2-1} <:range>} // could add bounds checking as max(p1, p2)-1
#
# findAdjacent: {[r: T & range] -> range
#     r isEmpty ifTrue: [^ r]
#     s: r dropFront
#     [s isNotEmpty] whileTrue: [
#         (r front) == (s front) ifTrue: [^^ r]
#         r, s: r popFront, s popFront
#     ]
#     r
# }
#
# reverse: {[x:range] -> range
#     r: x  // force a copy
#     [r isNotEmpty] whileTrue: [
#         r: r swapFB
#         r: r popFront
#         r isEmpty ifTrue: [^]
#         r: r popBack
#     ]
#     r
# }
# swapFB: {[r:range]  t: r.p1[0];  r.p1[0]: r.p2[0];  r.p2[0]: t;  r}


# ForwardRange findAdjacent(ForwardRange r){
#    if (!r.empty()) {
#       auto s = r.save();
#       s.popFront();
#       for (; !s.empty();
#             r.popFront(), s.popFront()) {
#          if (r.front() == s.front()) break;
#       }
#    }
#    return r;
# }

# void reverse(DoubleEndedRange r) {
#    while (!r.empty()) {
#       swap(r.front(), r.back());
#       r.popFront();
#       if (r.empty()) break;
#       r.popBack();
#    }
# }







