#ifndef __BK_OS_WIN64_C
#define __BK_OS_WIN64_C "bk/os_win64.c"


#include <errno.h>
#include <limits.h> /* for INT_MAX */
#include <stdarg.h>
#include <stdio.h>          /* for vsnprintf */
#include <stdlib.h>
#define _AMD64_
#include <sysinfoapi.h>
//#include <windows.h>
#include "../../../include/bk/bk.h"
//#include "os_win64.h"

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

pub void * os_vm_reserve(void *addr, size_t len) {
    return 0;
}

pub int os_vm_unreserve(void *addr, size_t len) {
    return 0;
}

pub int os_mprotect(void *addr, size_t len, int prot) {
    return 0;
}

pub int os_madvise(void *addr, size_t len, int advice) {
    return 0;
}

pub int os_mfree(void *addr, size_t len) {
    return 0;
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

#endif  // __BK_OS_WIN64_C