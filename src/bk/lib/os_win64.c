// ---------------------------------------------------------------------------------------------------------------------
// Copyright 2025 David Briant, https://github.com/coppertop-bones. Licensed under the Apache License, Version 2.0
// ---------------------------------------------------------------------------------------------------------------------

#ifndef SRC_BK_LIB_OS_WIN64_C
#define SRC_BK_LIB_OS_WIN64_C "bk/os_win64.c"

#include <errno.h>
#include <limits.h>         /* for INT_MAX */
#include <stdarg.h>
#include <stdio.h>          /* for vsnprintf */
#include <stdlib.h>
#define _AMD64_
#include <sysinfoapi.h>
#include <memoryapi.h>
#include <errhandlingapi.h>
//#include <windows.h>
#include "../../../include/bk/bk.h"
#include "../../../include/bk/lib/os.h"
#include "../pp.c"

// win api constants
// #define MEM_COMMIT              0x00000000
// #define MEM_RESERVE             0x00002000
// #define MEM_RESET               0x00080000
// #define MEM_RESET_UNDO          0x01000000
// #define MEM_LARGE_PAGES         0x20000000
// #define MEM_PHYSICAL            0x00400000
// #define MEM_TOP_DOWN            0x00100000
// #define MEM_WRITE_WATCH         0x00200000

// #define MEM_DECOMMIT            0x00004000
// #define MEM_RELEASE             0x00008000

// #define PAGE_EXECUTE            0x10
// #define PAGE_EXECUTE_READ       0x20
// #define PAGE_EXECUTE_READWRITE  0x40
// #define PAGE_EXECUTE_WRITECOPY  0x80
// #define PAGE_NOACCESS           0x01
// #define PAGE_READONLY           0x02
// #define PAGE_READWRITE          0x04
// #define PAGE_WRITECOPY          0x08
// #define PAGE_TARGETS_INVALID    0x40000000
// #define PAGE_TARGETS_NO_UPDATE  0x40000000
// #define PAGE_GUARD              0x100
// #define PAGE_NOCACHE            0x200
// #define PAGE_WRITECOMBINE       0x400

// reserve      - VirtualAlloc(addr, sz, MEM_RESERVE, PAGE_NOACCESS)
// unreserve    - VirtualFree(addr, 0, MEM_RELEASE)
// commit       - VirtualAlloc(addr, sz, MEM_COMMIT, PAGE_XXX)
// decommit     - VirtualFree(addr, sz, MEM_DECOMMIT)
// dontneed     - VirtualAlloc(addr, sz, MEM_RESET, PAGE_NOACCESS)

// MEM_RESET does not decommit

// GetLastError


pub int os_page_size() {
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    return si.dwPageSize;
}

pub int os_cache_line_size() {
    int lineSize = 0;
    DWORD bufferSize = 0;
    SYSTEM_LOGICAL_PROCESSOR_INFORMATION * buffer = 0;

    GetLogicalProcessorInformation(0, &bufferSize);
    buffer = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION *) malloc(bufferSize);
    GetLogicalProcessorInformation(&buffer[0], &bufferSize);

    for (int i = 0; i != bufferSize / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION); ++i) {
        if (buffer[i].Relationship == RelationCache && buffer[i].Cache.Level == 1) {
            lineSize = buffer[i].Cache.LineSize;
            break;
        }
    }

    free(buffer);
    return lineSize;
}

pub void * os_vm_reserve(void *addr, size_t sz) {
    // https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtualalloc
    void *p = VirtualAlloc(addr, sz, MEM_RESERVE, PAGE_NOACCESS);
    return p;
}

pub int os_vm_unreserve(void *addr, size_t sz) {
    // https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtualfree
    return VirtualFree(addr, 0, MEM_RELEASE);
}

pub int os_mprotect(void *addr, size_t sz, int prot) {
    // On success answer 0, on failure set errno and answer -1
    if (prot == BK_PROT_NONE)                                       prot = PAGE_NOACCESS;
    else if (prot == BK_PROT_READ)                                  prot = PAGE_READONLY;
    else if (prot == (BK_PROT_READ | BK_PROT_WRITE))                prot = PAGE_READWRITE;
    else if (prot == (BK_PROT_EXEC))                                prot = PAGE_EXECUTE;
    else if (prot == (BK_PROT_READ | BK_PROT_EXEC))                 prot = PAGE_EXECUTE;
    else if (prot == (BK_PROT_READ | BK_PROT_EXEC | BK_PROT_WRITE)) prot = PAGE_EXECUTE_READWRITE;
    else return -1;
    void *p = VirtualAlloc(addr, sz, MEM_COMMIT, prot);
    if (p == addr) return 0;
    else return -1;
}

pub int os_madvise(void *addr, size_t sz, int advice) {
    // On success answer 0, on failure set errno and answer -1
    return 0;
}

pub int os_mwipe(void *addr, size_t sz) {
    // https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtualalloc - MEM_RESET cannot work
    //   "Using this value does not guarantee that the range operated on with MEM_RESET will contain zeros. If you want
    //   the range to contain zeros, decommit the memory and then recommit it."
    // https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtualfree
    int ret = VirtualFree(addr, sz, MEM_DECOMMIT);
    void *p = VirtualAlloc(addr, sz, MEM_COMMIT, PAGE_NOACCESS);
    return ret;
}

pub int os_mrelease(void *addr, size_t sz) {
    // https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtualalloc
    return VirtualFree(addr, sz, MEM_DECOMMIT);
}


// https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtualalloc
// https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-discardvirtualmemory
// https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-offervirtualmemory
// https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-reclaimvirtualmemory
// https://learn.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-getlasterror
// https://learn.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-setlasterror
// https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtualprotect
// https://learn.microsoft.com/en-us/windows/win32/Memory/memory-protection-constants

// VirtualAlloc - MEM_RESERVE, MEM_COMMIT, MEM_RESET
// VirtualProtect - PAGE_EXECUTE, PAGE_EXECUTE_READ, PAGE_EXECUTE_READWRITE, PAGE_EXECUTE_WRITECOPY, PAGE_READWRITE, PAGE_READONLY, PAGE_NOACCESS, PAGE_GUARD
// VirtualFree - MEM_DECOMMIT, MEM_RELEASE

// https://github.com/Kevin-Jin/mmap/issues/21

#endif  // SRC_BK_LIB_OS_WIN64_C
