#ifndef __JONES_VA_C
#define __JONES_VA_C "jones/va.c"

#include "pipe_structs.c"
#include "../../include/bk/bk.h"


// https://github.com/dlang/phobos/blob/master/std/experimental/allocator/mmap_allocator.d
// https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man2/mmap.2.html
// https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man2/madvise.2.html#//apple_ref/doc/man/2/madvise

// https://stackoverflow.com/questions/55768549/in-malloc-why-use-brk-at-all-why-not-just-use-mmap

// overcommit - https://www.etalabs.net/overcommit.html - mmap - readonly, then mmap read-write what you need


#include <unistd.h>
#include <sys/mman.h>

#include "cache_line_size.h"


#define CACHE_LINE_SIZE_M1_COMPATIBLE 128
#define PAGE_SIZE_M1_COMPATIBLE _16K



// int munmap(void *addr, size_t len);
// int madvise(void *addr, size_t len, int advice);
// MADV_SEQUENTIAL
// MADV_FREE pages may be reused right away



// VA (standing for virtual arena) is an arena style allocator that tracks in units of whole number of pages
// it informs the os whenever a whole page is no longer needed from physical memory
// it is fixed size
// it forces us to be aware of alignment and cache lines
// we use it to store types, symbols (interned subset of strings) and enums (small groups of interned strings)
// we want dispatch caches to be compact - but we don't know the size of the args yet
// inference types can be done in a large scratch pad -

// need space for hash maps which may need resizing, growable arrays to hold utfs strings, object pointers



typedef struct VA {
    // new line
    size_t cachelinesize;
    size_t pagesize;
    void *next_free_page;       // if we need to realloc we just drop the page(s) back to OS rather than reusing ourself
    void *ceiling;              // points to the byte after my last byte
    unsigned int num_reserved;         // can count up to 16TB at 4096k per page
    unsigned int num_unreserved;
} VA;


typedef struct Chunk {
    void *ceiling;              // points to the byte after my last byte
} Chunk;


pvt VA * init_va(size_t numpages) {
    size_t pagesize = db_os_page_size();
    size_t cachelinesize = db_os_cache_line_size();

    // for the mo just code for my M1
    if (pagesize != PAGE_SIZE_M1_COMPATIBLE) return 0;
    if (cachelinesize != CACHE_LINE_SIZE_M1_COMPATIBLE) return 0;

    size_t totalsize = numpages * pagesize;
    if (totalsize > _1TB) return 0;
    VA *va = (VA*) mmap((void*) 0, totalsize, PROT_READ, MAP_ANON | MAP_PRIVATE, -1, 0);
    if ((long) -1 == (long) va) return 0;
    int protect_res = mprotect((void*) va, pagesize, PROT_READ | PROT_WRITE);
    if (protect_res == -1) return 0;
    va->cachelinesize = db_os_cache_line_size();
    va->pagesize = pagesize;
    va->next_free_page = (void*)((size_t) va + pagesize);
    va->ceiling = (void*)((size_t) va + totalsize);
    va->num_reserved = 1;
    va->num_unreserved = 0;
    return va;
}


pvt void * reserve(VA *va, size_t numpages) {
    Chunk *chunk = (Chunk *) va->next_free_page;        // we allocate the new chunk at the  what was the next free page
    void *chunk_ceiling = (void*)((size_t)va->next_free_page + numpages * va->pagesize);
    if (chunk_ceiling > va->ceiling) return 0;       // there's not enough vm left to satisfy the request
    int protect_res = mprotect((void*) chunk, numpages * va->pagesize, PROT_READ | PROT_WRITE);
    if (protect_res == -1) return 0;
    // TODO to verify os can give us the memory - if not return 0  // will MADV_WILLNEED work?
    chunk->ceiling = chunk_ceiling;
    va->next_free_page = chunk_ceiling;
    va->num_reserved += numpages;
    return (void *) chunk;
}

pvt int unreserve(VA *va, Chunk *chunk) {
    size_t size = (size_t) chunk->ceiling - (size_t) chunk;
    int protect_res = mprotect((void*) chunk, size, 0);
    if (protect_res == -1) return 0;
    va->num_unreserved += (unsigned int)(size / va->pagesize);
    madvise((void*) chunk, size, MADV_FREE);            // tell os can reclaim the physical memory
    return 1;
}


static VA *g_va;

pvt PyObject * _reserve(PyObject *mod, PyObject *const *args, Py_ssize_t nargs) {
    VA *va; size_t numpages, pChunk;

    if (nargs != 2) return _raiseWrongNumberOfArgs(__FUNCTION__, 2, nargs);
    // TODO raise a type error & check within bounds of u16
    if (!PyLong_Check(args[0])) return 0;        // ptr
    if (!PyLong_Check(args[1])) return 0;        // u16 index

    va = (VA*) PyLong_AsLong(args[0]);
    numpages = (size_t) PyLong_AsLong(args[1]);
    pChunk = (size_t) reserve(va, numpages);
    return PyLong_FromLong(pChunk);
}

pvt PyObject * _getVaPtr(PyObject *mod, PyObject *const *args, Py_ssize_t nargs) {
    if (nargs != 0) return _raiseWrongNumberOfArgs(__FUNCTION__, 0, nargs);
    return PyLong_FromLong((long) g_va);
}



#endif  // __JONES_VA_C