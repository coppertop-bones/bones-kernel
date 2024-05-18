import sys
from bones import qu

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


def test_black():
    r = 0.0
    calls = []
    puts = []
    f = 5 #0.05
    callK = 6#0.06
    putK = 4#0.04
    sigma = 0.20
    for t in [0.25, 0.5, 1, 2, 4]:
        calls.append(qu.b76_call(t*1.0, callK*1.0, f*1.0, sigma, r))
        puts.append(qu.b76_put(t*1.0, putK*1.0, f*1.0, sigma, r))
    print(calls)
    print(puts)
    return "test_sm passed"



def test_cn_Hart():
    print([qu.cn_Hart(x * 1.0) for x in range(-3, 4)])
    return "test_sm passed"


def main():
    test_cn_Hart() >> PP
    test_black() >> PP


if __name__ == '__main__':
    main()
    'passed' >> PP

