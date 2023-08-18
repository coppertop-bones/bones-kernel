#ifndef __BK_OS_C
#define __BK_OS_C "bk/os.c"


#include "../../include/bk/bk.h"
#include "pp.c"
#include <sys/sysctl.h>
#include <libc.h>

export int os_cache_line_size();
export int os_page_size();
export void * jvmreserve(void *start, size_t size);
export int jvmrelease();
export int jmprotect(void *start, size_t size, int prot);
export int jmrelease(void *start, size_t size);

#if defined _WIN64 || defined _WIN32
#include "os_win64.c"
#elif defined _APPLE_ || defined __MACH__
#include "os_macos.c"
#elif defined __linux__
#include "os_linux.c"
#endif


#endif  // __BK_OS_C
