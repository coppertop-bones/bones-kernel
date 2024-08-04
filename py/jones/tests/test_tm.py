import sys, traceback
from bones import jones
from bones.jones import BTypeError


# TODO
#   once all types can be created rework bones.lang.metatypes
#   create the null tuple for functions that take no arguments


# OPEN:
#   - intersections of unions
#   - handle recursion in PP
#   - check every valid arg in constructor can handle recursion (fiddly)
#   - handle tm.minus(t1, tm.intersection(t1, t2)) == tm.intersection(t1, tm.exclusion(t2))? currently consider it
#     hard to reason about so probably not
#   - sizes



def test_atom():
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

    assert not tm.exists('sally')
    with assertRaises(jones.BTypeError):
        tm.fromName('sally')
    t = tm.atom('sally')
    assert tm.exists('sally')
    assert tm.fromName('sally') == t
    assert tm.name(t) == 'sally'

    assert t == tm.atom(f'sally')
    assert t is tm.atom(f'sally')

    return "test_atom passed"


def test_intersection():
    sys._k = jones.Kernel()
    tm = sys._k.tm

    mem = tm.atom('mem')
    ccy = tm.atom('ccy')

    _GBP = tm.intersection(ccy, tm.atom('_GBP'))
    assert _GBP == tm.intersection(tm.atom(f'_GBP'), ccy)

    assert tm.intersection(mem, _GBP) == tm.intersection(mem, tm.atom(f'_GBP'), mem, ccy, _GBP)

    for t in tm.intersectionTl(tm.intersection(_GBP, mem)):
        assert t in [ccy, mem, tm.atom(f'_GBP')]

    GBP = tm.options(name='GBP')

    return "test_intersection passed"


def test_orthogonal_spaces():
    sys._k = jones.Kernel()
    tm = sys._k.tm

    ccyfx = tm.atom('ccyfx')

    opts = tm.options()
    assert opts is not None

    opts = tm.options(orthspc=ccyfx)
    assert tm.orthspc(opts) == ccyfx

    ccy = tm.atom('ccy', self=tm.options(orthspc=ccyfx))
    fx = tm.atom('fx', self=tm.options(orthspc=ccyfx))

    assert tm.orthspc(ccy) == ccyfx
    assert tm.rootOrthspc(fx) == ccyfx

    with assertRaises(BTypeError):
        t = tm.intersection(ccy, fx)

    mem = tm.atom('mem')         # mem is a builtin type
    py = tm.atom('py')           # to indicate Python types
    null = tm.atom('null')

    sz = tm.atom('sz')
    m64 = tm.atom('m64', self=tm.options(orthspc=sz))

    mut = tm.options()
    consty = tm.atom('consty', self=tm.options(implicitly=mut))
    mut = tm.atom('mut', self=mut, orthspc=consty)

    f64 = tm.options(name='f64')
    f64 = tm.intersection(f64, m64, self=f64, orthspc=mem)

    GBP = tm.options(name='GBP')
    GBP = tm.intersection(GBP, f64, ccy, self=GBP, orthspc=ccy)         # intersection of GBP, f64 and ccy in the ccy space

    USD = tm.options(name='USD')
    USD = tm.intersection(USD, f64, ccy, self=USD, orthspc=ccy)

    assert tm.isRecursive(GBP)
    with assertRaises(BTypeError):
        tm.intersection(GBP, USD)


    # recursive types do not need to be named
    f64Tree = tm.options()
    tLhs = tm.union(f64Tree, f64, null)
    tRhs = tm.union(f64Tree, f64, null)
    f64Tree = tm.struct(('lhs', 'rhs'), (tLhs, tRhs), self=f64Tree)

    assert tm.isRecursive(f64Tree) and not tm.isRecursive(tLhs) and not tm.isRecursive(tRhs)


    pylist = tm.intersection(tm.options(name='pylist'), orthspc=py)
    pytup = tm.intersection(tm.options(name='pytup'), orthspc=py)

    # pyToBones[list] = pylist
    # pyToBones[tuple] = pytup

    lit = tm.atom('lit')
    littxt = tm.intersection(tm.options(name='littxt'), orthspc=lit)


    return "test_orthogonal passed"


def test_name_as():
    sys._k = jones.Kernel()
    tm = sys._k.tm

    mem = tm.atom('mem')

    tCcy = tm.atom('f8', self=tm.options(orthspc=mem))
    GBP = tm.intersection(tCcy, tm.atom(f'_GBP'))

    # test nameAs
    assert tm.name(GBP) != 'GBP'
    t = tm.nameAs(GBP, 'GBP')
    assert GBP == t
    assert tm.name(GBP) == 'GBP'

    return "test_name_as passed"


def test_union():
    sys._k = jones.Kernel()
    tm = sys._k.tm

    t1 = tm.atom(f'u32')
    t2 = tm.atom(f'err')

    assert tm.union(t1, t2) == tm.union(t2, t1)
    assert tm.union(t1, t2, t1) == tm.union(t2, t1, t2)
    assert tm.union(tm.union(t1, t2), t1) == tm.union(t2, tm.union(t2, t1))

    assert tm.unionTl(tm.union(tm.union(t1, t2), t1)) == (t1, t2)

    return "test_union passed"


def test_tuple():
    sys._k = jones.Kernel()
    tm = sys._k.tm

    t1 = tm.atom(f'u32')
    t2 = tm.atom(f'err')

    assert tm.tuple(t1, t2) == tm.tuple(t1, t2)
    assert tm.tuple(t1, t2) != tm.tuple(t2, t1)

    return "test_tuple passed"


def test_struct():
    sys._k = jones.Kernel()
    tm = sys._k.tm

    t1 = tm.atom(f'f64')
    t2 = tm.atom(f'txt')

    assert tm.struct(('x', 'y'), (t1, t2)) == tm.struct(('x', 'y'), (t1, t2))
    assert tm.struct(('x', 'y'), (t1, t2)) != tm.struct(('y', 'x'), (t1, t2))

    return "test_struct passed"


def test_sequence():
    sys._k = jones.Kernel()
    tm = sys._k.tm

    t1 = tm.union(tm.atom(f'u32'), tm.atom(f'err'))
    assert tm.seq(t1) == tm.seq(t1)
    assert tm.seqT(tm.seq(t1)) == t1

    return "test_sequence passed"


def test_map():
    sys._k = jones.Kernel()
    tm = sys._k.tm

    t1 = tm.atom(f'txt')
    t2 = tm.atom(f'f64')
    assert tm.map(t1, t2) == tm.map(t1, t2)
    assert tm.mapTK(tm.map(t1, t2)) == t1
    assert tm.mapTV(tm.map(t1, t2)) == t2

    return "test_map passed"


def test_function():
    sys._k = jones.Kernel()
    tm = sys._k.tm

    t1 = tm.atom(f'u32')
    t2 = tm.union(t1, tm.atom(f'err'))
    assert tm.fn((t1, t1), t2) == tm.fn((t1, t1), t2)
    assert tm.tupleTl(tm.fnTArgs(tm.fn((t1, t1), t2)))[0] == t1
    assert tm.tupleTl(tm.fnTArgs(tm.fn((t1, t1), t2)))[1] == t1
    assert tm.fnTRet(tm.fn((t1, t1), t2)) == t2

    return "test_function passed"


def test_schemavar():
    sys._k = jones.Kernel()
    tm = sys._k.tm

    assert tm.exists('T1') == False
    with assertRaises(BTypeError):
        tm.fromName('T1')
    t = tm.schemavar(f'T1')
    assert tm.exists('T1') == True
    assert tm.fromName('T1') == t
    assert tm.name(t) == 'T1'

    return "test_schemavar passed"


def test_minus():
    sys._k = jones.Kernel()
    tm = sys._k.tm

    t3 = tm.atom(f'GBP')     # deliberately in reverse order
    t2 = tm.atom(f'ccy')
    t1 = tm.atom(f'f64')
    t4 = tm.atom(f'u32')

    # intersections
    assert tm.minus(tm.intersection(t1, t2, t3), t2) == tm.intersection(t1, t3)
    assert tm.minus(tm.intersection(t1, t2, t3), tm.intersection(t1, t3)) == t2
    with assertRaises(BTypeError):
        tm.minus(t1, t1)
    with assertRaises(BTypeError):
        tm.minus(t1, t2)
    with assertRaises(BTypeError):
        tm.minus(tm.intersection(t1, t2, t3), t4)
    with assertRaises(BTypeError):
        tm.minus(tm.intersection(t1, t2, t3), tm.intersection(t1, t3, t4))

    # unions
    assert tm.minus(tm.union(t1, t2, t3), t2) == tm.union(t1, t3)
    assert tm.minus(tm.union(t1, t2, t3), tm.union(t1, t3)) == t2
    with assertRaises(BTypeError):
        tm.minus(tm.union(t1, t2, t3), t4)
    with assertRaises(BTypeError):
        tm.minus(tm.union(t1, t2, t3), tm.union(t1, t3, t4))

    return "test_minus passed"


def test_hasT():
    sys._k = jones.Kernel()
    tm = sys._k.tm

    t1 = tm.atom("u8")
    assert tm.hasT(t1) == False

    T1 = tm.schemavar(f'T1')
    assert tm.hasT(T1) == True

    assert tm.hasT(tm.intersection(t1, T1)) == True
    assert tm.hasT(tm.union(t1, T1)) == True
    assert tm.hasT(tm.tuple(t1, T1)) == True
    assert tm.hasT(tm.seq(T1)) == True
    assert tm.hasT(tm.map(t1, T1)) == True
    assert tm.hasT(tm.fn((t1, t1), T1)) == True
    assert tm.hasT(tm.struct(("x",), (T1,))) == True

    return "test_hasT passed"


def test_recursion():
    sys._k = jones.Kernel()
    tm = sys._k.tm

    u8 = tm.atom('u8')
    null = tm.atom('null')
    txt = tm.atom('txt')
    f64 = tm.atom('f64')

    # struct - linked list of u8
    tr1 = tm.options()
    tnode1 = tm.struct(('i', 'next'), (u8, tm.union(tr1, null)), self=tr1)
    assert tr1 == tnode1

    # check we can't use tr1 again
    with assertRaises(BTypeError):
        tm.tuple(u8, tm.union(tr1, null), self=tr1)

    # tuple - linked list of u8
    tr2 = tm.options()
    tnode2 = tm.tuple(u8, tm.union(tr2, null), self=tr2)
    assert tr2 == tnode2

    # seq - linked list of u8
    tr3 = tm.options()
    tnode3 = tm.seq(tm.union(u8, tr3, null), self=tr3)
    assert tr3 == tnode3

    # map - linked list of u8 - txt {"this", "next"} -> u8 + tr4 + null
    tr4 = tm.options()
    tnode4 = tm.map(txt, tm.union(u8, tr4, null), self=tr4)
    assert tr4 == tnode4

    # union
    tr5 = tm.options()
    tnode5 = tm.union(null, tm.tuple(u8, tr5), self=tr5)
    assert tr5 == tnode5

    # intersection
    tr6 = tm.options()
    GBP = tm.intersection(tr6, f64, self=tr6)
    tm.nameAs(GBP, 'GBP')

    return "test_recursion passed"


def test_offsets():
    sys._k = jones.Kernel()
    tm = sys._k.tm

    mem = tm.exclusionCat('mem')
    f64 = tm.exclusiveNominal('f64', mem, 8)
    u32 = tm.exclusiveNominal('f64', mem, 4)

    s1 = tm.struct(('t', 'x', 'y'), (u32, f64, f64))
    assert tm.sz(s1) == 24
    assert tm.align(s1) == 8
    assert tm.offsets(s1) == (0, 8, 16)

    s2 = tm.struct(('x', 'y', 't'), (f64, f64, u32))
    assert tm.sz(s2) == 20
    assert tm.align(s2) == 8
    assert tm.offsets(s2) == (0, 8, 16)

    s3 = tm.struct(('x', 'y', 't'), (u32, u32, u32))
    assert tm.sz(s3) == 12
    assert tm.align(s3) == 4
    assert tm.offsets(s3) == (0, 4, 8)

    return "test_offsets passed"



def main():
    # test_em()                       # not needed for dispatch
    print(test_atom())
    print(test_intersection())
    print(test_name_as())
    print(test_union())
    print(test_tuple())
    print(test_struct())
    print(test_sequence())
    print(test_map())
    print(test_function())
    print(test_schemavar())

    print(test_orthogonal_spaces())

    print(test_minus())
    print(test_hasT())
    print(test_recursion())
    # test_offsets())            # needed for memory layout but not for dispatch



class assertRaises:

    def __init__(self, expectedExceptionType):
        self.expectedExceptionType = expectedExceptionType
        self.exceptionType = None
        self.exceptionValue = None
        self.tb = None

    def __enter__(self):
        return self

    def __exit__(self, exceptionType, exceptionValue, tb):
        self.exceptionType = exceptionType
        self.exceptionValue = exceptionValue
        self.tb = tb
        if exceptionType is None:
            # no exception was raised
            raise AssertionError("No exception raised, %s expected." % self.expectedExceptionType)        # no error was raised
        elif not issubclass(exceptionType, self.expectedExceptionType):
            # the wrong exception was raised
            # print the tb to make it easier to figure why the test is failing
            traceback.print_tb(tb)
            raise AssertionError("%s raised. %s expected." % (exceptionType, self.expectedExceptionType))
        else:
            # the correct error was raised
            return True




if __name__ == '__main__':
    sys._k = jones.Kernel()
    main()
    sys._k = None
    'passed'

