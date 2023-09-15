from bones import jones
import sys


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
    sm = sys,_k.sm
    id1 = sm.id("hello")
    id2 = sm.id("hello")
    rp1 = sm.rp("hello")
    rp2 = sm.rp("hello")
    assert id1 == id2
    assert rp1 == rp2
    assert sm.id2str(id1) == sm.rp2str(rp1)

def test_em():
    1/0

def test_intersection():
    k = sys._k
    assert k.tm.btype('GBP') is None
    assert k.tm.btype('_GBP') is None
    assert k.tm.btype('ccy') is None
    assert k.tm.exists('ccy') is False

    tCcy = k.tm.atom('ccy')
    res = k.tm.setExplicit(tCcy)
    assert res

    name = "GBP"
    tag = k.tm.atom(f'_{name}')         # checks that there is no name conflict

    GBP = k.tm.intersection(tCcy, tag)
    assert k.tm.name(GBP) is None
    res = k.tm.nameAs(GBP, name)
    assert res


def main():
    test_sm()
    test_em()
    test_intersection()



if __name__ == '__main__':
    sys._k = jones.Kernel()
    main()
    sys._k = None
