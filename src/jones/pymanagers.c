#ifndef JONES_KERNEL_C
#define JONES_KERNEL_C "jones/kernel.c"

#include "python.h"
#include "_jones.h"
#include "../bk/sm.c"
#include "../bk/em.c"
#include "../bk/tm.c"
#include "../bk/kernel.c"


// ---------------------------------------------------------------------------------------------------------------------
// PySM
// ---------------------------------------------------------------------------------------------------------------------

pvt PyMemberDef PySM_members[] = {
//    {"id", T_UINT, offsetof(struct PySM, btid), 0, "id (u32)"},
        {0}
};


pvt PyMethodDef PySM_methods[] = {
        {0}
};


pvt PyObject * PySM_create(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    struct PySM *self = (struct PySM *) type->tp_alloc(type, 0);
    self->pSm = sm_create();
    return (PyObject *) self;
}


pvt void PySM_trash(struct PySM *self) {
    // tear down kernel and release all os memory!
    sm_trash(self->pSm);
    Py_TYPE(self)->tp_free((PyObject *) self);
}


//pvt int PySM_init(struct PySM *self, PyObject *args, PyObject *kwds) {
//    static char *kwlist[] = {"id", 0};
//    int btid;
//    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|i", kwlist, &btid)) return -1;
//    self->btid = btid;
//    return 0;
//}


pvt PyTypeObject PySMCls = {
    PyVarObject_HEAD_INIT(0, 0)
    .tp_name = "jones.Kernel",
    .tp_doc = PyDoc_STR("TBC"),
    .tp_basicsize = sizeof(struct PySM),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = PySM_create,
//    .tp_init = (initproc) PySM_init,
    .tp_dealloc = (destructor) PySM_trash,
    .tp_members = PySM_members,
    .tp_methods = PySM_methods,
};


// ---------------------------------------------------------------------------------------------------------------------
// PyEM
// ---------------------------------------------------------------------------------------------------------------------

pvt PyMemberDef PyEM_members[] = {
//    {"id", T_UINT, offsetof(struct PyEM, btid), 0, "id (u32)"},
    {0}
};


pvt PyMethodDef PyEM_methods[] = {
    {0}
};


pvt PyObject * PyEM_create(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    struct PyEM *self = (struct PyEM *) type->tp_alloc(type, 0);
    self->pEm = em_create();
    return (PyObject *) self;
}


pvt void PyEM_trash(struct PyEM *self) {
    // tear down kernel and release all os memory!
    em_trash(self->pEm);
    Py_TYPE(self)->tp_free((PyObject *) self);
}


//pvt int PyEM_init(struct PyEM *self, PyObject *args, PyObject *kwds) {
//    static char *kwlist[] = {"id", 0};
//    int btid;
//    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|i", kwlist, &btid)) return -1;
//    self->btid = btid;
//    return 0;
//}


pvt PyTypeObject PyEMCls = {
        PyVarObject_HEAD_INIT(0, 0)
        .tp_name = "jones.Kernel",
        .tp_doc = PyDoc_STR("TBC"),
        .tp_basicsize = sizeof(struct PyEM),
        .tp_itemsize = 0,
        .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
        .tp_new = PyEM_create,
//    .tp_init = (initproc) PyEM_init,
        .tp_dealloc = (destructor) PyEM_trash,
        .tp_members = PyEM_members,
        .tp_methods = PyEM_methods,
};


// ---------------------------------------------------------------------------------------------------------------------
// PyTM
// ---------------------------------------------------------------------------------------------------------------------

pvt PyMemberDef PyTM_members[] = {
//    {"id", T_UINT, offsetof(struct PyTM, btid), 0, "id (u32)"},
    {0}
};


pvt PyMethodDef PyTM_methods[] = {
    {0}
};


pvt PyObject * PyTM_create(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    struct PyTM *self = (struct PyTM *) type->tp_alloc(type, 0);
    self->pTm = tm_create();
    return (PyObject *) self;
}


pvt void PyTM_trash(struct PyTM *self) {
    // tear down kernel and release all os memory!
    tm_trash(self->pTm);
    Py_TYPE(self)->tp_free((PyObject *) self);
}


//pvt int PyTM_init(struct PyTM *self, PyObject *args, PyObject *kwds) {
//    static char *kwlist[] = {"id", 0};
//    int btid;
//    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|i", kwlist, &btid)) return -1;
//    self->btid = btid;
//    return 0;
//}


pvt PyTypeObject PyTMCls = {
    PyVarObject_HEAD_INIT(0, 0)
    .tp_name = "jones.Kernel",
    .tp_doc = PyDoc_STR("TBC"),
    .tp_basicsize = sizeof(struct PyTM),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = PyTM_create,
//    .tp_init = (initproc) PyTM_init,
    .tp_dealloc = (destructor) PyTM_trash,
    .tp_members = PyTM_members,
    .tp_methods = PyTM_methods,
};



// ---------------------------------------------------------------------------------------------------------------------
// PyKernel
// ---------------------------------------------------------------------------------------------------------------------

pvt PyMemberDef PyKernel_members[] = {
//    {"id", T_UINT, offsetof(struct PyKernel, btid), 0, "id (u32)"},
    {0}
};


pvt PyMethodDef PyKernel_methods[] = {
    {0}
};


pvt PyObject * PyKernel_create(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    struct PyKernel *self = (struct PyKernel *) type->tp_alloc(type, 0);
    struct K *k = (struct K *) malloc(sizeof(struct K));
    int res = K_init(k);
    return (PyObject *) self;
}


pvt void PyKernel_trash(struct PyKernel *self) {
    // tear down kernel and release all os memory!
    int res = K_trash(self->pKernel);
    Py_TYPE(self)->tp_free((PyObject *) self);
}


//pvt int PyKernel_init(struct PyKernel *self, PyObject *args, PyObject *kwds) {
//    static char *kwlist[] = {"id", 0};
//    int btid;
//    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|i", kwlist, &btid)) return -1;
//    self->btid = btid;
//    return 0;
//}


pvt PyTypeObject PyKernelCls = {
    PyVarObject_HEAD_INIT(0, 0)
    .tp_name = "jones.Kernel",
    .tp_doc = PyDoc_STR("TBC"),
    .tp_basicsize = sizeof(struct PyKernel),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = PyKernel_create,
//    .tp_init = (initproc) PyKernel_init,
    .tp_dealloc = (destructor) PyKernel_trash,
    .tp_members = PyKernel_members,
    .tp_methods = PyKernel_methods,
};


#endif  // JONES_KERNEL_C