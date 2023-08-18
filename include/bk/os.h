#ifndef __BK_OS_H
#define __BK_OS_H "bk/os.h"


#include "../all.cfg"
#include "bk.h"


export int os_cache_line_size();
export int os_page_size();

export void * jvmreserve(void *start, size_t size);
export int jvmrelease();
export int jmprotect(void *start, size_t size, int prot);
export int jmrelease(void *start, size_t size);


#endif // __BK_OS_H