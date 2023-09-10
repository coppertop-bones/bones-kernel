#ifndef JONES_KERNEL_C
#define JONES_KERNEL_C "jones/kernel.c"

#include "python.h"
#include "../bk/kernel.c"


struct PyKernel {
    PyObject_HEAD
    struct K *pKernel;
};


pvt PyMemberDef PyKernel_members[] = {
//        {"id", T_UINT, offsetof(struct PyKernel, btid), 0, "id (u32)"},
        {0}
};


pvt PyMethodDef PyKernel_methods[] = {
        {0}
};


pvt void PyKernel_dealloc(struct PyKernel *self) {
    // tear down kernel and release all os memory!
    int res = K_shutdown(self->pKernel);
    free(self->pKernel);
    Py_TYPE(self)->tp_free((PyObject *) self);
}


pvt PyObject * PyKernel_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    struct PyKernel *self = (struct PyKernel *) type->tp_alloc(type, 0);
    struct K *k = (struct K *) malloc(sizeof(struct K));
    int res = K_init(k);
    return (PyObject *) self;
}


//pvt int PyKernel_init(struct PyKernel *self, PyObject *args, PyObject *kwds) {
//    static char *kwlist[] = {"id", 0};
//    int btid;
//    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|i", kwlist, &btid)) return -1;
//    self->btid = btid;
//    return 0;
//}


static PyTypeObject PyKernelCls = {
        PyVarObject_HEAD_INIT(0, 0)
        .tp_name = "jones.Kernel",
        .tp_doc = PyDoc_STR("TBC"),
        .tp_basicsize = sizeof(struct PyKernel),
        .tp_itemsize = 0,
        .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
        .tp_new = PyKernel_new,
//        .tp_init = (initproc) PyKernel_init,
        .tp_dealloc = (destructor) PyKernel_dealloc,
        .tp_members = PyKernel_members,
        .tp_methods = PyKernel_methods,
};


#endif  // JONES_KERNEL_C