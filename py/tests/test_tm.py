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
    sm = sys._k.sm
    id1 = sm.sym("joe")
    id2 = sm.sym("fred")
    id3 = sm.sym("fred")
    assert id2 == id3
    assert sm.name(id2) == "fred"
    assert sm.sym("a") > id3
    assert sm.sym("b") > id3
    assert sm.sym("c") > id3
    assert sm.sym("d") > id3
    assert sm.sym("e") > id3
    assert sm.sym("f") > id3
    assert sm.sym("g") > id3
    assert sm.sym("h") > id3
    assert sm.sym("i") > id3
    assert sm.sym("j") > id3
    assert sm.sym("k") == 13

    try:
        sm.name(0)
        raise AssertionError("shouldn't get here")
    except ValueError:
        pass

    try:
        sm.name(14)
        raise AssertionError("shouldn't get here")
    except ValueError:
        pass


def test_sm_sort_order():
    sm = sys._k.sm
    id1 = sm.sym("joe")
    id2 = sm.sym("fred")
    assert sm.le(id2, id1) == True
    assert sm.le(id2, id2) == False
    assert sm.le(id1, id2) == False


def test_em():
    1/0


def test_intersection():
    tm = sys._k.tm

    assert not tm.exists('GBP')
    try:
        tm.btype('GBP')
        raise AssertionError("shouldn't get here")
    except TypeError:                   # OPEN: make this a BTypeError
        pass

    tCcy = tm.nominal('ccy')
    assert tm.exists('ccy')

    tag = tm.nominal(f'_GBP')
    assert tm.exists('_GBP')

    res = tm.setExplicit(tCcy)
    assert res

    GBP = tm.intersection(tCcy, tag)
    assert tm.name(GBP) is None
    res = tm.nameAs(GBP, 'GBP')
    assert res


def main():
    test_sm()  #;   print('test_sm passed')
    # test_sm_sort_order()
    # test_em()
    test_intersection()



if __name__ == '__main__':
    sys._k = jones.Kernel()
    main()
    sys._k = None

