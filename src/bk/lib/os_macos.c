// ---------------------------------------------------------------------------------------------------------------------
// Copyright 2025 David Briant, https://github.com/coppertop-bones. Licensed under the Apache License, Version 2.0
// ---------------------------------------------------------------------------------------------------------------------

#ifndef SRC_BK_LIB_OS_MACOS_C
#define SRC_BK_LIB_OS_MACOS_C "bk/os_macos.c"

// https://developer.apple.com/library/archive/documentation/Performance/Conceptual/ManagingMemory/Articles/MemoryAlloc.html
// https://developer.apple.com/library/archive/documentation/Performance/Conceptual/ManagingMemory/Articles/AboutMemory.html

// https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man2/msync.2.html
// https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man3/sysconf.3.html
// https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man3/sysctl.3.html
// https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man2/mincore.2.html
// https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man2/mlock.2.html
// https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man2/minherit.2.html

// https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man3/memset.3.html
// https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man3/bzero.3.html
// https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man3/memset_pattern.3.html
// https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man3/memmove.3.html
// https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man3/memcpy.3.html
// https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man3/bcopy.3.html

// https://github.com/dlang/phobos/blob/master/std/experimental/allocator/mmap_allocator.d

// page copying and moving
// https://stackoverflow.com/questions/45043993/understanding-page-copying-in-c
// https://developer.apple.com/documentation/kernel/1585277-vm_copy
// https://github.com/lattera/glibc/blob/master/sysdeps/generic/pagecopy.h

// zeroing
// https://travisdowns.github.io/blog/2020/01/20/zero.html - https://news.ycombinator.com/item?id=22104576
// https://en.wikipedia.org/wiki/Cache_control_instruction#Data_cache_block_allocate_zero
// https://lemire.me/blog/2020/01/20/filling-large-arrays-with-zeroes-quickly-in-c/


// /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include/sys/mman.h

//#define MADV_NORMAL             0   /* [MC1] no further special treatment */
//#define MADV_RANDOM             1   /* [MC1] expect random page refs */
//#define MADV_SEQUENTIAL         2   /* [MC1] expect sequential page refs */
//#define MADV_WILLNEED           3   /* [MC1] will need these pages */
//#define MADV_DONTNEED           4   /* [MC1] dont need these pages */
//#define MADV_FREE               5   /* pages unneeded, discard contents */
//#define MADV_ZERO_WIRED_PAGES   6   /* zero the wired pages that have not been unwired before the entry is deleted */
//#define MADV_FREE_REUSABLE      7   /* pages can be reused (by anyone) */
//#define MADV_FREE_REUSE         8   /* caller wants to reuse those pages */
//#define MADV_CAN_REUSE          9
//#define MADV_PAGEOUT            10  /* page out now (internal only) */


// Protections are chosen from these bits, or-ed together
// #define  PROT_NONE   0x00   /* [MC2] no permissions */
// #define  PROT_READ   0x01   /* [MC2] pages can be read */
// #define  PROT_WRITE  0x02   /* [MC2] pages can be written */
// #define  PROT_EXEC   0x04   /* [MC2] pages can be executed */


#include "../../../include/bk/bk.h"
#include "../../../include/bk/lib/os.h"
#include "../pp.c"
#include <sys/sysctl.h>
#include <libc.h>
#include <sys/mman.h>
#include <sys/errno.h>


pub int os_cache_line_size() {
    size_t lineSize = 0;
    size_t sizeOfLineSize = sizeof(lineSize);
    sysctlbyname("hw.cachelinesize", &lineSize, &sizeOfLineSize, 0, 0);
    return lineSize;
}

pub int os_page_size() {
    // https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man3/getpagesize.3.html
    return getpagesize();
}

pub void * os_vm_reserve(void *addr, size_t sz) {
    // https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man2/mmap.2.html
    void *p = mmap(addr, sz, PROT_NONE, MAP_ANON | MAP_PRIVATE, -1, 0);
    return p;
}

pub int os_vm_unreserve(void *addr, size_t sz) {
    // https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man2/munmap.2.html
    int ret = munmap(addr, sz);
    return ret;
}

pub int os_mprotect(void *addr, size_t sz, int prot) {
    // On success answer 0, on failure set errno and answer -1
    // https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man2/mprotect.2.html
    // OPEN: check start is page aligned and size is whole number of pages>
    int ret = mprotect(addr, sz, prot);
    switch (ret) {
        case EACCES:
            PP(info, "The requested protection conflicts with the access permissions of the process on the specified "
                     "address range."
            );
        case EINVAL:
            PP(info, "addr is not a multiple of the page size.");
        case ENOTSUP:
            PP(info, "The combination of accesses requested in prot is not supported.");
    }
    return ret;
}

pub int os_madvise(void *addr, size_t sz, int advice) {
    // On success answer 0, on failure set errno and answer -1
    // https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man2/madvise.2.html
    // https://man.freebsd.org/cgi/man.cgi?query=madvise&sektion=2&format=html
    if (advice < BK_MADV_NORMAL || advice > BK_MADV_WILLNEED) return -1;
    return madvise(addr, sz, advice);
}

pub int os_mwipe(void *addr, size_t sz) {
    // https://stackoverflow.com/questions/24171602/mmap-resetting-old-memory-to-a-zerod-non-resident-state
    // https://man7.org/linux/man-pages/man2/madvise.2.html - MADV_DONTNEED
    int ret = os_mprotect(addr, sz, PROT_NONE);
    ret = madvise(addr, sz, MADV_DONTNEED);
    return ret;
}

pub int os_mrelease(void *addr, size_t sz) {
    // https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man2/mprotect.2.html
    // https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man2/madvise.2.html
    // https://man7.org/linux/man-pages/man2/madvise.2.html - MADV_FREE
    int ret = os_mprotect(addr, sz, PROT_NONE);
    ret = madvise(addr, sz, MADV_FREE);
    return ret;
}


#endif  // SRC_BK_LIB_OS_MACOS_C
