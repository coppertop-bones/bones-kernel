// ---------------------------------------------------------------------------------------------------------------------
// Copyright 2025 David Briant, https://github.com/coppertop-bones. Licensed under the Apache License, Version 2.0
//
// PYKERNEL - PYTHON INTERFACE TO THE KERNEL
// ---------------------------------------------------------------------------------------------------------------------

#ifndef SRC_JONES_PYKERNEL_C
#define SRC_JONES_PYKERNEL_C "jones/pykernel.c"


#include "jones.h"
#include "../bk/mm.c"
#include "pysm.c"
#include "pyem.c"
#include "pytm.c"


// ---------------------------------------------------------------------------------------------------------------------
// PyKernelCls
// ---------------------------------------------------------------------------------------------------------------------

pvt PyObject * PyKernel_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    // OPEN: assert no args or kwargs are passed
    PyKernel *self = (PyKernel *) type->tp_alloc(type, 0);
    BK_MM *mm = MM_create();
    Buckets *buckets = mm->malloc(sizeof(Buckets));
    Buckets_init(buckets, BUCKETS_CHUNK_SIZE);
    self->kernel = K_create(mm, buckets);
    self->pySM = (PyObject *) ((&PySMCls)->tp_alloc(&PySMCls, 0));
    ((PySM *) self->pySM)->sm = self->kernel->sm;
    self->pyEM = (PyObject *) ((&PyEMCls)->tp_alloc(&PyEMCls, 0));
    ((PyEM *) self->pyEM)->em = self->kernel->em;
    self->pyTM = (PyObject *) ((&PyTMCls)->tp_alloc(&PyTMCls, 0));
    ((PyTM *) self->pyTM)->tm = self->kernel->tm;
    return (PyObject *) self;
}

pvt void PyKernel_dealloc(PyKernel *self) {
    BK_MM *mm = self->kernel->mm;
    Buckets_finalise(self->kernel->buckets);
    i32 res = K_trash(self->kernel);
    if (res) PP(error, "%s: K_trash failed", FN_NAME);
    res = MM_trash(mm);
    if (res) PP(error, "%s: MM_trash failed", FN_NAME);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

pvt PyMemberDef PyKernel_members[] = {
    {"sm", Py_T_OBJECT_EX, offsetof(PyKernel, pySM), Py_READONLY, "symbol manager"},
    {"em", Py_T_OBJECT_EX, offsetof(PyKernel, pyEM), Py_READONLY, "enum manager"},
    {"tm", Py_T_OBJECT_EX, offsetof(PyKernel, pyTM), Py_READONLY, "type manager"},
    {0}
};

pvt PyMethodDef PyKernel_methods[] = {
    {0}
};

pvt PyTypeObject PyKernelCls = {
        PyVarObject_HEAD_INIT(0, 0)
        .tp_name = "jones.Kernel",
        .tp_doc = PyDoc_STR("TBC"),
        .tp_basicsize = sizeof(PyKernel),
        .tp_itemsize = 0,
        .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
        .tp_new = PyKernel_new,
//        .tp_init = (initproc) PyOM_init,
        .tp_dealloc = (destructor) PyKernel_dealloc,
        .tp_members = PyKernel_members,
        .tp_methods = PyKernel_methods,
};

#endif  // SRC_JONES_PYKERNEL_C