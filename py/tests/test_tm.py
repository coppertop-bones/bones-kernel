from coppertop.pipe import *
from dm.core.types import pylist
from dm.testing import check, raises, equals, gt, different
from bones import jones
import dm.pp
from groot import PP

import sys

@coppertop(style=binary)
def apply_(fn, arg):
    return lambda : fn(arg)

@coppertop(style=binary)
def apply_(fn, arg:pylist):
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


def test_sm_sort_order():
    sm = sys._k.sm
    id1 = sm.symid("joe")
    id2 = sm.symid("fred")
    sm.le(id2, id1) >> check >> equals >> True
    sm.le(id2, id2) >> check >> equals >> False
    sm.le(id1, id2) >> check >> equals >> False


def test_em():
    1/0


def test_intersection():
    tm = sys._k.tm

    tm.exists('GBP') >> check >> equals >> False
    tm.btype >> apply_ >> ['GBP'] >> check >> raises >> TypeError  # OPEN: make this a BTypeError

    tCcy = tm.exclusiveNominal('ccy', 2)
    tm.exists('ccy') >> check >> equals >> True

    tag = tm.nominal(f'_GBP')
    tm.exists('_GBP') >> check >> equals >> True

    GBP = tm.intersection(tCcy, tag)
    GBP.id >> check >> equals >> tm.intersection(tCcy, tag).id
    tm.name(GBP) >> check >> equals >> None

    t = tm.nameAs(GBP, 'GBP')
    GBP.id >> PP >> check >> equals >> t.id
    tm.name(GBP) >> check >> equals >> 'GBP'


def main():
    test_sm()  #;   print('test_sm passed')
    # test_sm_sort_order()
    # test_em()
    test_intersection()



if __name__ == '__main__':
    sys._k = jones.Kernel()
    main()
    'passed' >> PP
    sys._k = None

