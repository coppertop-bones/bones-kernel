from coppertop.pipe import *
from dm.core.types import pylist, pytuple
from dm.testing import check, raises, equals, gt, different
from bones import jones
from bones.core.errors import NotYetImplemented
from dm.pp import PP

import sys, itertools

@coppertop(style=binary)
def apply_(fn, arg):
    return lambda : fn(arg)

@coppertop(style=binary)
def apply_(fn, arg:pylist+pytuple):
    return lambda : fn(*arg)

def isType(a, b) -> bool:
    return a == b

def isNotType(a, b) -> bool:
    return a != b



def test_sm():
    sys._k = jones.Kernel()
    sm = sys._k.sm

    id1 = sm.symid("joe")
    id2 = sm.symid("fred")
    id3 = sm.symid("fred")
    id2 >> check >> equals >> id3
    sm.name(id2) >> check >> equals >> "fred"
    sm.symid("a") >> check >> gt >> id3
    sm.symid("b") >> check >> gt >> id3

    sm.name >> apply_ >> 0 >> check >> raises >> ValueError
    sm.name >> apply_ >> 100000 >> check >> raises >> ValueError

    return "test_sm passed"


def test_sm_sort_order():
    sys._k = jones.Kernel()
    sm = sys._k.sm

    id1 = sm.symid("joe")
    id2 = sm.symid("fred")
    sm.le(id2, id1) >> check >> equals >> True
    sm.le(id2, id2) >> check >> equals >> False
    sm.le(id1, id2) >> check >> equals >> False

    return "test_sm_sort_order passed"


def test_em():
    sys._k = jones.Kernel()
    em = sys._k.em

    e = em.enum('excl', 'mem')
    e.id >> check >> equals >> 1
    em.enum('excl', 'mem').id  >> check >> equals >> 1

    e = em.setEnumTo('excl', 'ccy', 2)
    e.id >> check >> equals >> 2
    em.enum('excl', 'ccy').id  >> check >> equals >> 2

    e = em.enum('fred', 'mem')
    e.id >> check >> equals >> 1
    em.enum('fred', 'mem').id >> check >> equals >> 1

    em.setEnumTo(['fred', 'mem', 1]).id >> check >> equals >> 1
    em.setEnumTo >> apply_ >> ('fred', 'mem', 2) >> check >> raises >> ValueError

    return "test_em passed"


def test_nominal():
    # OPEN: dangerous in Python - raise an error if the underlying tm has been trashed
    # tm = jones.Kernel().tm()
    # options:
    # 1) keep as is - customer beware
    # 2) weak refs?
    # 3a) explicit deallocation - leak memory by not trashing when k = None
    # 3b) kernel manager
    # this is why gc is good for scripting languages - bones cleans up the mess so you don't have to :)

    # OPEN: explicit independent life cycle, e.g.
    # jones.createKernel("fred")
    # tm = jones.getKernel("fred").tm
    # [jones.trashKernel(name) for name in jones.kernels()]


    sys._k = jones.Kernel()
    tm = sys._k.tm

    tm.exists('u32') >> check >> equals >> False
    tm.btype >> apply_ >> ['u32'] >> check >> raises >> TypeError
    t = tm.nominal(f'u32')
    tm.exists('u32') >> check >> equals >> True
    tm.btype('u32') >> check >> isType >> t
    # tm.btype('u32') >> check >> equals >> t
    tm.name(t) >> check >> equals >> 'u32'

    return "test_nominal passed"


def test_intersection():
    sys._k = jones.Kernel()
    tm = sys._k.tm

    mem = tm.exclusionCat('mem')
    ccy = tm.exclusionCat('ccy')

    tCcy = tm.exclusiveNominal('ccy', ccy)

    GBP = tm.intersection(tCcy, tm.nominal(f'_GBP'))
    GBP >> check >> equals >> tm.intersection(tm.nominal(f'_GBP'), tCcy)
    EUR = tm.intersection(tCcy, tm.nominal(f'_EUR'))

    # test exclusivity
    u32 = tm.exclusiveNominal('u32', mem)
    u64 = tm.exclusiveNominal('u64', mem)
    tm.intersection >> apply_ >> (u32, u64) >> check >> raises >> TypeError
    t = tm.intersection(u32, GBP)
    tm.intersection >> apply_ >> (t, u64) >> check >> raises >> TypeError

    tl = tm.intersectionTl(tm.intersection(GBP, u32))
    [e for e in tl] >> check >> isType >> [tCcy, tm.nominal(f'_GBP'), u32]

    # OPEN: intersections of unions

    return "test_intersection passed"


def test_name_as():
    sys._k = jones.Kernel()
    tm = sys._k.tm

    mem = tm.exclusionCat('mem')

    tCcy = tm.exclusiveNominal('f8', mem)
    GBP = tm.intersection(tCcy, tm.nominal(f'_GBP'))

    # test nameAs
    tm.name(GBP) >> check >> different >> 'GBP'
    t = tm.nameAs(GBP, 'GBP')
    GBP >> check >> isType >> t
    tm.name(GBP) >> check >> equals >> 'GBP'

    return "test_name_as passed"


def test_union():
    sys._k = jones.Kernel()
    tm = sys._k.tm

    t1 = tm.nominal(f'u32')
    t2 = tm.nominal(f'err')

    tm.union(t1, t2) >> check >> isType >> tm.union(t2, t1)
    tm.union(t1, t2, t1) >> check >> isType >> tm.union(t2, t1, t2)
    tm.union(tm.union(t1, t2), t1) >> check >> isType >> tm.union(t2, tm.union(t2, t1))

    tl = tm.unionTl(tm.union(tm.union(t1, t2), t1))
    tuple([e for e in tl]) >> check >> isType >> (t1, t2)

    return "test_union passed"


def test_tuple():
    sys._k = jones.Kernel()
    tm = sys._k.tm

    t1 = tm.nominal(f'u32')
    t2 = tm.nominal(f'err')

    tm.tuple(t1, t2) >> check >> isType >> tm.tuple(t1, t2)
    tm.tuple(t1, t2) >> check >> isNotType >> tm.tuple(t2, t1)

    return "test_tuple passed"


def test_struct():
    sys._k = jones.Kernel()
    tm = sys._k.tm

    t1 = tm.nominal(f'f64')
    t2 = tm.nominal(f'txt')

    tm.struct(('x', 'y'), (t1, t2)) >> check >> isType >> tm.struct(('x', 'y'), (t1, t2))
    tm.struct(('x', 'y'), (t1, t2)) >> check >> isNotType >> tm.struct(('y', 'x'), (t1, t2))

    return "test_struct passed"


def test_sequence():
    sys._k = jones.Kernel()
    tm = sys._k.tm

    t1 = tm.union(tm.nominal(f'u32'), tm.nominal(f'err'))
    tm.seq(t1) >> check >> isType >> tm.seq(t1)
    tm.seqT(tm.seq(t1)) >> check >> isType >> t1

    return "test_sequence passed"


def test_map():
    sys._k = jones.Kernel()
    tm = sys._k.tm

    t1 = tm.nominal(f'txt')
    t2 = tm.nominal(f'f64')
    tm.map(t1, t2) >> check >> isType >> tm.map(t1, t2)
    tm.mapTK(tm.map(t1, t2)) >> check >> isType >> t1
    tm.mapTV(tm.map(t1, t2)) >> check >> isType >> t2

    return "test_map passed"


def test_function():
    sys._k = jones.Kernel()
    tm = sys._k.tm

    t1 = tm.nominal(f'u32')
    t2 = tm.union(t1, tm.nominal(f'err'))
    tm.fn((t1, t1), t2) >> check >> isType >> tm.fn((t1, t1), t2)
    tm.tupleTl(tm.fnTArgs(tm.fn((t1, t1), t2)))[0] >> check >> isType >> t1
    tm.tupleTl(tm.fnTArgs(tm.fn((t1, t1), t2)))[1] >> check >> isType >> t1
    tm.fnTRet(tm.fn((t1, t1), t2)) >> check >> isType >> t2

    return "test_function passed"


def test_schemavar():
    sys._k = jones.Kernel()
    tm = sys._k.tm

    tm.exists('T1') >> check >> equals >> False
    tm.btype >> apply_ >> ['T1'] >> check >> raises >> TypeError
    t = tm.schemavar(f'T1')
    tm.exists('T1') >> check >> equals >> True
    tm.btype('T1') >> check >> isType >> t
    tm.name(t) >> check >> equals >> 'T1'

    return "test_schemavar passed"


def test_equals():
    sys._k = jones.Kernel()
    tm = sys._k.tm

    # BTypes need to be cached in the kernel and dropped on kernel shutdown

    t1 = tm.nominal(f'u32')
    assert t1 == tm.nominal(f'u32')     # have to use assert as check uses the type system (which we are testing)
    assert t1 is tm.nominal(f'u32')

    return "test_equals passed"


def test_minus():
    sys._k = jones.Kernel()
    tm = sys._k.tm

    t3 = tm.nominal(f'GBP')     # deliberately in reverse order
    t2 = tm.nominal(f'ccy')
    t1 = tm.nominal(f'f64')
    t4 = tm.nominal(f'u32')

    # intersections
    tm.minus(tm.intersection(t1, t2, t3), t2) >> check >> isType >> tm.intersection(t1, t3)
    tm.minus(tm.intersection(t1, t2, t3), tm.intersection(t1, t3)) >> check >> isType >> t2
    # OPEN: do the following one day? currently consider it hard to reason about so probably not
    # tm.minus(t1, tm.intersection(t1, t2)) >> check >> equals >> tm.intersection(t1, tm.exclusion(t2))
    tm.minus >> apply_ >> (t1, t1) >> check >> raises >> TypeError
    tm.minus >> apply_ >> (t1, t2) >> check >> raises >> TypeError
    tm.minus >> apply_ >> (tm.intersection(t1, t2, t3), t4) >> check >> raises >> TypeError
    tm.minus >> apply_ >> (tm.intersection(t1, t2, t3), tm.intersection(t1, t3, t4)) >> check >> raises >> TypeError

    # unions
    tm.minus(tm.union(t1, t2, t3), t2) >> check >> isType >> tm.union(t1, t3)
    tm.minus(tm.union(t1, t2, t3), tm.union(t1, t3)) >> check >> isType >> t2
    tm.minus >> apply_ >> (tm.union(t1, t2, t3), t4) >> check >> raises >> TypeError
    tm.minus >> apply_ >> (tm.union(t1, t2, t3), tm.union(t1, t3, t4)) >> check >> raises >> TypeError

    return "test_minus passed"


def test_hasT():
    sys._k = jones.Kernel()
    tm = sys._k.tm

    t1 = tm.nominal("u8")
    tm.hasT(t1) >> check >> equals >> False

    T1 = tm.schemavar(f'T1')
    tm.hasT(T1) >> check >> equals >> True

    tm.hasT(tm.intersection(t1, T1)) >> check >> equals >> True
    tm.hasT(tm.union(t1, T1)) >> check >> equals >> True
    tm.hasT(tm.tuple(t1, T1)) >> check >> equals >> True
    tm.hasT(tm.seq(T1)) >> check >> equals >> True
    tm.hasT(tm.map(t1, T1)) >> check >> equals >> True
    tm.hasT(tm.function((t1, t1), T1)) >> check >> equals >> True
    # tm.hasT(tm.struct(("x",), (T1,))) >> check >> equals >> True

    return "test_hasT passed"


def test_recursion():
    sys._k = jones.Kernel()
    tm = sys._k.tm

    u8 = tm.nominal('u8')
    null = tm.nominal('null')
    txt = tm.nominal('txt')
    f64 = tm.nominal('f64')

    # various linked lists

    # struct
    tr1 = tm.recursive()
    tnode1 = tm.struct_at(('i', 'next'), (u8, tm.union(tr1, null)), tr1)
    tr1 >> check >> isType >> tnode1

    # check we can't use tr1 again
    tm.tuple_at >> apply_ >> ((u8, tm.union(tr1, null)), tr1) >> check >> raises >> TypeError

    # tuple
    tr2 = tm.recursive()
    tnode2 = tm.tuple_at((u8, tm.union(tr2, null)), tr2)
    tr2 >> check >> isType >> tnode2

    # seq
    tr3 = tm.recursive()
    tnode3 = tm.seq_at((u8, tm.union(u8, tr3, null)), tr3)
    tr3 >> check >> isType >> tnode3

    # map
    tr4 = tm.recursive()
    tnode4 = tm.map_at(txt, (u8, tm.union(u8, tr4, null)), tr4)
    tr4 >> check >> isType >> tnode4

    # union
    tr5 = tm.recursive()
    tnode5 = tm.union_at((null, tm.tuple(u8, tr5)), tr5)
    tr5 >> check >> isType >> tnode5

    # intersection without need for dummy type
    tr6 = tm.recursive()
    GBP = tm.intersection_at((tr6, f64), tr6)
    tm.nameAs(GBP, 'GBP')

    # need isRecursive for printing?

    return "test_recursion passed"


def test_offsets():
    sys._k = jones.Kernel()
    tm = sys._k.tm

    mem = tm.exclusionCat('mem')
    f64 = tm.exclusiveNominal('f64', mem, 8)
    u32 = tm.exclusiveNominal('f64', mem, 4)

    s1 = tm.struct(('t', 'x', 'y'), (u32, f64, f64))
    tm.sz(s1) >> check >> equal >> 24
    tm.align(s1) >> check >> equal >> 8
    tm.offsets(s1) >> check >> equal >> (0, 8, 16)

    s2 = tm.struct(('x', 'y', 't'), (f64, f64, u32))
    tm.sz(s2) >> check >> equal >> 20
    tm.align(s2) >> check >> equal >> 8
    tm.offsets(s2) >> check >> equal >> (0, 8, 16)

    s3 = tm.struct(('x', 'y', 't'), (u32, u32, u32))
    tm.sz(s3) >> check >> equal >> 12
    tm.align(s3) >> check >> equal >> 4
    tm.offsets(s3) >> check >> equal >> (0, 4, 8)

    return "test_offsets passed"




# blocks in bones / RST must behave the same as C ones - so in bones parameters are new variables
# each(agg, fn) can just call the fn via C-ABI
# each(agg, block) can emit RST or use a block calling api? or the block is made to look like a function with pointers
# to vars


# TODO
#   check sizes
#   add python BTypeError (subclass of TypeError)
#   once all types can be created rework bones.lang.metatypes
#   create the null tuple for functions that take no arguments

def main():
    test_sm() >> PP
    # test_sm_sort_order()
    # test_em()
    test_nominal() >> PP
    test_intersection() >> PP
    test_name_as() >> PP
    test_union() >> PP
    test_tuple() >> PP
    test_struct() >> PP
    test_sequence() >> PP
    test_map() >> PP
    test_function() >> PP
    test_schemavar() >> PP
    test_equals() >> PP
    test_minus() >> PP
    test_hasT() >> PP
    test_recursion() >> PP
    test_offsets() >> PP



if __name__ == '__main__':
    sys._k = jones.Kernel()
    main()
    sys._k = None
    'passed' >> PP

