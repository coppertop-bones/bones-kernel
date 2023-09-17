#ifndef __JONES_PLAY_C
#define __JONES_PLAY_C "jones/play.c"


#include "_jones.h"
#include "../bk/ht_impl.h"
#include "../../include/bk/bk.h"
#include "../../include/bk/os.h"
#include "../other/khash.h"

#include <unistd.h>
#include <sys/mman.h>


// https://llllllllll.github.io/c-extension-tutorial/fancy-argument-parsing.html
// http://web.mit.edu/people/amliu/vrut/python/ext/parseTuple.html
// https://docs.activestate.com/activepython/3.8/python/c-api/structures.html#c._PyCFunctionFast



pvt bk_inline u32 _X31_hash_fred(const char *s) {
    u32 h = (u32)*s;
    if (h) for (++s ; *s; ++s) h = (h << 5) - h + (u32)*s;
    return h;
}

pvt int fredcmp (const char *p1, const char *p2) {
    const char *s1 = (const char *) p1;
    const char *s2 = (const char *) p2;
    char c1, c2;
    do {
        c1 = (char) *s1++;
        c2 = (char) *s2++;
        if (c1 == '\0') return c1 - c2;
    }
    while (c1 == c2);
    return c1 - c2;
}

#define kh_fred_hash_func(h, key) _X31_hash_fred(key)
#define kh_fred_hash_equal(h, a, b) (fredcmp(a, b) == 0)

struct hm_u32_u8 {

};

KHASH_MAP_STRUCT(hm_u32_u8, khint32_t, unsigned char)
KHASH_IMPL(hm_u32_u8, khint32_t, unsigned char, KHASH_MAP, kh_int_hash_func, kh_int_hash_equal)

KHASH_MAP_STRUCT(hm_txt_u32, kh_cstr_t, unsigned int)
KHASH_IMPL(hm_txt_u32, kh_cstr_t, unsigned int, KHASH_MAP, kh_str_hash_func, kh_str_hash_equal)

KHASH_MAP_STRUCT(hm_txt_typenum, kh_cstr_t, unsigned short)
KHASH_IMPL(hm_txt_typenum, kh_cstr_t, unsigned short, KHASH_MAP, kh_fred_hash_func, kh_fred_hash_equal)



struct PyFred {
    PyObject_HEAD                   // ob_refcnt:Py_ssize_t, *ob_type:PyTypeObject
};


struct PyJoe {
    PyObject_VAR_HEAD                   // ob_refcnt:Py_ssize_t, *ob_type:PyTypeObject
};


struct PyPlay {
    PyObject_HEAD                   // ob_refcnt:Py_ssize_t, *ob_type:PyTypeObject
    PyObject *first;                // first name
    PyObject *last;                 // last name
    int number;
    kh_struct(hm_u32_u8) *hm;         // (u32**u8)&hashmap
};


struct VA {
    size_t cachelinesize;
    size_t pagesize;
    void *next_free_page;           // if we need to realloc we just drop the page(s) back to OS rather than reusing ourself
    void *ceiling;                  // points to the byte after my last byte
    unsigned int num_reserved;      // can count up to 16TB at 4096k per page
    unsigned int num_unreserved;
};


struct Chunk {
    void *ceiling;                  // points to the byte after my last byte
};


pvt PyTypeObject PyPlayCls;
pvt struct VA *g_va;



// ---------------------------------------------------------------------------------------------------------------------
// free fns
// ---------------------------------------------------------------------------------------------------------------------

pvt PyObject * _sizeofFredJoe(PyObject *mod, PyObject *const *args, Py_ssize_t nargs) {
    if (nargs != 0) return _raiseWrongNumberOfArgs(__FUNCTION__, 0, nargs);
    return PyTuple_Pack(2, PyLong_FromLong((long) sizeof(struct PyFred)), PyLong_FromLong((long) sizeof(struct PyJoe)));
}


pvt PyObject * _execShell(PyObject *mod, PyObject *args) {
    const char *command;  int ret;
    if (!PyArg_ParseTuple(args, "s", &command)) return 0;
    ret = system(command);
    if (ret < 0) {
        PyErr_SetString(PyJonesError, "System command failed");
        return 0;
    }
    return PyLong_FromLong(ret);
}



// ---------------------------------------------------------------------------------------------------------------------
// PyPlay
// ---------------------------------------------------------------------------------------------------------------------

pvt void PyPlay_dealloc(struct PyPlay *self) {
    kh_trash(hm_u32_u8, self->hm);
    Py_XDECREF(self->first);
    Py_XDECREF(self->last);
    Py_TYPE(self)->tp_free((PyObject *) self);
}


pvt PyObject * PyPlay_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    struct PyPlay *self;
    self = (struct PyPlay *) type->tp_alloc(type, 0);
    if (self != 0) {
        self->first = PyUnicode_FromString("");         // ref count will be 1
        if (self->first == 0) {
            Py_DECREF(self);
            return 0;
        }
        self->last = PyUnicode_FromString("");
        if (self->last == 0) {
            Py_DECREF(self);
            return 0;
        }
        self->number = 0;
        self->hm = kh_create(hm_u32_u8);
    }
    return (PyObject *) self;
}


pvt int PyPlay_init(struct PyPlay *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {"first", "last", "number", 0};
    PyObject *first = 0, *last = 0, *old;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|OOi", kwlist, &first, &last, &self->number)) return -1;

    if (first) {
        old = self->first;
        Py_INCREF(first);               // we also own a ref to first
        self->first = first;
        Py_XDECREF(old);
    }
    if (last) {
        old = self->last;
        Py_INCREF(last);
        self->last = last;
        Py_XDECREF(old);
    }
    return 0;
}


pvt PyMemberDef PyPlay_members[] = {
        {"first", T_OBJECT_EX, offsetof(struct PyPlay, first), 0, "first name"},
        {"last", T_OBJECT_EX, offsetof(struct PyPlay, last), 0, "last name"},
        {"number", T_INT, offsetof(struct PyPlay, number), 0, "custom number"},
        {0}
};


pvt PyObject * PyPlay_name(struct PyPlay *self, PyObject *Py_UNUSED(ignored)) {
    if (self->first == 0) {
        PyErr_SetString(PyExc_AttributeError, "first");
        return 0;
    }
    if (self->last == 0) {
        PyErr_SetString(PyExc_AttributeError, "last");
        return 0;
    }
    return PyUnicode_FromFormat("%S %S", self->first, self->last);
}


pvt PyObject * PyPlay_has(struct PyPlay *self, PyObject *const *args, Py_ssize_t nargs) {
    kh_iter_t it;  int exists;  int k;
    if (nargs != 1) return _raiseWrongNumberOfArgs(__FUNCTION__, 1, nargs);
    if (!PyLong_Check(args[0])) return 0;       // TODO raise a type error
    k = (int) PyLong_AsLong(args[0]);
//    if (!PyArg_ParseTuple(args, "I", &key)) return 0;
    it = kh_get_it(hm_u32_u8, self->hm, k);        // find key or end
    exists = (it != kh_it_end(self->hm));
    return PyBool_FromLong(exists);
}


pvt PyObject * PyPlay_atIfNone(struct PyPlay *self, PyObject *const *args, Py_ssize_t nargs) {
    kh_iter_t it;  int k;
    if (nargs != 2) return _raiseWrongNumberOfArgs(__FUNCTION__, 2, nargs);
    if (!PyLong_Check(args[0])) return 0;       // TODO raise a type error
    k = (int) PyLong_AsLong(args[0]);
    it = kh_get_it(hm_u32_u8, self->hm, k);        // find key or end
    if (it == kh_it_end(self->hm))
        return args[1];
    else
        return PyLong_FromLong(kh_value(self->hm, it));
}


pvt PyObject * PyPlay_atPut(struct PyPlay *self, PyObject *const *args, Py_ssize_t nargs) {
    kh_iter_t it;  int ret;  int k;  int v;
    if (nargs != 2) return _raiseWrongNumberOfArgs(__FUNCTION__, 2, nargs);
    if (!PyLong_Check(args[0])) return 0;        // TODO raise a type error
    if (!PyLong_Check(args[1])) return 0;        // TODO raise a type error
    k = (int) PyLong_AsLong(args[0]);
    v = (int) PyLong_AsLong(args[1]);

    it = kh_put_it(hm_u32_u8, self->hm, k, &ret);  // find key or insert
    if (ret == -1) return 0;
    kh_value(self->hm, it) = v;                 // set the value

    Py_INCREF(self);
    return (PyObject *) self;
}


pvt PyObject * PyPlay_drop(struct PyPlay *self, PyObject *const *args, Py_ssize_t nargs) {
    kh_iter_t it;  int k;
    if (nargs != 1) return _raiseWrongNumberOfArgs(__FUNCTION__, 1, nargs);
    if (!PyLong_Check(args[0])) return 0;           // TODO raise a type error
    k = (int) PyLong_AsLong(args[0]);

    it = kh_get_it(hm_u32_u8, self->hm, k);         // find key or end
    if (it != kh_it_end(self->hm))
        kh_del(hm_u32_u8, self->hm, it);            // TODO raise error if absent?

    // https://docs.python.org/3/extending/extending.html#ownership-rules
    // "The object reference returned from a C function that is called from Python must be an owned reference"
    Py_INCREF(self);
    return (PyObject *) self;
}


pvt PyObject * PyPlay_count(struct PyPlay *self, PyObject *const *args, Py_ssize_t nargs) {
    if (nargs != 0) return _raiseWrongNumberOfArgs(__FUNCTION__, 0, nargs);
    return PyLong_FromLong(kh_size(self->hm));
}


pvt PyObject * PyPlay_numBuckets(struct PyPlay *self, PyObject *const *args, Py_ssize_t nargs) {
    if (nargs != 0) return _raiseWrongNumberOfArgs(__FUNCTION__, 0, nargs);
    return PyLong_FromLong(kh_n_buckets(self->hm));
}


pvt PyMethodDef PyPlay_methods[] = {
        {"has", (PyCFunction) PyPlay_has, METH_FASTCALL, "has(key)\n\nanswer if has key"},
        {"atPut", (PyCFunction) PyPlay_atPut, METH_FASTCALL, "atPut(key, value)\n\nat key put value, answer self"},
        {"atIfNone", (PyCFunction) PyPlay_atIfNone, METH_FASTCALL, "atIfNone(key, value, alt)\n\nanswer the value at key or alt if the key is absent"},
        {"drop", (PyCFunction) PyPlay_drop, METH_FASTCALL, "drop(key)\n\ndrop value at key, answer self"},
        {"count", (PyCFunction) PyPlay_count, METH_FASTCALL, "count()\n\nanswer the number of elements"},
        {"numBuckets", (PyCFunction) PyPlay_numBuckets, METH_FASTCALL, "numBuckets()\n\nanswer the number of buckets"},
        {"name", (PyCFunction) PyPlay_name, METH_NOARGS, "Return the name, combining the first and last name"},
        {0}
};


pvt PyTypeObject PyPlayCls = {
    PyVarObject_HEAD_INIT(0, 0)
    .tp_name = "jones.Toy",
    .tp_doc = PyDoc_STR("a Toy to play with"),
    .tp_basicsize = sizeof(struct PyPlay),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = PyPlay_new,                                      // was PyType_GenericNew
    .tp_init = (initproc) PyPlay_init,
    .tp_dealloc = (destructor) PyPlay_dealloc,
    .tp_members = PyPlay_members,
    .tp_methods = PyPlay_methods,
};



// ---------------------------------------------------------------------------------------------------------------------
// VA
// ---------------------------------------------------------------------------------------------------------------------

// https://github.com/dlang/phobos/blob/master/std/experimental/allocator/mmap_allocator.d
// https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man2/mmap.2.html
// https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man2/madvise.2.html#//apple_ref/doc/man/2/madvise

// https://stackoverflow.com/questions/55768549/in-malloc-why-use-brk-at-all-why-not-just-use-mmap

// overcommit - https://www.etalabs.net/overcommit.html - mmap - readonly, then mmap read-write what you need

#define CACHE_LINE_SIZE_M1_COMPATIBLE 128
#define PAGE_SIZE_M1_COMPATIBLE _16K

// int munmap(void *addr, size_t len);
// int madvise(void *addr, size_t len, int advice);
// MADV_SEQUENTIAL
// MADV_FREE pages may be reused right away

pvt struct VA * init_va(size_t numpages) {
    size_t pagesize = os_page_size();
    size_t cachelinesize = os_cache_line_size();

    // for the mo just code for my M1
    if (pagesize != PAGE_SIZE_M1_COMPATIBLE) return 0;
    if (cachelinesize != CACHE_LINE_SIZE_M1_COMPATIBLE) return 0;

    size_t totalsize = numpages * pagesize;
    if (totalsize > _1TB) return 0;
    struct VA *va = (struct VA*) mmap((void*) 0, totalsize, PROT_READ, MAP_ANON | MAP_PRIVATE, -1, 0);
    if ((long) -1 == (long) va) return 0;
    int protect_res = mprotect((void*) va, pagesize, PROT_READ | PROT_WRITE);
    if (protect_res == -1) return 0;
    va->cachelinesize = os_cache_line_size();
    va->pagesize = pagesize;
    va->next_free_page = (void*)((size_t) va + pagesize);
    va->ceiling = (void*)((size_t) va + totalsize);
    va->num_reserved = 1;
    va->num_unreserved = 0;
    return va;
}


pvt void * reserve(struct VA *va, size_t numpages) {
    struct Chunk *chunk = (struct Chunk *) va->next_free_page;        // we allocate the new chunk at what was the next free page
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


pvt int unreserve(struct VA *va, struct Chunk *chunk) {
    size_t size = (size_t) chunk->ceiling - (size_t) chunk;
    int protect_res = mprotect((void*) chunk, size, 0);
    if (protect_res == -1) return 0;
    va->num_unreserved += (unsigned int)(size / va->pagesize);
    madvise((void*) chunk, size, MADV_FREE);            // tell os can reclaim the physical memory
    return 1;
}


pvt PyObject * _reserve(PyObject *mod, PyObject *const *args, Py_ssize_t nargs) {
    struct VA *va; size_t numpages, pChunk;

    if (nargs != 2) return _raiseWrongNumberOfArgs(__FUNCTION__, 2, nargs);
    // TODO raise a type error & check within bounds of u16
    if (!PyLong_Check(args[0])) return 0;        // ptr
    if (!PyLong_Check(args[1])) return 0;        // u16 index

    va = (struct VA*) PyLong_AsLong(args[0]);
    numpages = (size_t) PyLong_AsLong(args[1]);
    pChunk = (size_t) reserve(va, numpages);
    return PyLong_FromLong(pChunk);
}


pvt PyObject * _getVaPtr(PyObject *mod, PyObject *const *args, Py_ssize_t nargs) {
    if (nargs != 0) return _raiseWrongNumberOfArgs(__FUNCTION__, 0, nargs);
    return PyLong_FromLong((long) g_va);
}



#endif  // __JONES_PLAY_C