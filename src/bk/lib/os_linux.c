// ---------------------------------------------------------------------------------------------------------------------
// Copyright 2025 David Briant, https://github.com/coppertop-bones. Licensed under the Apache License, Version 2.0
// ---------------------------------------------------------------------------------------------------------------------

#ifndef SRC_BK_LIB_OS_LINUX_C
#define SRC_BK_LIB_OS_LINUX_C "bk/os_linux.c"

#include <stdio.h>
#include "../../../include/bk/bk.h"
#include "../../../include/bk/lib/os.h"
#include "../pp.c"

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

#endif  // SRC_BK_LIB_OS_LINUX_C
