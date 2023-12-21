#ifndef JONES_KERNEL_C
#define JONES_KERNEL_C "jones/kernel.c"


#include "jones.h"
#include "../bk/mm.c"
#include "../bk/sm.c"
#include "../bk/em.c"
#include "../bk/tm.c"
#include "../bk/tp.c"
#include "../bk/kernel.c"
#include "lib/pyutils.h"



// ---------------------------------------------------------------------------------------------------------------------
// PySM
// ---------------------------------------------------------------------------------------------------------------------

pvt PyObject * PySM_symid(struct PySM *self, PyObject **args, Py_ssize_t nargs) {
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyUnicode_Check(args[0]) || (PyUnicode_KIND(args[0]) != PyUnicode_1BYTE_KIND)) {
        return PyErr_Format(PyExc_TypeError, "name must be utf8");
    }
    char *name = (char *) PyUnicode_1BYTE_DATA(args[0]);
    return PyLong_FromLong(sm_id(self->sm, name));
}

pvt PyObject * PySM_name(struct PySM *self, PyObject **args, Py_ssize_t nargs) {
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyLong_Check(args[0])) {
        return PyErr_Format(PyExc_TypeError, "symid must be int");
    }
    long id = PyLong_AsLong(args[0]);
    if (id == SM_NA_SYM || id >= self->sm->next_symid) {
        return PyErr_Format(PyExc_ValueError, "symid out of range");
    }
    return PyUnicode_FromString(sm_name(self->sm, id));
}

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

// ---------------------------------------------------------------------------------------------------------------------
// btype: (name:str) -> btype + exception
// ---------------------------------------------------------------------------------------------------------------------
pvt PyObject * PyTM_btype(struct PyTM *self, PyObject **args, Py_ssize_t nargs) {
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyUnicode_Check(args[0]) || (PyUnicode_KIND(args[0]) != PyUnicode_1BYTE_KIND)) {
        return PyErr_Format(PyExc_TypeError, "name must be utf8");
    }
    char *name = (char *) PyUnicode_1BYTE_DATA(args[0]);
    btypeid_t btypeid = tm_btypeid(self->tm, name);
    if (!btypeid) {
        return PyErr_Format(PyExc_TypeError, "'%s' is not a btype", name);
    }
    struct PyBType *answer = (struct PyBType *) ((&PyBTypeCls)->tp_alloc(&PyBTypeCls, 0));
    answer->btypeid = btypeid;
    return (PyObject *) answer;
}

// ---------------------------------------------------------------------------------------------------------------------
// exists: (name:str) -> boolean
// ---------------------------------------------------------------------------------------------------------------------
pvt PyObject * PyTM_exists(struct PyTM *self, PyObject **args, Py_ssize_t nargs) {
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyUnicode_Check(args[0]) || (PyUnicode_KIND(args[0]) != PyUnicode_1BYTE_KIND)) {
        return PyErr_Format(PyExc_TypeError, "name must be utf8");
    }
    char *name = (char *) PyUnicode_1BYTE_DATA(args[0]);
    return PyBool_FromLong(tm_btypeid(self->tm, name));
}

// ---------------------------------------------------------------------------------------------------------------------
// exclusiveNominal: (str, int) -> btype + exception
// ---------------------------------------------------------------------------------------------------------------------
pvt PyObject * PyTM_exclusiveNominal(struct PyTM *self, PyObject **args, Py_ssize_t nargs) {
    if (nargs != 2) return jErrWrongNumberOfArgs(FN_NAME, 2, nargs);
    if (!PyUnicode_Check(args[0]) || (PyUnicode_KIND(args[0]) != PyUnicode_1BYTE_KIND)) return PyErr_Format(PyExc_TypeError, "name must be utf8");
    if (!PyLong_Check(args[1])) return PyErr_Format(PyExc_TypeError, "exclusionCategory must be an integer");
    btexclusioncat_t excl = PyLong_AsLong(args[1]);
    if (excl != btmemory &&
        excl != btuser1 &&
        excl != btuser2 &&
        excl != btuser3 &&
        excl != btuser4 &&
        excl != btuser5 &&
        excl != btuser6 &&
        excl != btuser7) {
        PyErr_SetString(PyExc_TypeError, "invalid exclusion category");
        return 0;
    }
    char *name = (char *) PyUnicode_1BYTE_DATA(args[0]);
    btypeid_t btypeid = tm_exclnominal(self->tm, name, excl);
    if (btypeid) {
        struct PyBType *answer = (struct PyBType *) ((&PyBTypeCls)->tp_alloc(&PyBTypeCls, 0));
        answer->btypeid = btypeid;
        return (PyObject *) answer;
    } else {
        return PyErr_Format(PyExc_TypeError, "The name '%s' is already is taken by another type", name);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
// intersection: (btype1, btype2, ...) -> btype + exception
// ---------------------------------------------------------------------------------------------------------------------
pvt PyObject * PyTM_intersection(struct PyTM *self, PyObject **args, Py_ssize_t nargs) {
    BK_TP tp;  Buckets *buckets;  BucketsCheckpoint cp;
    if (nargs == 0) return PyErr_Format(PyExc_TypeError, "Must provide at least one type");
    if (nargs == 1) {
        if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "arg1 is not a BType");
        return args[0];
    }
    checkpointBuckets((buckets = self->tm->buckets), &cp);

    // create a type list of the correct length
    btypeid_t *typelist = allocInBuckets(buckets, ((1 + nargs) * sizeof(btypeid_t)), alignof(btypeid_t));
    typelist[0] = (btypeid_t) nargs;
    for (int i=0; i < nargs; i++) {
        if (!PyObject_IsInstance(args[i], (PyObject *) &PyBTypeCls)) {
            resetToCheckpoint(buckets, &cp);
            return PyErr_Format(PyExc_TypeError, "arg%i is not a BType", i + 1);
        }
        typelist[i+1] = ((struct PyBType *) args[i])->btypeid;
    }

    btypeid_t btypeid = tm_inter(self->tm, typelist);

    if (btypeid) {
        struct PyBType *answer = (struct PyBType *) ((&PyBTypeCls)->tp_alloc(&PyBTypeCls, 0));
        answer->btypeid = btypeid;
        resetToCheckpoint(buckets, &cp);
        return (PyObject *) answer;
    } else {
        TP_init(&tp, 0, buckets);
        PyErr_Format(PyExc_TypeError, "There are exclusion conflicts within (%s)", tm_pp_typelist(self->tm, &tp, typelist).cs);
        resetToCheckpoint(buckets, &cp);
        return 0;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
// name: (btype) -> str + exception
// ---------------------------------------------------------------------------------------------------------------------
pvt PyObject * PyTM_name(struct PyTM *self, PyObject **args, Py_ssize_t nargs) {
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "btype is not a BType");
    // OPEN: what to do if there is no name (use t123?) - 0 means invalid type?
    char *name = tm_name(self->tm, ((struct PyBType *) args[0])->btypeid);
    if (name ==0) Py_RETURN_NONE;
    return PyUnicode_FromString(name);
}

// ---------------------------------------------------------------------------------------------------------------------
// nameAs: (btype, str) -> btype + exception
// ---------------------------------------------------------------------------------------------------------------------
pvt PyObject * PyTM_nameAs(struct PyTM *self, PyObject **args, Py_ssize_t nargs) {
    if (nargs != 2) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "btype is not a BType");
    if (!PyUnicode_Check(args[1]) || (PyUnicode_KIND(args[1]) != PyUnicode_1BYTE_KIND)) return PyErr_Format(PyExc_TypeError, "name must be utf8");

    struct PyBType *btype = (struct PyBType *) args[0];
    char *name = (char *) PyUnicode_1BYTE_DATA(args[1]);
    btypeid_t btypeid = tm_name_as(self->tm, btype->btypeid, name);
    return btypeid ? args[0] : PyErr_Format(PyExc_ValueError, "Name already in use");
}

// ---------------------------------------------------------------------------------------------------------------------
// nominal: (str) -> btype + exception
// ---------------------------------------------------------------------------------------------------------------------
pvt PyObject * PyTM_nominal(struct PyTM *self, PyObject **args, Py_ssize_t nargs) {
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyUnicode_Check(args[0]) || (PyUnicode_KIND(args[0]) != PyUnicode_1BYTE_KIND)) {
        PyErr_SetString(PyExc_TypeError, "name must be utf8");
        return 0;
    }
    char *name = (char *) PyUnicode_1BYTE_DATA(args[0]);
    btypeid_t btypeid = tm_nominal(self->tm, name);
    if (btypeid) {
        struct PyBType *answer = (struct PyBType *) ((&PyBTypeCls)->tp_alloc(&PyBTypeCls, 0));
        answer->btypeid = btypeid;
        return (PyObject *) answer;
    } else
        return PyErr_Format(PyExc_TypeError, "name is taken by another type");  // OPEN: better error
}

// ---------------------------------------------------------------------------------------------------------------------
// struct: (str, btype1, str, btype2...) -> btype + exception
// ---------------------------------------------------------------------------------------------------------------------
pvt PyObject * PyTM_struct(struct PyTM *self, PyObject **args, Py_ssize_t nargs) {
    // create a typelist and symlist of the correct length
    // call sm_struct which will return a btypeid
    // free the typelist and symlist
    // return btype
    Py_RETURN_NONE;
}

// ---------------------------------------------------------------------------------------------------------------------
// tuple: (btype1, btype2, ...) -> btype + exception
// ---------------------------------------------------------------------------------------------------------------------
pvt PyObject * PyTM_tuple(struct PyTM *self, PyObject **args, Py_ssize_t nargs) {
    // is a tuple of 1 element the same type as the element?
    // no
    // 1) tuple unpacking should be clear e.g. (A): fn() is not the same as A: fn()
    // 2) tuples can be indexed e.g. fn()[1]

    // create a type list of the correct length
    // call sm_tuple which will return a btypeid
    // free the typelist
    // return btype
    Py_RETURN_NONE;
}

// ---------------------------------------------------------------------------------------------------------------------
// union: (btype1, btype2, ...) -> btype
// ---------------------------------------------------------------------------------------------------------------------
pvt PyObject * PyTM_union(struct PyTM *self, PyObject **args, Py_ssize_t nargs) {
    BK_TP tp;  Buckets *buckets;  BucketsCheckpoint cp;
    if (nargs == 0) return PyErr_Format(PyExc_TypeError, "Must provide at least one type");
    if (nargs == 1) {
        if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "arg1 is not a BType");
        return args[0];
    }
    checkpointBuckets((buckets = self->tm->buckets), &cp);

    // create a type list of the correct length
    btypeid_t *typelist = allocInBuckets(buckets, ((1 + nargs) * sizeof(btypeid_t)), alignof(btypeid_t));
    typelist[0] = (btypeid_t) nargs;
    for (int i=0; i < nargs; i++) {
        if (!PyObject_IsInstance(args[i], (PyObject *) &PyBTypeCls)) {
            resetToCheckpoint(buckets, &cp);
            return PyErr_Format(PyExc_TypeError, "arg%i is not a BType", i + 1);
        }
        typelist[i+1] = ((struct PyBType *) args[i])->btypeid;
    }

    btypeid_t btypeid = tm_union(self->tm, typelist);

    if (btypeid) {
        struct PyBType *answer = (struct PyBType *) ((&PyBTypeCls)->tp_alloc(&PyBTypeCls, 0));
        answer->btypeid = btypeid;
        resetToCheckpoint(buckets, &cp);
        return (PyObject *) answer;
    } else {
        TP_init(&tp, 0, buckets);
        PyErr_Format(PyExc_TypeError, "There are exclusion conflicts within (%s)", tm_pp_typelist(self->tm, &tp, typelist).cs);
        resetToCheckpoint(buckets, &cp);
        return 0;
    }
}



// ---------------------------------------------------------------------------------------------------------------------

pvt PyMethodDef PyTM_methods[] = {
    {"btype",               (PyCFunction) PyTM_btype, METH_FASTCALL, "btype(name)\n\nanswers the btype called 'name' or None"},
    {"exclusiveNominal",    (PyCFunction) PyTM_exclusiveNominal, METH_FASTCALL,
        "exclusiveNominal(t)\n\n"
        "answers the exclusive nominal called 'name', creating it if it doesn't exist, or raises an error if the "
        "name is used by another type"
    },
    {"exists",              (PyCFunction) PyTM_exists, METH_FASTCALL,
        "exists(name)\n\n"
        "answers if the type with <name> exists or not"
    },
    {"intersection",        (PyCFunction) PyTM_intersection, METH_FASTCALL,
        "intersection(t1, t2, ...)\n\n"
        "answers the intersection t1 & t2 & ..."
    },
    {"name",                (PyCFunction) PyTM_name, METH_FASTCALL,
        "name(t)\n\n"
        "answers the name of the type if it exists else throws an error"
    },
    {"nameAs",              (PyCFunction) PyTM_nameAs, METH_FASTCALL,
        "nameAs(t, name)\n\n"
        "Names a type, raising an error if <name> has already been taken"},
    {"nominal",             (PyCFunction) PyTM_nominal, METH_FASTCALL,
        "nominal(name)\n\n"
        "answers the nominal called <name>, creating it if it doesn't exist, or raising an error if "
        "<name> is used by another type"
    },
    {"struct",              (PyCFunction) PyTM_struct, METH_FASTCALL,
            "struct(f1, t1, f2, t2, ...)\n\n"
            "answers the struct {f1:t1, f2:t2, ...}"
    },
    {"tuple",               (PyCFunction) PyTM_tuple, METH_FASTCALL,
            "tuple(t1, t2, ...)\n\n"
            "answers the tuple (t1, t2, ...)"
    },
    {"union",               (PyCFunction) PyTM_union, METH_FASTCALL,
            "union(t1, t2, ...)\n\n"
            "answers the union t1 + t2 + ..."
    },
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
    BK_MM *mm = MM_create();
    Buckets *buckets = mm->malloc(sizeof(Buckets));
    initBuckets(buckets, BUCKETS_CHUNK_SIZE);
    self->kernel = K_create(mm, buckets);
    self->pySM = (PyObject *) ((&PySMCls)->tp_alloc(&PySMCls, 0));
    ((struct PySM *) self->pySM)->sm = self->kernel->sm;
    self->pyEM = (PyObject *) ((&PyEMCls)->tp_alloc(&PyEMCls, 0));
    ((struct PyEM *) self->pyEM)->em = self->kernel->em;
    self->pyTM = (PyObject *) ((&PyTMCls)->tp_alloc(&PyTMCls, 0));
    ((struct PyTM *) self->pyTM)->tm = self->kernel->tm;
    return (PyObject *) self;
}

pvt void PyKernel_trash(struct PyKernel *self) {
    BK_MM *mm = self->kernel->mm;
    freeBuckets(self->kernel->buckets->first_bucket);
    i32 res = K_trash(self->kernel);
    if (res) PP(error, "%s: K_trash failed", FN_NAME);
    res = MM_trash(mm);
    if (res) PP(error, "%s: MM_trash failed", FN_NAME);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

pvt PyMemberDef PyKernel_members[] = {
    {"sm", Py_T_OBJECT_EX, offsetof(struct PyKernel, pySM), Py_READONLY, "symbol manager"},
    {"em", Py_T_OBJECT_EX, offsetof(struct PyKernel, pyEM), Py_READONLY, "enum manager"},
    {"tm", Py_T_OBJECT_EX, offsetof(struct PyKernel, pyTM), Py_READONLY, "type manager"},
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