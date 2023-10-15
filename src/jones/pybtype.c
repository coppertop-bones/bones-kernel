#ifndef JONES_PY_BTYPE_C
#define JONES_PY_BTYPE_C "jones/py_btype.c"

#include "_jones.h"
#include "../../include/bk/tm.h"


// OPEN: for performance could keep track of instances and use ref counting rather than btype creation to answer a btype

pvt PyObject * PyBType_create(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    struct PyBType *self = (struct PyBType *) type->tp_alloc(type, 0);
    return (PyObject *) self;
}

pvt void PyBType_trash(struct PyBType *self) {
    Py_TYPE(self)->tp_free((PyObject *) self);
}

pvt PyMemberDef PyBType_members[] = {
    {"id", Py_T_UINT, offsetof(struct PyBType, btypeId), 0, "bones type id"},
    {0}
};

pvt PyMethodDef PyBType_methods[] = {
    {0}
};

pvt PyTypeObject PyBTypeCls = {
    PyVarObject_HEAD_INIT(0, 0)
    .tp_name = "jones.BType",
    .tp_doc = PyDoc_STR("A bones type"),
    .tp_basicsize = sizeof(struct PyBType),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = PyBType_create,
    .tp_dealloc = (destructor) PyBType_trash,
    .tp_members = PyBType_members,
    .tp_methods = PyBType_methods,
};


#endif  // JONES_PY_BTYPE_C