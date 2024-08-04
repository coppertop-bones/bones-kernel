// ---------------------------------------------------------------------------------------------------------------------
// PYBTYPE - PYTHON BTYPE
// ---------------------------------------------------------------------------------------------------------------------

#ifndef SRC_JONES_PYBTYPE_C
#define SRC_JONES_PYBTYPE_C "jones/pybtype.c"

#include "../../include/jones/jones.h"



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
};


#endif  // SRC_JONES_PYBTYPE_C