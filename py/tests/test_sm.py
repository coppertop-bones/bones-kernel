import sys, itertools

from coppertop.pipe import *

from bones import jones
from bones.core.sentinels import Missing
from bones.core.errors import NotYetImplemented

from dm.core.types import pylist, pytuple
from dm.testing import check, raises, equals, gt, different
from dm.pp import PP


@coppertop(style=binary)
def apply_(fn, arg):
    return lambda : fn(arg)

@coppertop(style=binary)
def apply_(fn, args:pylist+pytuple):
    return lambda : fn(*args)

def isType(a, b) -> bool:
    return a == b

def isNotType(a, b) -> bool:
    return a != b



def test_sm(k):
    k = jones.Kernel() if k is Missing else k
    sm = k.sm

    id1 = sm.symid("joe")
    id2 = sm.symid("fred")
    id3 = sm.symid("fred")
    id2 >> check >> equals >> id3
    sm.name(id2) >> check >> equals >> "fred"
    sm.symid("a") >> check >> gt >> id3
    sm.symid("b") >> check >> gt >> id3

    sm.name >> apply_ >> 0 >> check >> raises >> ValueError
    sm.name >> apply_ >> 100000 >> check >> raises >> ValueError

    del sys.__dict__['_k']
    return "test_sm passed"


def test_sm_sort_order(k):
    k = jones.Kernel() if k is Missing else k
    sm = k.sm

    id1 = sm.symid("joe")
    id2 = sm.symid("fred")
    sm.le(id2, id1) >> check >> equals >> True
    sm.le(id2, id2) >> check >> equals >> False
    sm.le(id1, id2) >> check >> equals >> False

    return "test_sm_sort_order passed"



def main(k=Missing):
    test_sm(k) >> PP
    test_sm_sort_order(k)            # not needed for dispatch



if __name__ == '__main__':
    sys._k = jones.Kernel()
    main(sys._k)
    sys._k = None
    'passed' >> PP

