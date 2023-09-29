#ifndef __BK_OS_H
#define __BK_OS_H "bk/os.h"


#include "../all.cfg"
#include "bk.h"

#include <sys/mman.h>
#include <sys/errno.h>

pub int os_cache_line_size();
pub int os_page_size();

pub void * os_vm_reserve(void *addr, size_t len);
pub int os_vm_unreserve(void *addr, size_t len);
pub int os_mprotect(void *addr, size_t len, int prot);
pub int os_madvise(void *addr, size_t len, int advice);
pub int os_mfree(void *addr, size_t len);


#endif // __BK_OS_H