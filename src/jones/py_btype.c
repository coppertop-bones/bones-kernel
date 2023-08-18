#ifndef JONES_PYBTYPE_C
#define JONES_PYBTYPE_C "jones/PyTypes.c"

#include "pipe_structs.c"


struct PyBType {
    PyObject_HEAD
    TypeNum TN1;
    TypeNum TN2;
};


static PyMemberDef PyBType_members[] = {
        {"TN1", T_USHORT, offsetof(struct PyBType, TN1), 0, "custom number"},
        {"TN2", T_USHORT, offsetof(struct PyBType, TN2), 0, "custom number"},
        {0}
};


static PyMethodDef PyBType_methods[] = {
        {0}
};


pvt void PyBType_dealloc(struct PyBType *self) {
    Py_TYPE(self)->tp_free((PyObject *) self);
}


pvt PyObject * PyBType_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    struct PyBType *self = (struct PyBType *) type->tp_alloc(type, 0);
    if (self != 0) {
        self->TN1 = 0;
        self->TN2 = 0;
    }
    return (PyObject *) self;
}


pvt int PyBType_init(struct PyBType *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {"id", 0};
    int id;
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|i", kwlist, &id)) return -1;
    self->TN1 = id & 0xFFFF;
    self->TN2 = (id & 0xFFFF0000) >> 16;
    return 0;
}


static PyTypeObject PyBTypeCls = {
        PyVarObject_HEAD_INIT(0, 0)
                .tp_name = "jones.BType",
        .tp_doc = PyDoc_STR("a BType to play with"),
        .tp_basicsize = sizeof(struct PyBType),
        .tp_itemsize = 0,
        .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
        .tp_new = PyBType_new,
        .tp_init = (initproc) PyBType_init,
        .tp_dealloc = (destructor) PyBType_dealloc,
        .tp_members = PyBType_members,
        .tp_methods = PyBType_methods,
};


#endif  // JONES_PYBTYPE_C