#ifndef JONES_PY_BTYPE_C
#define JONES_PY_BTYPE_C "jones/py_btype.c"

#include "python.h"
#include "_common.h"
#include "../../include/bk/tm.h"



pvt PyMemberDef PyBType_members[] = {
        {"id", T_UINT, offsetof(struct PyBType, btype), 0, "id (u32)"},
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
//    int btype;
//    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|i", kwlist, &btype)) return -1;
//    self->btype = btype;
//    return 0;
//}


pvt PyTypeObject PyBTypeCls = {
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