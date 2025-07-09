// ---------------------------------------------------------------------------------------------------------------------
// Copyright 2025 David Briant, https://github.com/coppertop-bones. Licensed under the Apache License, Version 2.0
// ---------------------------------------------------------------------------------------------------------------------

#ifndef SRC_BK_LIB_OS_LINUX_C
#define SRC_BK_LIB_OS_LINUX_C "bk/lib/os_linux.c"

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include "../../../include/bk/bk.h"
#include "../../../include/bk/lib/os.h"
#include "../pp.c"

pub int os_cache_line_size() {
    FILE *p = fopen("/sys/devices/system/cpu/cpu0/cache/index0/coherency_line_size", "r");
    unsigned int lineSize = 0;
    if (p) {
        fscanf(p, "%u", &lineSize);
        fclose(p);
    }
    return lineSize;
}

pub int os_page_size() {
    return getpagesize();
}

pub void *os_vm_reserve(void *addr, size_t sz) {
    void *p = mmap(addr, sz, PROT_NONE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    return p == MAP_FAILED ? NULL : p;
}

pub int os_vm_unreserve(void *addr, size_t sz) {
    return munmap(addr, sz);
}

pub int os_mprotect(void *addr, size_t sz, int prot) {
    int ret = mprotect(addr, sz, prot);
    if (ret != 0) {
        switch (errno) {
            case EACCES:
                PP(info, "The requested protection conflicts with the access permissions of the process on the specified address range.");
                break;
            case EINVAL:
                PP(info, "addr is not a multiple of the page size.");
                break;
            case ENOMEM:
                PP(info, "Addresses in the range are invalid for the address space of the process, or specify one or more pages that are not mapped.");
                break;
        }
    }
    return ret;
}

pub int os_madvise(void *addr, size_t sz, int advice) {
    // On Linux, valid advice values are MADV_NORMAL, MADV_RANDOM, MADV_SEQUENTIAL, MADV_WILLNEED, MADV_DONTNEED, etc.
    return madvise(addr, sz, advice);
}

pub int os_mwipe(void *addr, size_t sz) {
    int ret = os_mprotect(addr, sz, PROT_NONE);
    if (ret != 0) return ret;
    return madvise(addr, sz, MADV_DONTNEED);
}

pub int os_mrelease(void *addr, size_t sz) {
    int ret = os_mprotect(addr, sz, PROT_NONE);
    if (ret != 0) return ret;
#ifdef MADV_FREE
    return madvise(addr, sz, MADV_FREE);
#else
    return madvise(addr, sz, MADV_DONTNEED);
#endif
}

#endif  // SRC_BK_LIB_OS_LINUX_C
