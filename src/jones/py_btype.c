#ifndef JONES_PY_BTYPE_C
#define JONES_PY_BTYPE_C "jones/py_btype.c"

#include "python.h"
#include "pipe_structs.c"


struct PyBType {
    PyObject_HEAD
    unsigned int id;
};


pvt PyMemberDef PyBType_members[] = {
        {"id", T_UINT, offsetof(struct PyBType, id), 0, "type id (u32)"},
        {0}
};


pvt PyMethodDef PyBType_methods[] = {
        {0}
};


pvt void PyBType_dealloc(struct PyBType *self) {
    Py_TYPE(self)->tp_free((PyObject *) self);
}


pvt PyObject * PyBType_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    struct PyBType *self = (struct PyBType *) type->tp_alloc(type, 0);
    return (PyObject *) self;
}


//pvt int PyBType_init(struct PyBType *self, PyObject *args, PyObject *kwds) {
//    static char *kwlist[] = {"id", 0};
//    int id;
//    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|i", kwlist, &id)) return -1;
//    self->id = id;
//    return 0;
//}


static PyTypeObject PyBTypeCls = {
        PyVarObject_HEAD_INIT(0, 0)
        .tp_name = "jones.BType",
        .tp_doc = PyDoc_STR("TBC"),
        .tp_basicsize = sizeof(struct PyBType),
        .tp_itemsize = 0,
        .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
        .tp_new = PyBType_new,
//        .tp_init = (initproc) PyBType_init,
        .tp_dealloc = (destructor) PyBType_dealloc,
        .tp_members = PyBType_members,
        .tp_methods = PyBType_methods,
};


#endif  // JONES_PY_BTYPE_C