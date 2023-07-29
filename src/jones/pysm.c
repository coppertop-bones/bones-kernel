// ---------------------------------------------------------------------------------------------------------------------
// Copyright 2025 David Briant, https://github.com/coppertop-bones. Licensed under the Apache License, Version 2.0
//
// PYSM - PYTHON INTERFACE TO SYMBOL MANAGER
// ---------------------------------------------------------------------------------------------------------------------

#ifndef SRC_JONES_PYSM_C
#define SRC_JONES_PYSM_C "jones/pysm.c"


#include "lib/pyutils.h"
#include "../bk/sm.c"


// ---------------------------------------------------------------------------------------------------------------------
// PySM Methods
// ---------------------------------------------------------------------------------------------------------------------

pvt PyObject * PySM_symid(PySM *self, PyObject **args, Py_ssize_t nargs) {
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyUnicode_Check(args[0]) || (PyUnicode_KIND(args[0]) != PyUnicode_1BYTE_KIND)) return PyErr_Format(PyExc_TypeError, "name must be utf8");
    char *name = (char *) PyUnicode_AsUTF8(args[0]);
    return PyLong_FromLong(sm_id(self->sm, name));
}

pvt PyObject * PySM_name(PySM *self, PyObject **args, Py_ssize_t nargs) {
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyLong_Check(args[0])) return PyErr_Format(PyExc_TypeError, "symid must be int");
    long id = PyLong_AsLong(args[0]);
    if (id == SM_NA_SYM || id >= self->sm->next_symid) return PyErr_Format(PyExc_ValueError, "symid out of range");
    return PyUnicode_FromString(sm_name(self->sm, id));
}


// ---------------------------------------------------------------------------------------------------------------------
// PySM Class
// ---------------------------------------------------------------------------------------------------------------------

pvt PyMethodDef PySM_methods[] = {
    {"symid", (PyCFunction) PySM_symid, METH_FASTCALL, "sym(name)\n\nanswers the symid for name"},
    {"name", (PyCFunction) PySM_name, METH_FASTCALL, "name(symid)\n\nanswers the name for symid"},
    {0}
};

pvt PyGetSetDef PySM_get_set[] = {
//    {"o_tbc", (getter) Partial_o_tbc, 0, "offsets of missing arguments", 0},
//    {"args", (getter) Partial_args, 0, "arguments thus far", 0},
    {0}
};

pvt PyTypeObject PySMCls = {
    PyVarObject_HEAD_INIT(0, 0)
    .tp_name = "jones.SM",
    .tp_doc = PyDoc_STR("TBC"),
    .tp_basicsize = sizeof(PySM),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_methods = PySM_methods,
//    .tp_getset = PySM_get_set,
//    .tp_call = (ternaryfunc) _Partial__call__,
//    .tp_as_number = (PyNumberMethods*) &PySM_number_methods,
};



#endif  // SRC_JONES_PYSM_C