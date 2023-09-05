#ifndef __BK_OS_H
#define __BK_OS_H "bk/os.h"


#include "../all.cfg"
#include "bk.h"


pub int os_cache_line_size();
pub int os_page_size();

pub void * jvmreserve(void *start, size_t size);
pub int jvmrelease();
pub int jmprotect(void *start, size_t size, int prot);
pub int jmrelease(void *start, size_t size);


#endif // __BK_OS_H