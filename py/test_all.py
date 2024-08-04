from bones import jones
import sys
from tests import test_sm, test_tm

sys._k = jones.Kernel()
test_sm.main()
test_tm.main()
sys._k = None
