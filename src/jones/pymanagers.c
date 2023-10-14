#ifndef JONES_KERNEL_C
#define JONES_KERNEL_C "jones/kernel.c"

#include "Python.h"
#include "_jones.h"
#include "../bk/mm.c"
#include "../bk/sm.c"
#include "../bk/em.c"
#include "../bk/tm.c"
#include "../bk/kernel.c"
#include "_utils.c"


// ---------------------------------------------------------------------------------------------------------------------
// PySM
// ---------------------------------------------------------------------------------------------------------------------

//pvt PyObject * PySM_create(PyTypeObject *type, PyObject *args, PyObject *kwds) {
//    struct PySM *self = (struct PySM *) type->tp_alloc(type, 0);
//    struct MM *mm = MM_create();
//    self->sm = SM_create(mm);
//    return (PyObject *) self;
//}

//pvt void PySM_trash(struct PySM *self) {
//    SM_trash(self->sm);
//    Py_TYPE(self)->tp_free((PyObject *) self);
//}

pvt PyObject * PySM_sym(struct PySM *self, PyObject *const *args, Py_ssize_t nargs) {
    if (nargs != 1) return _raiseWrongNumberOfArgs(__FUNCTION__, 1, nargs);
    if (!PyUnicode_Check(args[0]) || (PyUnicode_KIND(args[0]) != PyUnicode_1BYTE_KIND)) {
        PyErr_SetString(PyExc_TypeError, "name must be utf8");
        return 0;
    }
    const char *name = (const char *) PyUnicode_1BYTE_DATA(args[0]);
    return PyLong_FromLong(sm_id(self->sm, name));
}

pvt PyObject * PySM_name(struct PySM *self, PyObject *const *args, Py_ssize_t nargs) {
    if (nargs != 1) return _raiseWrongNumberOfArgs(__FUNCTION__, 1, nargs);
    if (!PyLong_Check(args[0])) {
        PyErr_SetString(PyExc_TypeError, "symid must be int");
        return 0;
    }
    uint id = PyLong_AsLong(args[0]);
    if (id < 1 || id >= self->sm->next_sym_id) {
        PyErr_SetString(PyExc_ValueError, "symid out of range");
        return 0;
    }
    return PyUnicode_FromString(sm_name(self->sm, id));
}



//pvt PyObject * _execShell(PyObject *mod, PyObject *args) {
//    const char *command;  int ret;
//    if (!PyArg_ParseTuple(args, "s", &command)) return 0;
//    ret = system(command);
//    if (ret < 0) {
//        PyErr_SetString(PyJonesError, "System command failed");
//        return 0;
//    }
//    return PyLong_FromLong(ret);
//}



//pvt int PySM_init(struct PySM *self, PyObject *args, PyObject *kwds) {
//    static char *kwlist[] = {"id", 0};
//    int btid;
//    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|i", kwlist, &btid)) return -1;
//    self->btid = btid;
//    return 0;
//}

pvt PyMemberDef PySM_members[] = {
//        {"id", T_UINT, offsetof(struct PyBType, btype), 0, "id (u32)"},
//        {"name", T_OBJECT, offsetof(struct Fn, name), READONLY, "function name"},
//        {"num_tbc", T_UBYTE, offsetof(struct Partial, num_tbc), READONLY, "number of argument to be confirmed"},
//        {"num_args", T_UBYTE, offsetof(PyVarObject, ob_size), READONLY, "total number of arguments"},
    {0}
};

pvt PyMethodDef PySM_methods[] = {
//    {"__array_ufunc__", (PyCFunction) _Common__array_ufunc__, METH_VARARGS | METH_KEYWORDS, "__array_ufunc__"},
        {"sym", (PyCFunction) PySM_sym, METH_FASTCALL, "sym(name)\n\nanswers the symid for name"},
        {"name", (PyCFunction) PySM_name, METH_FASTCALL, "name(symid)\n\nanswers the name for symid"},
//    {"name", (PyCFunction) PyPlay_name, METH_NOARGS, "Return the name, combining the first and last name"},
    {0}
};



pvt PyGetSetDef PySM_get_set[] = {
//    {"o_tbc", (getter) Partial_o_tbc, 0, "offsets of missing arguments", 0},
//    {"args", (getter) Partial_args, 0, "arguments thus far", 0},
    {0}
};

//pvt PyNumberMethods PySM_number_methods = {
//    .nb_rshift = (binaryfunc) _nullary_nb_rshift,
//};


pvt PyTypeObject PySMCls = {
    PyVarObject_HEAD_INIT(0, 0)
//    .tp_base = &PFnCls,
    .tp_name = "jones.SM",
    .tp_doc = PyDoc_STR("TBC"),
    .tp_basicsize = sizeof(struct PySM),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
//    .tp_new = PySM_create,
//    .tp_init = (initproc) PySM_init,
//    .tp_dealloc = (destructor) PySM_trash,
    .tp_members = PySM_members,
    .tp_methods = PySM_methods,
//    .tp_getset = PySM_get_set,
//    .tp_call = (ternaryfunc) _Partial__call__,
//    .tp_as_number = (PyNumberMethods*) &PySM_number_methods,
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


//pvt PyObject * PyEM_create(PyTypeObject *type, PyObject *args, PyObject *kwds) {
//    struct PyEM *self = (struct PyEM *) type->tp_alloc(type, 0);
//    self->em = EM_create(mm);
//    return (PyObject *) self;
//}
//
//
//pvt void PyEM_trash(struct PyEM *self) {
//    // tear down kernel and release all os memory!
//    EM_trash(self->em);
//    Py_TYPE(self)->tp_free((PyObject *) self);
//}


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
//        .tp_new = PyEM_create,
//    .tp_init = (initproc) PyEM_init,
//        .tp_dealloc = (destructor) PyEM_trash,
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


//pvt PyObject * PyTM_create(PyTypeObject *type, PyObject *args, PyObject *kwds) {
//    struct PyTM *self = (struct PyTM *) type->tp_alloc(type, 0);
//    self->tm = TM_create();
//    return (PyObject *) self;
//}
//
//
//pvt void PyTM_trash(struct PyTM *self) {
//    // tear down kernel and release all os memory!
//    TM_trash(self->tm);
//    Py_TYPE(self)->tp_free((PyObject *) self);
//}


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
//    .tp_new = PyTM_create,
//    .tp_init = (initproc) PyTM_init,
//    .tp_dealloc = (destructor) PyTM_trash,
    .tp_members = PyTM_members,
    .tp_methods = PyTM_methods,
};



// ---------------------------------------------------------------------------------------------------------------------
// PyKernel
// ---------------------------------------------------------------------------------------------------------------------

pvt PyMemberDef PyKernel_members[] = {
    {"sm", T_OBJECT, offsetof(struct PyKernel, pySM), READONLY, "symbol manager"},
    {"em", T_OBJECT, offsetof(struct PyKernel, pyEM), READONLY, "enum manager"},
    {"tm", T_OBJECT, offsetof(struct PyKernel, pyTM), READONLY, "type manager"},
    {0}
};

pvt PyMethodDef PyKernel_methods[] = {
    {0}
};


pvt PyObject * PyKernel_create(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    // OPEN: assert no args or kwargs are passed
    struct PyKernel *self = (struct PyKernel *) type->tp_alloc(type, 0);
    struct MM *mm = MM_create();
    self->kernel = K_create(mm);
    self->pySM = (struct PySM *) ((&PySMCls)->tp_alloc(&PySMCls, 0));
    ((struct PySM *) self->pySM)->sm = self->kernel->sm;
    self->pyEM = (struct PyEM *) ((&PyEMCls)->tp_alloc(&PyEMCls, 0));
    ((struct PyEM *) self->pyEM)->em = self->kernel->em;
    self->pyTM = (struct PyTM *) ((&PyTMCls)->tp_alloc(&PyTMCls, 0));
    ((struct PyTM *) self->pyTM)->tm = self->kernel->em;
    return (PyObject *) self;
}


pvt void PyKernel_trash(struct PyKernel *self) {
    struct MM *mm = self->kernel->mm;
    int res = K_trash(self->kernel);
    res = MM_trash(mm);
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