// ---------------------------------------------------------------------------------------------------------------------
// module functions
// ---------------------------------------------------------------------------------------------------------------------
#ifndef __JONES_MOD_C
#define __JONES_MOD_C "jones/mod.c"


#include "../../include/bk/bk.h"
#include "../../include/bk/os.h"
#include "_jones.h"
#include "../lib/pyutils.c"



// ---------------------------------------------------------------------------------------------------------------------
// memory manipulation
// ---------------------------------------------------------------------------------------------------------------------

pvt PyObject * _toAddress(PyObject *mod, PyObject **args, Py_ssize_t nargs) {
    if (nargs != 1) return jErrWrongNumberOfArgs(__FUNCTION__, 1, nargs);
    return PyTuple_Pack(2, PyLong_FromVoidPtr(args[0]), PyLong_FromSize_t(args[0] -> ob_refcnt));
}

pvt PyObject * _toPtr(PyObject *mod, PyObject **args, Py_ssize_t nargs) {
    if (nargs != 1) return jErrWrongNumberOfArgs(__FUNCTION__, 1, nargs);
    return PyLong_FromVoidPtr(args[0]);
}

pvt PyObject * _pageSize(PyObject *mod, PyObject **args, Py_ssize_t nargs) {
    if (nargs != 0) return jErrWrongNumberOfArgs(__FUNCTION__, 0, nargs);
    return PyLong_FromLong(os_page_size());
}

pvt PyObject * _getCacheLineSize(PyObject *mod, PyObject **args, Py_ssize_t nargs) {
    if (nargs != 0) return jErrWrongNumberOfArgs(__FUNCTION__, 0, nargs);
    return PyLong_FromLong((long) os_cache_line_size());
}

pvt PyObject * _toObj(PyObject *mod, PyObject *args) {
    // could check that address is PyObject aligned
    PyObject *object;
    if (!PyArg_ParseTuple(args, "K", &object)) return 0;
    Py_INCREF(object);
    return PyTuple_Pack(2, object, PyLong_FromSize_t(object -> ob_refcnt));
}

pvt PyObject * _ob_refcnt(PyObject *mod, PyObject *args) {
    PyObject *object;  size_t address;
    if (!PyArg_ParseTuple(args, "K", &address)) return 0;
    object = (PyObject*) address;
    return PyLong_FromSize_t(object -> ob_refcnt);
}

pvt PyObject * _malloc(PyObject *mod, PyObject **args, Py_ssize_t nargs) {
    if (nargs != 1) return jErrWrongNumberOfArgs(__FUNCTION__, 1, nargs);
    // TODO raise a type error
    if (!PyLong_Check(args[0])) return 0;        // size_t
    size_t size = (size_t) PyLong_AsSize_t(args[0]);
    void *p = malloc(size);
    return PyLong_FromVoidPtr(p);
}

pvt PyObject * _atU16(PyObject *mod, PyObject **args, Py_ssize_t nargs) {
    // for the given pointer to an array of u16 and the index get a u16

    if (nargs != 2) return jErrWrongNumberOfArgs(__FUNCTION__, 2, nargs);
    // TODO raise a type error & check within bounds of u16
    if (!PyLong_Check(args[0])) return 0;        // ptr
    if (!PyLong_Check(args[1])) return 0;        // size_t index

    size_t index = PyLong_AsSize_t(args[1]);
    unsigned short *pItem = ((unsigned short*) (PyLong_AsSize_t(args[0]) & PTR_MASK)) + index - 1;

    return PyLong_FromLong(*pItem);
}

pvt PyObject * _atU16Put(PyObject *mod, PyObject **args, Py_ssize_t nargs) {
    // for the given pointer to an array of u16, the index, set the bits given by the mask and value

    if (nargs != 4) return jErrWrongNumberOfArgs(__FUNCTION__, 4, nargs);
    // TODO raise a type error & check within bounds of u16
    if (!PyLong_Check(args[0])) return 0;        // ptr
    if (!PyLong_Check(args[1])) return 0;        // size_t index
    if (!PyLong_Check(args[2])) return 0;        // u16 bit mask
    if (!PyLong_Check(args[3])) return 0;        // u16

    size_t index = PyLong_AsSize_t(args[1]);
    unsigned short *pItem = ((unsigned short*) (PyLong_AsSize_t(args[0]) & PTR_MASK)) + index - 1;
    unsigned short mask = (unsigned short) PyLong_AsLong(args[2]);           // OPEN check range before converting
    unsigned short v = (unsigned short) PyLong_AsLong(args[3]);

    *pItem = (*pItem & (mask ^ 0xFFFF)) | (v & mask);
    return PyBool_FromLong(*pItem);
}

pvt PyObject * _atU8(PyObject *mod, PyObject **args, Py_ssize_t nargs) {
    // for the given pointer to an array of u8 and the index get a U8

    if (nargs != 2) return jErrWrongNumberOfArgs(__FUNCTION__, 2, nargs);
    // TODO raise a type error & check within bounds of u16
    if (!PyLong_Check(args[0])) return 0;        // ptr
    if (!PyLong_Check(args[1])) return 0;        // size_t index

    size_t index = PyLong_AsSize_t(args[1]);
    unsigned char *pItem = ((unsigned char*) (PyLong_AsSize_t(args[0]) & PTR_MASK)) + index - 1;

    return PyLong_FromLong(*pItem);
}

pvt PyObject * _atU8Put(PyObject *mod, PyObject **args, Py_ssize_t nargs) {
    // for the given pointer to an array of u8 and the index, set the bits given by the mask and value

    if (nargs != 4) return jErrWrongNumberOfArgs(__FUNCTION__, 4, nargs);
    // TODO raise a type error & check within bounds of u8
    if (!PyLong_Check(args[0])) return 0;        // ptr
    if (!PyLong_Check(args[1])) return 0;        // size_t index
    if (!PyLong_Check(args[2])) return 0;        // u8 bit mask
    if (!PyLong_Check(args[3])) return 0;        // u8

    size_t index = (size_t) PyLong_AsSize_t(args[1]);
    unsigned char *pItem = ((unsigned char*) (PyLong_AsSize_t(args[0]) & PTR_MASK)) + index - 1;
    unsigned char mask = (unsigned char) PyLong_AsLong(args[2]);
    unsigned char v = (unsigned char) PyLong_AsLong(args[3]);

    *pItem = (*pItem & (mask ^ 0xFF)) | (v & mask);
    return PyBool_FromLong(*pItem);
}



#endif  // __JONES_MOD_C