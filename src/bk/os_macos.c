// https://developer.apple.com/library/archive/documentation/Performance/Conceptual/ManagingMemory/Articles/MemoryAlloc.html
// https://developer.apple.com/library/archive/documentation/Performance/Conceptual/ManagingMemory/Articles/AboutMemory.html

// https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man3/getpagesize.3.html
// https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man2/mmap.2.html
// https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man2/mprotect.2.html
// https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man2/madvise.2.html
// https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man2/munmap.2.html
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



#ifndef __BK_OS_MACOS_C
#define __BK_OS_MACOS_C "bk/os_macos.c"


#include "../../include/bk/bk.h"
#include "pp.c"
#include <sys/sysctl.h>
#include <libc.h>

pub int os_cache_line_size() {
    size_t lineSize = 0;
    size_t sizeOfLineSize = sizeof(lineSize);
    sysctlbyname("hw.cachelinesize", &lineSize, &sizeOfLineSize, 0, 0);
    return lineSize;
}

pub int os_page_size() {
    return getpagesize();
}

pub void * jvmreserve(void *start, size_t size) {
    // reserves virtual address space - marked as PROT_NONE
    return (void *) 0;
}

pub int jvmrelease() {
    // tidying up before shutdown?
    return 1;
}

pub int jmprotect(void *start, size_t size, int prot) {
    // check start is page aligned and size is whole number of pages
    return 1;
}

pub int jmrelease(void *start, size_t size) {
    // tell os the range is no longer needed via madvise
    // mark as PROT_NONE via mprotect
    return 1;
}


#endif  // __BK_OS_MACOS_C
