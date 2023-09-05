#ifndef __BK_UTILS_OS_LINUX_C
#define __BK_UTILS_OS_LINUX_C "bk/os_linux.c"

#include <stdio.h>
#include "../../include/bk/bk.h"

pub size_t os_cache_line_size() {
    FILE * p = 0;
    p = fopen("/sys/devices/system/cpu/cpu0/cache/index0/coherency_line_size", "r");
    unsigned int lineSize = 0;
    if (p) {
        fscanf(p, "%d", &lineSize);
        fclose(p);
    }
    return lineSize;
}

#endif  // __BK_UTILS_OS_LINUX_C
