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


def test_intersection():
    k = sys.k
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
    test_intersection()



if __name__ == '__main__':
    sys.k = jones.Kernel()
    main()
    sys.k = None
