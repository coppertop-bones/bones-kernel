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

//#define MADV_ZERO_WIRED_PAGES   6       /* zero the wired pages that have not been unwired before the entry is deleted */
//#define MADV_FREE_REUSABLE      7       /* pages can be reused (by anyone) */
//#define MADV_FREE_REUSE         8       /* caller wants to reuse those pages */
//#define MADV_CAN_REUSE          9
//#define MADV_PAGEOUT            10      /* page out now (internal only) */


#ifndef __BK_OS_MACOS_C
#define __BK_OS_MACOS_C "bk/os_macos.c"


#include "../../include/bk/bk.h"
//#include "../../include/bk/os.h"
#include "../bk/pp.c"
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

pub void * os_vm_reserve(void *addr, size_t len) {
    // https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man2/mmap.2.html
    void *p = mmap(addr, len, PROT_NONE, MAP_ANON | MAP_PRIVATE, -1, 0);
    return p;
}

pub int os_vm_unreserve(void *addr, size_t len) {
    // https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man2/munmap.2.html
    int ret = munmap(addr, len);
    return ret;
}

pub int os_mprotect(void *addr, size_t len, int prot) {
    // https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man2/mprotect.2.html
    // OPEN: check start is page aligned and size is whole number of pages>
    int ret = mprotect(addr, len, prot);
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

pub int os_madvise(void *addr, size_t len, int advice) {
    // https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man2/madvise.2.html
    // https://man.freebsd.org/cgi/man.cgi?query=madvise&sektion=2&format=html
    int ret = madvise(addr, len, advice);
    return ret;
}

pub int os_mfree(void *addr, size_t len) {
    int ret = os_mprotect(addr, len, PROT_NONE);
    ret = madvise(addr, len, MADV_FREE);
    return ret;
}


#endif  // __BK_OS_MACOS_C
