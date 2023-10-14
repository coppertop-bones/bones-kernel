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

pvt PyMethodDef PySM_methods[] = {
    {"sym", (PyCFunction) PySM_sym, METH_FASTCALL, "sym(name)\n\nanswers the symid for name"},
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
//    .tp_base = &PFnCls,
    .tp_name = "jones.SM",
    .tp_doc = PyDoc_STR("TBC"),
    .tp_basicsize = sizeof(struct PySM),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_methods = PySM_methods,
//    .tp_getset = PySM_get_set,
//    .tp_call = (ternaryfunc) _Partial__call__,
//    .tp_as_number = (PyNumberMethods*) &PySM_number_methods,
};



// ---------------------------------------------------------------------------------------------------------------------
// PyEM
// ---------------------------------------------------------------------------------------------------------------------

pvt PyMethodDef PyEM_methods[] = {
    {0}
};

pvt PyTypeObject PyEMCls = {
    PyVarObject_HEAD_INIT(0, 0)
    .tp_name = "jones.EM",
    .tp_doc = PyDoc_STR("TBC"),
    .tp_basicsize = sizeof(struct PyEM),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_methods = PyEM_methods,
};



// ---------------------------------------------------------------------------------------------------------------------
// PyTM
// ---------------------------------------------------------------------------------------------------------------------

// btype: (name:str) -> btype + exception
pvt PyObject * PyTM_btype(struct PyTM *self, PyObject *const *args, Py_ssize_t nargs) {
    if (nargs != 1) return _raiseWrongNumberOfArgs(__FUNCTION__, 1, nargs);
    if (!PyUnicode_Check(args[0]) || (PyUnicode_KIND(args[0]) != PyUnicode_1BYTE_KIND)) {
        PyErr_SetString(PyExc_TypeError, "name must be utf8");
        return 0;
    }
    const char *name = (const char *) PyUnicode_1BYTE_DATA(args[0]);
    BTYPE_ID_T btype = tm_id(self->tm, name);
    if (btype) {
        struct PyBType *answer = (struct PyBType *) ((&PyBTypeCls)->tp_alloc(&PyBTypeCls, 0));
        answer->btype = btype;
        return (PyObject *) answer;
    } else {
        PyErr_SetString(PyExc_TypeError, "name is not a btype");  // OPEN: better error
        return 0;
    }
}

// exists: (name:str) -> boolean
pvt PyObject * PyTM_exists(struct PyTM *self, PyObject *const *args, Py_ssize_t nargs) {
    if (nargs != 1) return _raiseWrongNumberOfArgs(__FUNCTION__, 1, nargs);
    if (!PyUnicode_Check(args[0]) || (PyUnicode_KIND(args[0]) != PyUnicode_1BYTE_KIND)) {
        PyErr_SetString(PyExc_TypeError, "name must be utf8");
        return 0;
    }
    const char *name = (const char *) PyUnicode_1BYTE_DATA(args[0]);
    return PyBool_FromLong(tm_id(self->tm, name));
}

// intersection: (btype1, btype2, ...) -> btype + exception
pvt PyObject * PyTM_intersection(struct PyTM *self, PyObject *const *args, Py_ssize_t nargs) {
    Py_RETURN_NONE;
}

// name: (btype) -> str + exception
pvt PyObject * PyTM_name(struct PyTM *self, PyObject *const *args, Py_ssize_t nargs) {
    if (nargs != 1) return _raiseWrongNumberOfArgs(__FUNCTION__, 1, nargs);
    Py_RETURN_NONE;
}

// nameAs: (btype, str) -> btype + exception
pvt PyObject * PyTM_nameAs(struct PyTM *self, PyObject *const *args, Py_ssize_t nargs) {
    Py_RETURN_NONE;
}

// nominal: (str) -> btype + exception
pvt PyObject * PyTM_nominal(struct PyTM *self, PyObject *const *args, Py_ssize_t nargs) {
    if (nargs != 1) return _raiseWrongNumberOfArgs(__FUNCTION__, 1, nargs);
    if (!PyUnicode_Check(args[0]) || (PyUnicode_KIND(args[0]) != PyUnicode_1BYTE_KIND)) {
        PyErr_SetString(PyExc_TypeError, "name must be utf8");
        return 0;
    }
    const char *name = (const char *) PyUnicode_1BYTE_DATA(args[0]);
    BTYPE_ID_T btype = tm_nominal(self->tm, name);
    if (btype) {
        struct PyBType *answer = (struct PyBType *) ((&PyBTypeCls)->tp_alloc(&PyBTypeCls, 0));
        answer->btype = btype;
        return (PyObject *) answer;
    } else {
        PyErr_SetString(PyExc_TypeError, "name is taken by another type");  // OPEN: better error
        return 0;
    }
}

// setExplicit: (btype) -> res
pvt PyObject * PyTM_setExplicit(struct PyTM *self, PyObject *const *args, Py_ssize_t nargs) {
    Py_RETURN_NONE;
}

// struct: (str, btype1, str, btype2...) -> btype + exception
pvt PyObject * PyTM_struct(struct PyTM *self, PyObject *const *args, Py_ssize_t nargs) {
    Py_RETURN_NONE;
}

// tuple: (btype1, btype2, ...) -> btype + exception
pvt PyObject * PyTM_tuple(struct PyTM *self, PyObject *const *args, Py_ssize_t nargs) {
    Py_RETURN_NONE;
}

// union: (btype1, btype2, ...) -> btype + exception
pvt PyObject * PyTM_union(struct PyTM *self, PyObject *const *args, Py_ssize_t nargs) {
    Py_RETURN_NONE;
}



pvt PyMethodDef PyTM_methods[] = {
    {"btype", (PyCFunction) PyTM_btype, METH_FASTCALL, "btype(name)\n\nanswers the type called 'name' or None"},
    {"exists", (PyCFunction) PyTM_exists, METH_FASTCALL,
        "exists(name)\n\n"
        "answers if name exists or not"
    },
    {"intersection", (PyCFunction) PyTM_intersection, METH_FASTCALL,
        "intersection(t1, t2, ...)\n\n"
        "answers the intersection of t1, t2, ..."
    },
    {"name", (PyCFunction) PyTM_name, METH_FASTCALL,
        "name(t)\n\n"
        "answers the name of the type if it exists else throws an error"
    },
    {"nameAs", (PyCFunction) PyTM_nameAs, METH_FASTCALL,
        "nameAs(t, name)\n\n"
        "Names a type, raising an error if the name is already taken"},
    {"nominal", (PyCFunction) PyTM_nominal, METH_FASTCALL,
        "nominal(name)\n\n"
        "answers the nominal called 'name', creating it if it doesn't exist, or raises an error if the "
        "name is used by another type"
    },
    {"setExplicit", (PyCFunction) PyTM_setExplicit, METH_FASTCALL, "setExplicit(t)\n\nTBD"},
    {0}
};


pvt PyTypeObject PyTMCls = {
    PyVarObject_HEAD_INIT(0, 0)
    .tp_name = "jones.TM",
    .tp_doc = PyDoc_STR("TBC"),
    .tp_basicsize = sizeof(struct PyTM),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_methods = PyTM_methods,
};



// ---------------------------------------------------------------------------------------------------------------------
// PyKernel
// ---------------------------------------------------------------------------------------------------------------------

pvt PyObject * PyKernel_create(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    // OPEN: assert no args or kwargs are passed
    struct PyKernel *self = (struct PyKernel *) type->tp_alloc(type, 0);
    struct MM *mm = MM_create();
    self->kernel = K_create(mm);
    self->pySM = (PyObject *) ((&PySMCls)->tp_alloc(&PySMCls, 0));
    ((struct PySM *) self->pySM)->sm = self->kernel->sm;
    self->pyEM = (PyObject *) ((&PyEMCls)->tp_alloc(&PyEMCls, 0));
    ((struct PyEM *) self->pyEM)->em = self->kernel->em;
    self->pyTM = (PyObject *) ((&PyTMCls)->tp_alloc(&PyTMCls, 0));
    ((struct PyTM *) self->pyTM)->tm = self->kernel->tm;
    return (PyObject *) self;
}

pvt void PyKernel_trash(struct PyKernel *self) {
    struct MM *mm = self->kernel->mm;
    int res = K_trash(self->kernel);
    res = MM_trash(mm);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

pvt PyMemberDef PyKernel_members[] = {
    {"sm", T_OBJECT, offsetof(struct PyKernel, pySM), READONLY, "symbol manager"},
    {"em", T_OBJECT, offsetof(struct PyKernel, pyEM), READONLY, "enum manager"},
    {"tm", T_OBJECT, offsetof(struct PyKernel, pyTM), READONLY, "type manager"},
    {0}
};

pvt PyMethodDef PyKernel_methods[] = {
    {0}
};

pvt PyTypeObject PyKernelCls = {
    PyVarObject_HEAD_INIT(0, 0)
    .tp_name = "jones.Kernel",
    .tp_doc = PyDoc_STR("TBC"),
    .tp_basicsize = sizeof(struct PyKernel),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = PyKernel_create,
    .tp_dealloc = (destructor) PyKernel_trash,
    .tp_members = PyKernel_members,
    .tp_methods = PyKernel_methods,
};



#endif  // JONES_KERNEL_C