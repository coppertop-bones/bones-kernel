import math, time, itertools, numpy as np

from bones import qu

from coppertop.pipe import *
from dm.core.types import pylist, pytuple
from dm.testing import check, raises, equals, gt, different
from bones import jones
from bones.core.errors import NotYetImplemented
from dm.pp import PP


@coppertop(style=binary)
def apply_(fn, arg):
    return lambda : fn(arg)

@coppertop(style=binary)
def apply_(fn, arg:pylist+pytuple):
    return lambda : fn(*arg)


# PHILOSPHY OF QUANT TESTING
#   - important write test first (or really early on in worst case) to figure api
#   - visual output is always necessary but noteable absolutely fine to start with
#   - out test checks does the code run, the visual output allows the quant to validate the results
#   - add useful checks for assert the results are in error
#   - over time we want to compare two or more ways of getting the same result, e.g. b76 closed form and mc
#   - make it super easy to run subsets of tdd_bones


# QU GOALS
#   - verify b76 formulas (both pricing and hedging) via MC
#       - delta requires bumps to underlier price, gamma requires more bumps, vega bumps to process, etc
#   - determine std error for delta hedging to defend price
#   - show diff between right vol and wrong vol is the diff in price
#   - show diff between using normal delta to hedge lognormal process and vice versa
#   - develop rules of thumb for how big an MC is needed for 99% confidence
#   - similar for std SABR etc


# TASKS
#   - discrete lognormal df = f.dt + ito drift + sigma dW
#   - determine how many discrete steps are needed to approximate a log-space step
#   - various SABR etc
#   - and ran0, ran1, box-muller etc
#   - compare b76 pricing with other cn
#   - how to select best cn / invcn implementations
#   - brent root finder - implied vol


# OPEN:
# - auto convert PyLong to PyDouble where sensible
# - seed=


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
    return "test_invcn_Acklam passed"


def test_cn_h():
    print([qu.cn_h(x * 1.0) for x in range(-3, 4)])
    return "test_cn_h passed"


def test_invcn_h():
    print(qu.invcn_h(0.0))
    print(qu.invcn_h(0.00001))
    print(qu.invcn_h(0.99999))
    print(qu.invcn_h(1.0))
    return "test_invcn_h passed"



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



def test_lognormal_martingale(numRuns):
    N, M = 365, 10_000
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


def test_check_b76():
    N, M = 2, 100_000
    f = 0.05
    T = 1.0
    sigma = 0.10
    r = 0.0
    k1 = 0.0375
    k2 = 0.0800
    muNoIto = sigma * sigma * T * 0.5   # to kill the ito drift added in fill_matrix("log")

    putStrikes = k1, f - 0.0025, 0.0025
    callStrikes = f, k2, 0.0025

    t = np.zeros((N,M))             # to keep (k - F) and 0 for option payout
    seChunkSize = 10_000

    closed = {}
    for k in np.arange(*putStrikes):
        closed[k] = qu.b76_put(T, k, f, sigma, r)
    for k in np.arange(*callStrikes):
        closed[k] = qu.b76_call(T, k, f, sigma, r)
    qu.new_mersennes_norm(2)

    ito = {}
    mc = qu.new_mersennes_norm(N, M)
    mc[0,:] = f
    mc = qu.fill_matrix(mc, "log", j1=0, j2=M-1, dt=T, sigma=sigma)
    for k in np.arange(*putStrikes):
        t[1,:] = k - mc[1,:]
        payouts = np.max(t, axis=0)
        ito[k] = np.average(payouts), np.std(payouts) / math.sqrt(seChunkSize), cse(payouts, seChunkSize)
    for k in np.arange(*callStrikes):
        t[1,:] = mc[1,:] - k
        payouts = np.max(t, axis=0)
        ito[k] = np.average(payouts), np.std(payouts) / math.sqrt(seChunkSize), cse(payouts, seChunkSize)

    anti_ito = {}
    mc = qu.new_mersennes_norm(N, M)
    for j in range(M, 0, -2):
        mc[1,j-1] = mc[1,int(j/2)-1]
        mc[1,j-2] = -mc[1,int(j/2)-1]
    mc[0,:] = f
    mc = qu.fill_matrix(mc, "log", j1=0, j2=M-1, dt=T, sigma=sigma)
    for k in np.arange(*putStrikes):
        t[1,:] = k - mc[1,:]
        payouts = np.max(t, axis=0)
        anti_ito[k] = np.average(payouts), np.std(payouts) / math.sqrt(seChunkSize), cse(payouts, seChunkSize)
    for k in np.arange(*callStrikes):
        t[1,:] = mc[1,:] - k
        payouts = np.max(t, axis=0)
        anti_ito[k] = np.average(payouts), np.std(payouts) / math.sqrt(seChunkSize), cse(payouts, seChunkSize)

    noito = {}
    mc = qu.new_mersennes_norm(N, M)
    mc[0,:] = f
    mc = qu.fill_matrix(mc, "log", j1=0, j2=M-1, dt=T, sigma=sigma, mu=muNoIto)
    for k in np.arange(*putStrikes):
        t[1,:] = k - mc[1,:]
        payouts = np.max(t, axis=0)
        noito[k] = np.average(payouts), np.std(payouts) / math.sqrt(seChunkSize), cse(payouts, seChunkSize)
    for k in np.arange(*callStrikes):
        t[1,:] = mc[1,:] - k
        payouts = np.max(t, axis=0)
        noito[k] = np.average(payouts), np.std(payouts) / math.sqrt(seChunkSize), cse(payouts, seChunkSize)


    print("strike     b76            mc ito              antithetic mc ito   no-ito err")
    for ((k, p), (mc1, se1, cse1), (amc1, ase1, acse1), (mc2, se2, cse2)) in zip(closed.items(), ito.values(), anti_ito.values(), noito.values()):
        dpk = 2
        dpp = 1
        dpse = 2
        print(
            f"{k*100:>5,.{dpk}f}%   {p*10_000:>5,.{dpp}f}bp   "
            f"{mc1*10_000:>5,.{dpp}f}bp ± {cse1*10_000:>4,.{dpse}f} / {se1*10_000:>4,.{dpse}f}   "
            f"{amc1*10_000:>5,.{dpp}f}bp ± {acse1*10_000:>4,.{dpse}f} / {ase1*10_000:>4,.{dpse}f}   "
            f"{(mc2-mc1) / se1:>5,.{dpp}f}se"
        )

    return "test_check_b76 passed"


def cse(payouts, chunkSize):
    # chunked standard error
    means = []
    for s1 in range(0, len(payouts), chunkSize):
        s2 = s1 + chunkSize
        means.append(np.average(payouts[s1:s2]))
    return np.std(means)

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
    test_cn_h() >> PP
    test_invcn_Acklam() >> PP
    test_invcn_h() >> PP
    test_black() >> PP
    # test_mersenne() >> PP
    # test_fill_matrix() >> PP
    # test_lognormal_martingale(10) >> PP
    test_check_b76() >> PP


if __name__ == '__main__':
    main()
    'passed' >> PP

