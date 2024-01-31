// ---------------------------------------------------------------------------------------------------------------------
// OS - OPERATING SYSTEM API
// ---------------------------------------------------------------------------------------------------------------------

#ifndef INC_BK_OS_H
#define INC_BK_OS_H "bk/os.h"


#include "bk.h"

#define BK_M_NONE       0x0
#define BK_M_READ       0x1
#define BK_M_WRITE      0x2
#define BK_M_EXEC       0x4


#define BK_AD_NORMAL            0
#define BK_AD_RANDOM            1
#define BK_AD_SEQUENTIAL        2
#define BK_AD_WILLNEED          3
#define BK_AD_DONTNEED          4
#define BK_AD_FREE              5


pub int os_cache_line_size();
pub int os_page_size();

pub void * os_vm_reserve(void *addr, size_t len);
pub int os_vm_unreserve(void *addr, size_t len);
pub int os_mprotect(void *addr, size_t len, int prot);
pub int os_madvise(void *addr, size_t len, int advice);
pub int os_mfree(void *addr, size_t len);


#endif // INC_BK_OS_H