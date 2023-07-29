// ---------------------------------------------------------------------------------------------------------------------
// Copyright 2025 David Briant, https://github.com/coppertop-bones. Licensed under the Apache License, Version 2.0
//
// PYBTYPE - PYTHON BTYPE
// ---------------------------------------------------------------------------------------------------------------------

#ifndef SRC_JONES_PYBTYPE_C
#define SRC_JONES_PYBTYPE_C "jones/pybtype.c"

#include "jones.h"



// ---------------------------------------------------------------------------------------------------------------------
// PyBTypeCls
// ---------------------------------------------------------------------------------------------------------------------

pvt PyObject * PyBType_create(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    PyBType *self = (PyBType *) type->tp_alloc(type, 0);
    return (PyObject *) self;
}

pvt void PyBType_trash(PyBType *self) {
    Py_TYPE(self)->tp_free((PyObject *) self);
}

pvt PyMemberDef PyBType_members[] = {
    {"id", Py_T_UINT, offsetof(PyBType, btypeid), 0, "bones type id"},
    {0}
};

pvt PyMethodDef PyBType_methods[] = {
    {0}
};

//char *buf = malloc(1000);

pvt PyObject * PyBType__str__(PyBType *self) {
//    return PyString_FromFormat("btype%i", self->btypeid);

    return PyUnicode_FromString("t");
//    return PyUnicodeUCS2_FromString("t");
}


pvt PyTypeObject PyBTypeCls = {
    PyVarObject_HEAD_INIT(0, 0)
    .tp_name = "jones.BType",
    .tp_doc = PyDoc_STR("A bones type"),
    .tp_basicsize = sizeof(PyBType),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = PyBType_create,
    .tp_dealloc = (destructor) PyBType_trash,
    .tp_members = PyBType_members,
    .tp_methods = PyBType_methods,
    .tp_str = (reprfunc) PyBType__str__,
};


#endif  // SRC_JONES_PYBTYPE_C