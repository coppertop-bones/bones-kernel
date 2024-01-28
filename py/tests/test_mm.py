from coppertop.pipe import *
from dm.core.types import pylist, pytuple
from dm.testing import check, raises, equals, gt, different
from bones import jones
from bones.core.errors import NotYetImplemented
import dm.pp
from groot import PP

import sys, itertools


def test_mm():
    sys._k = jones.Kernel()
    tm = sys._k.tm
    mm = sys._k.mm

    mem = 1

    i32 = tm.nominal('i32', mem, 4)
    p = mm.alloc(i32)
    mm.inc(p)
    mm.count(p) == 1
    mm.dec(p)
    mm.count(p) == 0
    mm.inc(p)
    mm.inc(p)
    mm.inc(p)
    mm.count(p) == 3
    mm.dec(p)
    mm.count(p) == 3
    assert mm.btypeid(p) == i32

    return "test_mm passed"



def main():
    test_mm() >> PP



if __name__ == '__main__':
    sys._k = jones.Kernel()
    main()
    sys._k = None
    'passed' >> PP
