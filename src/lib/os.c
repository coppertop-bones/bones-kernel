// ---------------------------------------------------------------------------------------------------------------------
// os.c exposes the platform specific implementations of:
//
// int os_cache_line_size()
// int os_page_size()
//
// void * os_vm_reserve(void *addr, size_t len)         - reserves an address range - with BK_ACCESS_NONE
// int os_vm_unreserve(void *addr, size_t len)          - unreseves an address range
// int os_mprotect(void *addr, size_t len, int prot)    - sets protection - read / write / execute / none
// int os_madvise(void *addr, size_t len, int advice)   - inform the os of expected usage
// int os_mfree(void *addr, size_t len)                 - returns physical pages back to OS marking as BK_ACCESS_NONE, without unreserving the virtual address space
//
// ---------------------------------------------------------------------------------------------------------------------

#ifndef __BK_OS_C
#define __BK_OS_C "bk/os.c"


#include "../../include/bk/bk.h"
#include "../../include/bk/os.h"
#include "../bk/pp.c"
#include <sys/sysctl.h>
#include <libc.h>


// https://www.etalabs.net/overcommit.html - mmap - none, then mprotect read-write what you need


#if defined _WIN64 || defined _WIN32
#include "os_win64.c"
#elif defined _APPLE_ || defined __MACH__
#include "os_macos.c"
#elif defined __linux__
#include "os_linux.c"
#endif


#endif  // __BK_OS_C


// mremap
// https://github.com/estraier/tkrzw/issues/11 - not on macos
// https://stackoverflow.com/questions/17197615/no-mremap-for-windows - use windows AWE


