from coppertop.pipe import *
from dm.core.types import pylist, pytuple
from dm.testing import check, raises, equals, gt, different
from bones import jones
import dm.pp
from groot import PP

import sys

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
    1 >> PP
    sys._k = jones.Kernel()
    2 >> PP
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

    mem = 1
    ccy = 2

    tCcy = tm.exclusiveNominal('ccy', ccy)

    GBP = tm.intersection(tCcy, tm.nominal(f'_GBP'))
    # GBP >> check >> equals >> tm.intersection(tCcy, tm.nominal(f'_GBP'))
    EUR = tm.intersection(tCcy, tm.nominal(f'_EUR'))

    # test nameAs
    tm.name(GBP) >> check >> different >> 'GBP'
    t = tm.nameAs(GBP, 'GBP')
    # GBP >> check >> equals >> t
    tm.name(GBP) >> check >> equals >> 'GBP'

    # test exclusivity
    u32 = tm.exclusiveNominal('u32', mem)
    u64 = tm.exclusiveNominal('u64', mem)
    tm.intersection >> apply_ >> (u32, u64) >> check >> raises >> TypeError

    t = tm.intersection(GBP, u32)
    tm.intersection >> apply_ >> (t, u64) >> check >> raises >> TypeError
    # tm.intersection >> apply_ >> (t, EUR) >> check >> raises >> TypeError

    return "test_intersection passed"


def main():
    test_sm() >> PP
    # test_sm_sort_order()
    # test_em()
    test_nominal() >> PP
    test_intersection() >> PP



if __name__ == '__main__':
    sys._k = jones.Kernel()
    main()
    'passed' >> PP
    sys._k = None

