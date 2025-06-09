// ---------------------------------------------------------------------------------------------------------------------
// Copyright 2025 David Briant, https://github.com/coppertop-bones. Licensed under the Apache License, Version 2.0
//
// OS - OPERATING SYSTEM API
//
// PURPOSE
//   Provide a cross-platform api so the bone VM can provide:
//     - huge amounts of memory for a function and return it effectively and immediate to the OS
//     - allow dynamic compilation
//
//
// API - INFORMATION
//   int os_cache_line_size()
//   int os_page_size()
//
//
// API - MEMORY MANAGEMENT
//   void * os_vm_reserve(void *addr, size_t sz)
//     reserves an address range - with BK_ACCESS_NONE
//
//   int os_vm_unreserve(void *addr, size_t sz)
//     - unreserves a previously reserved address range
//     - windows can only unreserve the whole lot but we don't want to track the size here so sz must be provided and
//       correspond to the sz reserved using os_vm_reserve
//
//   int os_mprotect(void *addr, size_t sz, int prot)
//     - sets protection - read / write / execute / none
//
//   int os_madvise(void *addr, size_t sz, int advice)
//     - inform the os of expected usage
//
//   int os_mwipe(void *addr, size_t sz);
//     - reset the page after marking as BK_ACCESS_NONE (next access should get zeros)
//
//   OPEN: decharge, decommit?
//   int os_mrelease(void *addr, size_t sz)
//     - returns physical pages back to OS after marking as BK_ACCESS_NONE
//
// NOTES
//   - addr amd sz must always be a whole number of pages
//   - "memory charge" - the overall size of memory the os has said it can provide including physical memory and
//      paging files on disk
//
//
//
// USE CASES
//   - the obvious ones
//   - hiding memory for a while using BK_M_NONE?
//   - guard pages?
//   - cross process sharing and IPC? could also share types, symbols, functions?
//   - threading concerns?
//
//
// OS API FUNCTIONS
//
//   Windows
//     - VirtualAlloc   - https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtualalloc
//     - VirtualFree    - https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtualfree
//     - VirtualProtect - https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtualprotect
//     - https://learn.microsoft.com/en-us/windows/win32/Memory/memory-protection-constants
//
//   macOS
//     mmap
//     munmap
//     mprotect - https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man2/mprotect.2.html
//     madvise
//
//   linux
//
// ---------------------------------------------------------------------------------------------------------------------

#ifndef INC_BK_LIB_OS_H
#define INC_BK_LIB_OS_H "bk/lib/os.h"

#include "../bk.h"

// These BK_PROT_ #defines are the same as the macOS ones thus must not be changed
#define BK_PROT_NONE       0x0
#define BK_PROT_READ       0x1
#define BK_PROT_WRITE      0x2
#define BK_PROT_EXEC       0x4

// These BK_MADV_ #defines are the same as the macOS ones thus must not be changed
#define BK_MADV_NORMAL            0
#define BK_MADV_RANDOM            1
#define BK_MADV_SEQUENTIAL        2
#define BK_MADV_WILLNEED          3

pub int os_cache_line_size();
pub int os_page_size();

pub void * os_vm_reserve(void *addr, size_t sz);
pub int os_vm_unreserve(void *addr, size_t sz);
pub int os_mprotect(void *addr, size_t sz, int prot);
pub int os_madvise(void *addr, size_t sz, int advice);
pub int os_mwipe(void *addr, size_t sz);
pub int os_mrelease(void *addr, size_t sz);


#endif  // INC_BK_LIB_OS_H
