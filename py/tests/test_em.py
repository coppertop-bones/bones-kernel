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



def test_em(k):
    k = jones.Kernel() if k is Missing else k
    em = k.em

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



def main(k=Missing):
    test_em(k) >> PP



if __name__ == '__main__':
    sys._k = jones.Kernel()
    main(sys._k)
    sys._k = None
    'passed' >> PP

