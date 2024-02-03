from coppertop.pipe import *
from dm.core.types import pylist, pytuple
from dm.testing import check, raises, equals, gt, different
from bones import jones
from bones.core.errors import NotYetImplemented
import dm.pp
from groot import PP

import sys, itertools

@coppertop(style=binary)
def apply_(fn, arg):
    return lambda : fn(arg)

@coppertop(style=binary)
def apply_(fn, arg:pylist+pytuple):
    return lambda : fn(*arg)


class tvfloat(float):
    def __new__(cls, t, v, *args, **kwargs):
        instance = super(cls, cls).__new__(cls, v)
        instance._t_ = t
        return instance
    @property
    def _v(self):
        return super().__new__(float, self)
    @property
    def _t(self):
        return self._t_
    def __repr__(self):
        return f'{self._t}{super().__repr__()}'
    def _asT(self, t):
        self._t_ = t
        return self



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
    tm.btype('u32').id >> check >> equals >> t.id
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
    GBP.id >> check >> equals >> tm.intersection(tm.nominal(f'_GBP'), tCcy).id
    EUR = tm.intersection(tCcy, tm.nominal(f'_EUR'))

    # test exclusivity
    u32 = tm.exclusiveNominal('u32', mem)
    u64 = tm.exclusiveNominal('u64', mem)
    tm.intersection >> apply_ >> (u32, u64) >> check >> raises >> TypeError
    t = tm.intersection(u32, GBP)
    tm.intersection >> apply_ >> (t, u64) >> check >> raises >> TypeError

    tl = tm.intersectionTl(tm.intersection(GBP, u32))
    [e.id for e in tl] >> check >> equals >> [tCcy.id, tm.nominal(f'_GBP').id, u32.id]

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
    GBP.id >> check >> equals >> t.id
    tm.name(GBP) >> check >> equals >> 'GBP'

    return "test_name_as passed"


def test_union():
    sys._k = jones.Kernel()
    tm = sys._k.tm

    t1 = tm.nominal(f'u32')
    t2 = tm.nominal(f'err')

    tm.union(t1, t2).id >> check >> equals >> tm.union(t2, t1).id
    tm.union(t1, t2, t1).id >> check >> equals >> tm.union(t2, t1, t2).id
    tm.union(tm.union(t1, t2), t1).id >> check >> equals >> tm.union(t2, tm.union(t2, t1)).id

    tl = tm.unionTl(tm.union(tm.union(t1, t2), t1))
    tuple([e.id for e in tl]) >> check >> equals >> (t1.id, t2.id)

    return "test_union passed"


def test_tuple():
    sys._k = jones.Kernel()
    tm = sys._k.tm

    t1 = tm.nominal(f'u32')
    t2 = tm.nominal(f'err')

    tm.tuple(t1, t2).id >> check >> equals >> tm.tuple(t1, t2).id
    tm.tuple(t1, t2).id >> check >> different >> tm.tuple(t2, t1).id

    return "test_tuple passed"


def test_struct():
    sys._k = jones.Kernel()
    tm = sys._k.tm

    t1 = tm.nominal(f'f64')

    tm.struct(('x', 'y'), (t1, t2)).id >> check >> equals >> tm.struct(('x', 'y'), (t1, t2)).id
    tm.struct(('x', 'y'), (t1, t2)).id >> check >> different >> tm.struct(('y', 'x'), (t1, t2)).id

    return "test_struct passed"


def test_sequence():
    sys._k = jones.Kernel()
    tm = sys._k.tm

    t1 = tm.union(tm.nominal(f'u32'), tm.nominal(f'err'))
    tm.seq(t1).id >> check >> equals >> tm.seq(t1).id
    tm.seqT(tm.seq(t1)).id >> check >> equals >> t1.id

    return "test_sequence passed"


def test_map():
    sys._k = jones.Kernel()
    tm = sys._k.tm

    t1 = tm.nominal(f'txt')
    t2 = tm.nominal(f'f64')
    tm.map(t1, t2).id >> check >> equals >> tm.map(t1, t2).id
    tm.mapTK(tm.map(t1, t2)).id >> check >> equals >> t1.id
    tm.mapTV(tm.map(t1, t2)).id >> check >> equals >> t2.id

    return "test_map passed"


def test_function():
    sys._k = jones.Kernel()
    tm = sys._k.tm

    t1 = tm.nominal(f'u32')
    t2 = tm.union(t1, tm.nominal(f'err'))
    tm.fn((t1, t1), t2).id >> check >> equals >> tm.fn((t1, t1), t2).id
    tm.tupleTl(tm.fnTArgs(tm.fn((t1, t1), t2)))[0].id >> check >> equals >> t1.id
    tm.tupleTl(tm.fnTArgs(tm.fn((t1, t1), t2)))[1].id >> check >> equals >> t1.id
    tm.fnTRet(tm.fn((t1, t1), t2)).id >> check >> equals >> t2.id

    return "test_function passed"


def test_schemavar():
    sys._k = jones.Kernel()
    tm = sys._k.tm

    tm.exists('T1') >> check >> equals >> False
    tm.btype >> apply_ >> ['T1'] >> check >> raises >> TypeError
    t = tm.schemavar(f'T1')
    tm.exists('T1') >> check >> equals >> True
    tm.btype('T1').id >> check >> equals >> t.id
    tm.name(t) >> check >> equals >> 'T1'

    return "test_schemavar passed"


def test_minus():
    sys._k = jones.Kernel()
    tm = sys._k.tm

    t3 = tm.nominal(f'GBP')     # deliberately in reverse order
    t2 = tm.nominal(f'ccy')
    t1 = tm.nominal(f'f64')

    # intersections
    tm.minus(tm.intersection(t1, t2, t3), t2).id >> check >> equals >> tm.intersection(t1, t3).id
    tm.minus(tm.intersection(t1, t2, t3), tm.intersection(t1, t3)).id >> check >> equals >> t2.id
    # tm.minus(t1, tm.intersection(t1, t2)).id >> check >> equals >> tm.minus(t1, tm.intersection(t1, t2)).id   # OPEN: do one day? currently consider it hard to reason about so probably not
    tm.minus >> apply_ >> (t1, t1) >> check >> raises >> TypeError
    tm.minus >> apply_ >> (t1, t2) >> check >> raises >> TypeError
    tm.minus >> apply_ >> (tm.intersection(t1, t2, t3), t4) >> check >> raises >> TypeError
    tm.minus >> apply_ >> (tm.intersection(t1, t2, t3), tm.intersection(t1, t3, t4)) >> check >> raises >> TypeError

    # unions
    tm.minus(tm.union(t1, t2, t3), t2).id >> check >> equals >> tm.union(t1, t3).id
    tm.minus(tm.union(t1, t2, t3), tm.union(t1, t3)).id >> check >> equals >> t2.id
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
    tr1.id >> check >> equals >> tnode1.id

    # check we can't use tr1 again
    tm.tuple_at >> apply_ >> ((u8, tm.union(tr1, null)), tr1) >> check >> raises >> TypeError

    # tuple
    tr2 = tm.recursive()
    tnode2 = tm.tuple_at((u8, tm.union(tr2, null)), tr2)
    tr2.id >> check >> equals >> tnode2.id

    # seq
    tr3 = tm.recursive()
    tnode3 = tm.seq_at((u8, tm.union(u8, tr3, null)), tr3)
    tr3.id >> check >> equals >> tnode3.id

    # map
    tr4 = tm.recursive()
    tnode4 = tm.map_at(txt, (u8, tm.union(u8, tr4, null)), tr4)

    # union
    tr5 = tm.recursive()
    tnode5 = tm.union_at((null, tm.tuple(u8, tr5)), tr5)

    # intersection without need for dummy type
    tr6 = tm.recursive()
    GBP = tm.intersection_at((tr6, f64), tr6)
    tm.nameAs(GBP, 'GBP')

    # need isRecursive for printing?

    return "test_recursion passed"



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
    # test_struct() >> PP
    test_sequence() >> PP
    test_map() >> PP
    test_function() >> PP
    test_schemavar() >> PP
    test_minus() >> PP
    test_hasT() >> PP
    test_recursion() >> PP



if __name__ == '__main__':
    sys._k = jones.Kernel()
    main()
    sys._k = None
    'passed' >> PP

