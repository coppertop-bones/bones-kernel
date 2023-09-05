#ifndef __BK_OS_C
#define __BK_OS_C "bk/os.c"


#include "../../include/bk/bk.h"
#include "../../include/bk/os.h"
#include "pp.c"
#include <sys/sysctl.h>
#include <libc.h>


#if defined _WIN64 || defined _WIN32
#include "os_win64.c"
#elif defined _APPLE_ || defined __MACH__
#include "os_macos.c"
#elif defined __linux__
#include "os_linux.c"
#endif


#endif  // __BK_OS_C
