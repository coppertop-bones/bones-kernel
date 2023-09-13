from bones import jones
import sys, tests.test_tm

sys._k = jones.Kernel()
tests.test_tm.main()
sys._k = None
