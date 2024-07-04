import time

import numpy as np
from bones import qu

from coppertop.pipe import *
from dm.core.types import pylist, pytuple
from dm.testing import check, raises, equals, gt, different
from bones import jones
from bones.core.errors import NotYetImplemented
from dm.pp import PP

import itertools

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
    callK = 6#0.063
    putK = 4#0.04
    sigma = 0.20
    for t in [0.25, 0.5, 1, 2, 4]:
        calls.append(qu.b76_call(t*1.0, callK*1.0, f*1.0, sigma, r))
        puts.append(qu.b76_put(t*1.0, putK*1.0, f*1.0, sigma, r))
    print(calls)
    print(puts)
    return "test_black passed"



def test_cn_Hart():
    print([qu.cn_Hart(x * 1.0) for x in range(-3, 4)])
    return "test_cn_Hart passed"



def test_invcn_Acklam():
    print(qu.invcn_Acklam(0.0))
    print(qu.invcn_Acklam(0.00001))
    print(qu.invcn_Acklam(0.99999))
    print(qu.invcn_Acklam(1.0))



def test_mersenne():
    n = 1_000_000
    w = qu.new_mersennes_f64(n)
    assert w.shape == (n,)
    assert np.min(w) > 0.0
    assert np.max(w) < 1.0
    w = qu.new_mersennes_f64(2, 2)
    assert w.shape == (2, 2)

    W = qu.new_mersennes_norm(16)
    print(W)

    means, stds = [], []
    for i in range(1000):
        w = qu.new_mersennes_norm(10_000)
        means.append(np.mean(w))
        stds.append(np.std(w))

    print(np.average(means), np.std(means), np.average(means) - np.std(means), np.average(stds), np.std(stds), (1 - np.average(stds)) / np.std(stds))

    return "test_mersenne passed"



def test_fill_matrix():
    N = 5
    np.set_printoptions(precision=3, suppress=True)
    W = np.zeros((N, 2), order='F')
    W = qu.fill_mersennes_norm(W, 0, N-1, 0, 0)
    print(W)
    W[0,1] = 3.0
    W = qu.fill_matrix(W, "norm", j=1, jW=0, dt=1.0/365.0, sigma=0.15)
    print(W)
    W = qu.fill_matrix(W, "log", j=1, jW=0, dt=1.0/365.0, sigma=0.15)
    print(W)
    W[0,0] = 3.0
    W = qu.fill_matrix(W, "log", j=0, W=0, dt=1.0/365.0, sigma=0.15)
    print(W)
    return "test_fill_matrix passed"



def test_lognormal_martingale():
    N, M, numRuns = 365, 10_000, 100
    f0 = 0.05
    dt = 1.0 / N
    sigma = 0.20
    muNoIto = sigma * sigma * dt * 0.5   # to kill the ito drift added in fill_matrix("log")

    meansNoIto, meansIto, meansDiff, stdsNoIto, stdIto, stdDiff = [], [], [], [], [], []
    ran_times, proc_times = [], []
    for iRun in range(numRuns):
        t1 = time.perf_counter_ns()
        runsNoIto = qu.new_mersennes_norm(N, M)
        runsIto = qu.new_mersennes_norm(N, M)
        runsNoIto[0,:] = f0
        runsIto[0,:] = f0
        t2 = time.perf_counter_ns()
        # OPEN: make j=0 by default, i1=1 by default, can override using j= or j1=0 and j2=M-1 but not both
        # OPEN: seed=
        runsIto = qu.fill_matrix(runsIto, "log", j1=0, j2=M-1, dt=dt, sigma=sigma, mu=0.0)
        runsNoIto = qu.fill_matrix(runsNoIto, "log", j1=0, j2=M-1, dt=dt, sigma=sigma, mu=muNoIto)
        t3 = time.perf_counter_ns()
        ran_times.append((t2 - t1) / 2.0)
        proc_times.append((t3 - t2) / 2.0 / M)
        meansNoIto.append(np.mean(runsNoIto[N-1,:] - f0))
        meansIto.append(np.mean(runsIto[N-1,:] - f0))
        meansDiff.append(np.mean(runsNoIto[N-1,:] - runsIto[N-1,:]))
        stdsNoIto.append(np.std(runsNoIto[N-1,:] - f0))
        stdIto.append(np.std(runsIto[N-1,:] - f0))
        stdDiff.append(np.std(runsNoIto[N-1,:] - runsIto[N-1,:]))

    print(f"ito     - {_PPMeanStd(meansIto, stdIto)}")
    print(f"no ito  - {_PPMeanStd(meansNoIto, stdsNoIto)}")
    print(f"diffs   - {_PPMeanStd(meansDiff, stdDiff)}")
    print(f"rans    - {_PPTimesMs(ran_times, numRuns * 2)}")
    print(f"process - {_PPTimesUs(proc_times, numRuns * 2 * M)}")

    return "test_lognormal_martingale passed"



def _PPMeanStd(means, stds):
    dp = 6
    dpSE = 3
    return f"mean: {np.average(means):,.{dp}f} ± {np.std(means):,.{dp}f} (i.e. {np.average(means) / np.std(means):,.{dpSE}f} SE), std: {np.average(stds):,.{dp}f} (SE: {np.std(stds):,.{dp}f})"

def _PPTimesMs(times, N):
    dp = 1
    return f"{np.average(times) / 1_000_000:,.{dp}f}mS ± {np.std(times) / 1_000_000:,.{dp}f}mS (total: {np.average(times) * N / 1_000_000:,.{dp}f}mS)"

def _PPTimesUs(times, N):
    dp = 1
    return f"{np.average(times) / 1_000:,.{dp}f}µS ± {np.std(times) / 1_000:,.{dp}f}µS (total: {np.average(times) * N / 1_000_000:,.{dp}f}mS)"


def main():
    test_cn_Hart() >> PP
    test_invcn_Acklam() >> PP
    test_black() >> PP
    test_mersenne() >> PP
    test_fill_matrix() >> PP
    test_lognormal_martingale() >> PP


if __name__ == '__main__':
    main()
    'passed' >> PP

