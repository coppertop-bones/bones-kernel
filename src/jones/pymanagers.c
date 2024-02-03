#ifndef JONES_KERNEL_C
#define JONES_KERNEL_C "jones/kernel.c"


#include "jones.h"
#include "../bk/mm.c"
#include "../bk/sm.c"
#include "../bk/em.c"
#include "../bk/tm.c"
#include "../bk/tp.c"
#include "../bk/k.c"
#include "lib/pyutils.h"




//OPEN: add BTypeError (as a subclass of PyTypeError?)

// ---------------------------------------------------------------------------------------------------------------------
// PySM
// ---------------------------------------------------------------------------------------------------------------------

pvt PyObject * PySM_symid(struct PySM *self, PyObject **args, Py_ssize_t nargs) {
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyUnicode_Check(args[0]) || (PyUnicode_KIND(args[0]) != PyUnicode_1BYTE_KIND)) return PyErr_Format(PyExc_TypeError, "name must be utf8");
    char *name = (char *) PyUnicode_1BYTE_DATA(args[0]);
    return PyLong_FromLong(sm_id(self->sm, name));
}

pvt PyObject * PySM_name(struct PySM *self, PyObject **args, Py_ssize_t nargs) {
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyLong_Check(args[0])) return PyErr_Format(PyExc_TypeError, "symid must be int");
    long id = PyLong_AsLong(args[0]);
    if (id == SM_NA_SYM || id >= self->sm->next_symid) return PyErr_Format(PyExc_ValueError, "symid out of range");
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
// btype: (name:str) -> PyBType + PyException
// ---------------------------------------------------------------------------------------------------------------------
pvt PyObject * PyTM_btype(struct PyTM *self, PyObject **args, Py_ssize_t nargs) {
    // answer a new BType given a name or a TypeError if there is no btype with that name
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyUnicode_Check(args[0]) || (PyUnicode_KIND(args[0]) != PyUnicode_1BYTE_KIND)) return PyErr_Format(PyExc_TypeError, "name must be utf8");
    char *name = (char *) PyUnicode_1BYTE_DATA(args[0]);
    btypeid_t btypeid = tm_btypeid(self->tm, name);
    if (!btypeid) return PyErr_Format(PyExc_TypeError, "'%s' is not a btype", name);
    PyBType *answer = (PyBType *) ((&PyBTypeCls)->tp_alloc(&PyBTypeCls, 0));
    answer->btypeid = btypeid;
    return (PyObject *) answer;
}

// ---------------------------------------------------------------------------------------------------------------------
// exists: (name:str) -> PyBoolean
// ---------------------------------------------------------------------------------------------------------------------
pvt PyObject * PyTM_exists(struct PyTM *self, PyObject **args, Py_ssize_t nargs) {
    // answer True if there is a btype with the given name, else False
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyUnicode_Check(args[0]) || (PyUnicode_KIND(args[0]) != PyUnicode_1BYTE_KIND)) return PyErr_Format(PyExc_TypeError, "name must be utf8");
    char *name = (char *) PyUnicode_1BYTE_DATA(args[0]);
    return PyBool_FromLong(tm_btypeid(self->tm, name));
}

// ---------------------------------------------------------------------------------------------------------------------
// exclusionCat: (name:str) -> PyLong + PyException
// ---------------------------------------------------------------------------------------------------------------------
pvt PyObject * PyTM_exclusionCat(struct PyTM *self, PyObject **args, Py_ssize_t nargs) {
    // answer the exclusion category for the given name, creating if necessary
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyUnicode_Check(args[0]) || (PyUnicode_KIND(args[0]) != PyUnicode_1BYTE_KIND)) return PyErr_Format(PyExc_TypeError, "name must be utf8");
    char *name = (char *) PyUnicode_1BYTE_DATA(args[0]);
    btexclusioncat_t res = tm_exclusion_cat(self->tm, name, 0);
    if (res == 0) return PyErr_Format(PyExc_TypeError, "Couldn't create exclusion category for name = \"%s\"", name);
    return PyLong_FromLong(res);
}

// ---------------------------------------------------------------------------------------------------------------------
// exclusiveNominal: (name:str, exclusionCat:int) -> PyBType + PyException
// ---------------------------------------------------------------------------------------------------------------------
pvt PyObject * PyTM_exclusiveNominal(struct PyTM *self, PyObject **args, Py_ssize_t nargs) {
    // answer a new nominal with the given exclusion
    if (nargs != 2) return jErrWrongNumberOfArgs(FN_NAME, 2, nargs);
    if (!PyUnicode_Check(args[0]) || (PyUnicode_KIND(args[0]) != PyUnicode_1BYTE_KIND)) return PyErr_Format(PyExc_TypeError, "name must be utf8");
    if (!PyLong_Check(args[1])) return PyErr_Format(PyExc_TypeError, "exclusionCategory must be an integer");
    btexclusioncat_t excl = PyLong_AsLong(args[1]);
    if (excl != btememory &&
        excl != bteptr &&
        excl != bteccy &&
        excl != bteuser1 &&
        excl != bteuser2 &&
        excl != bteuser3 &&
        excl != bteuser4 &&
        excl != bteuser5) {
        return PyErr_Format(PyExc_TypeError, "invalid exclusion category");
    }
    // OPEN: add size for btememory
    char *name = (char *) PyUnicode_1BYTE_DATA(args[0]);
    btypesize_t sz = 0;
    btypeid_t btypeid = tm_exclnominal(self->tm, name, excl, sz, 0);
    if (btypeid) {
        PyBType *answer = (PyBType *) ((&PyBTypeCls)->tp_alloc(&PyBTypeCls, 0));
        answer->btypeid = btypeid;
        return (PyObject *) answer;
    } else
        return PyErr_Format(PyExc_TypeError, "The name '%s' is already is taken by another type", name);
}

// ---------------------------------------------------------------------------------------------------------------------
// fn: ((btype1, btype2, ...), tRet) -> PyBType + PyException
// ---------------------------------------------------------------------------------------------------------------------
pvt PyObject * PyTM_fn(struct PyTM *self, PyObject **args, Py_ssize_t nargs) {
    BK_TP tp;  Buckets *buckets;  BucketsCheckpoint cp;  btypeid_t *tl, tArgs, tRet;  PyObject *tup, *e;  int n;

    // answer the fn type corresponding to tArgs and tRet
    if (nargs != 2) return PyErr_Format(PyExc_TypeError, "Must provide tArgs and tRet");
    checkpointBuckets((buckets = self->tm->buckets), &cp);

    // check tArgs
    if (PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) {
        tArgs = ((PyBType *) args[0])->btypeid;
        if (tm_bmetatypeid(self->tm, tArgs) != bmttup) {
            resetToCheckpoint(buckets, &cp);
            return PyErr_Format(PyExc_TypeError, "tArgs is not a BType nor a tuple of BTypes");
        }
    }
    else if (PyObject_IsInstance(args[0], (PyObject *) &PyTuple_Type)) {
        tup = args[0];
        n = PyTuple_Size(tup);
        tl = allocInBuckets(buckets, ((1 + n) * sizeof(btypeid_t)), bk_alignof(btypeid_t));
        tl[0] = (btypeid_t) n;
        for (int i=0; i < n; i++) {
            e = PyTuple_GetItem(tup, i);
            if (!PyObject_IsInstance(e, (PyObject *) &PyBTypeCls)) {
                resetToCheckpoint(buckets, &cp);
                return PyErr_Format(PyExc_TypeError, "element %i of tuple is not a BType", i);
            }
            tl[i+1] = ((PyBType *) e)->btypeid;
        }
        tArgs = tm_tuple(self->tm, tl, 0);
        if (!tArgs) {
            resetToCheckpoint(buckets, &cp);
            return PyErr_Format(PyExc_TypeError, "Error creating tArgs from Python tuple of BTypes");
        }
    }
    else {
        resetToCheckpoint(buckets, &cp);
        return PyErr_Format(PyExc_TypeError, "tArgs is not a tuple BType nor a Python tuple of BTypes");
    }

    // check tRet
    if (!PyObject_IsInstance(args[1], (PyObject *) &PyBTypeCls)) {
        resetToCheckpoint(buckets, &cp);
        return PyErr_Format(PyExc_TypeError, "tRet is not a BType");
    }
    tRet = ((PyBType *) args[1])->btypeid;

    btypeid_t btypeid = tm_fn(self->tm, tArgs, tRet, 0);

    if (btypeid) {
        PyBType *answer = (PyBType *) ((&PyBTypeCls)->tp_alloc(&PyBTypeCls, 0));
        answer->btypeid = btypeid;
        resetToCheckpoint(buckets, &cp);
        return (PyObject *) answer;
    } else {
        TP_init(&tp, 0, buckets);
        PyErr_Format(PyExc_TypeError, "Error creating (%s) -> %s", tm_s8_typelist(self->tm, &tp, tl).cs, tm_s8(self->tm, &tp, tRet).cs);
        resetToCheckpoint(buckets, &cp);
        return 0;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
// fnArgT: (tFn) -> tArgs + PyException
// ---------------------------------------------------------------------------------------------------------------------
pvt PyObject * PyTM_fnTArgs(struct PyTM *self, PyObject **args, Py_ssize_t nargs) {
    // answer tArgs of the given tFn
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "btype is not a BType");
    btypeid_t tFn = ((PyBType *) args[0])->btypeid;
    if (tm_bmetatypeid(self->tm, tFn) != bmtfnc) return PyErr_Format(PyExc_TypeError, "btype is not a fn type");
    PyBType *answer = (PyBType *) ((&PyBTypeCls)->tp_alloc(&PyBTypeCls, 0));
    answer->btypeid = tm_Fn(self->tm, tFn).tArgs;
    return (PyObject *) answer;
}

// ---------------------------------------------------------------------------------------------------------------------
// fnRetT: (tFn) -> tRet + PyException
// ---------------------------------------------------------------------------------------------------------------------
pvt PyObject * PyTM_fnTRet(struct PyTM *self, PyObject **args, Py_ssize_t nargs) {
    // answer tRet of the given tFn
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "btype is not a BType");
    btypeid_t tFn = ((PyBType *) args[0])->btypeid;
    if (tm_bmetatypeid(self->tm, tFn) != bmtfnc) return PyErr_Format(PyExc_TypeError, "btype is not a fn type");
    PyBType *answer = (PyBType *) ((&PyBTypeCls)->tp_alloc(&PyBTypeCls, 0));
    answer->btypeid = tm_Fn(self->tm, tFn).tRet;
    return (PyObject *) answer;
}

// ---------------------------------------------------------------------------------------------------------------------
// intersection: (btype1, btype2, ...) -> PyBType + PyException
// ---------------------------------------------------------------------------------------------------------------------
pvt PyObject * PyTM_intersection(struct PyTM *self, PyObject **args, Py_ssize_t nargs) {
    // answer a new intersection of the given btypes
    BK_TP tp;  Buckets *buckets;  BucketsCheckpoint cp;  btypeid_t *tl;
    if (nargs == 0) return PyErr_Format(PyExc_TypeError, "Must provide at least one type");
    if (nargs == 1) {
        if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "arg is not a BType");
        return Py_NewRef(args[0]);
    }
    checkpointBuckets((buckets = self->tm->buckets), &cp);

    // create a type list of the correct length
    tl = allocInBuckets(buckets, ((1 + nargs) * sizeof(btypeid_t)), bk_alignof(btypeid_t));
    tl[0] = (btypeid_t) nargs;
    for (int i=1; i <= nargs; i++) {
        if (!PyObject_IsInstance(args[i-1], (PyObject *) &PyBTypeCls)) {
            resetToCheckpoint(buckets, &cp);
            return PyErr_Format(PyExc_TypeError, "arg%i is not a BType", i);
        }
        tl[i] = ((PyBType *) args[i-1])->btypeid;
    }

    btypeid_t btypeid = tm_inter(self->tm, tl, 0);

    if (btypeid) {
        PyBType *answer = (PyBType *) ((&PyBTypeCls)->tp_alloc(&PyBTypeCls, 0));
        answer->btypeid = btypeid;
        resetToCheckpoint(buckets, &cp);
        return (PyObject *) answer;
    } else {
        TP_init(&tp, 0, buckets);
        PyErr_Format(PyExc_TypeError, "There are exclusion conflicts within (%s)", tm_s8_typelist(self->tm, &tp, tl).cs);
        resetToCheckpoint(buckets, &cp);
        return 0;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
// intersectionTl: (btype) -> (PyBype1, ...) + PyException
// ---------------------------------------------------------------------------------------------------------------------
pvt PyObject * PyTM_intersectionTl(struct PyTM *self, PyObject **args, Py_ssize_t nargs) {
    // answer a new PyTuple of new PyBTypes which is the type list of the given intersection btype
    PyBType *btype;
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "btype is not a BType");
    btypeid_t *tl = tm_inter_tl(self->tm, ((PyBType *) args[0])->btypeid);
    if (tl == 0) return PyErr_Format(PyExc_TypeError, "btype is not an intersection type");
    PyObject *answer = PyTuple_New(tl[0]);
    for (btypeid_t i = 1; i <= tl[0]; i++) {
        btype = (PyBType *) ((&PyBTypeCls)->tp_alloc(&PyBTypeCls, 0));
        btype->btypeid = tl[i];
        PyTuple_SET_ITEM(answer, i - 1, btype);
    }
    return answer;
}

// ---------------------------------------------------------------------------------------------------------------------
// map: (tK, tV) -> tMap + PyException
// ---------------------------------------------------------------------------------------------------------------------
pvt PyObject * PyTM_map(struct PyTM *self, PyObject **args, Py_ssize_t nargs) {
    BK_TP tp;  Buckets *buckets;  BucketsCheckpoint cp;  btypeid_t tK, tV, tMap;

    // answer the tMap corresponding to tK and tV
    if (nargs != 2) return PyErr_Format(PyExc_TypeError, "Must provide tK and tV");
    checkpointBuckets((buckets = self->tm->buckets), &cp);

    // check tK
    if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) {
        resetToCheckpoint(buckets, &cp);
        return PyErr_Format(PyExc_TypeError, "tK is not a BType");
    }
    tK = ((PyBType *) args[0])->btypeid;

    // check tV
    if (!PyObject_IsInstance(args[1], (PyObject *) &PyBTypeCls)) {
        resetToCheckpoint(buckets, &cp);
        return PyErr_Format(PyExc_TypeError, "tV is not a BType");
    }
    tV = ((PyBType *) args[1])->btypeid;

    tMap = tm_map(self->tm, tK, tV, 0);

    if (tMap) {
        PyBType *answer = (PyBType *) ((&PyBTypeCls)->tp_alloc(&PyBTypeCls, 0));
        answer->btypeid = tMap;
        resetToCheckpoint(buckets, &cp);
        return (PyObject *) answer;
    } else {
        TP_init(&tp, 0, buckets);
        PyErr_Format(PyExc_TypeError, "Error creating map for %s -> %s", tm_s8(self->tm, &tp, tK).cs, tm_s8(self->tm, &tp, tV).cs);
        resetToCheckpoint(buckets, &cp);
        return 0;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
// mapTK: (tMap) -> tK + PyException
// ---------------------------------------------------------------------------------------------------------------------
pvt PyObject * PyTM_mapTK(struct PyTM *self, PyObject **args, Py_ssize_t nargs) {
    // answer tK of the given tMap
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "btype is not a BType");
    btypeid_t tMap = ((PyBType *) args[0])->btypeid;
    if (tm_bmetatypeid(self->tm, tMap) != bmtmap) return PyErr_Format(PyExc_TypeError, "btype is not a map type");
    PyBType *answer = (PyBType *) ((&PyBTypeCls)->tp_alloc(&PyBTypeCls, 0));
    answer->btypeid = tm_Map(self->tm, tMap).tK;
    return (PyObject *) answer;
}

// ---------------------------------------------------------------------------------------------------------------------
// mapTV: (tMap) -> tV + PyException
// ---------------------------------------------------------------------------------------------------------------------
pvt PyObject * PyTM_mapTV(struct PyTM *self, PyObject **args, Py_ssize_t nargs) {
    // answer tV of the given tMap
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "btype is not a BType");
    btypeid_t tMap = ((PyBType *) args[0])->btypeid;
    if (tm_bmetatypeid(self->tm, tMap) != bmtmap) return PyErr_Format(PyExc_TypeError, "btype is not a map type");
    PyBType *answer = (PyBType *) ((&PyBTypeCls)->tp_alloc(&PyBTypeCls, 0));
    answer->btypeid = tm_Map(self->tm, tMap).tV;
    return (PyObject *) answer;
}

// ---------------------------------------------------------------------------------------------------------------------
// name: (btype) -> PyStr + PyException
// ---------------------------------------------------------------------------------------------------------------------
pvt PyObject * PyTM_name(struct PyTM *self, PyObject **args, Py_ssize_t nargs) {
    // answer the name of the given btype
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "btype is not a BType");
    // OPEN: what to do if there is no name (use t123?) - 0 means invalid type?
    char *name = tm_name(self->tm, ((PyBType *) args[0])->btypeid);
    if (name == 0) Py_RETURN_NONE;
    return PyUnicode_FromString(name);
}

// ---------------------------------------------------------------------------------------------------------------------
// nameAs: (btype, name:str) -> PyBType + PyException
// ---------------------------------------------------------------------------------------------------------------------
pvt PyObject * PyTM_nameAs(struct PyTM *self, PyObject **args, Py_ssize_t nargs) {
    // answer a new reference to the given btype, naming it in the process if it hasn't already got one
    if (nargs != 2) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "btype is not a BType");
    if (!PyUnicode_Check(args[1]) || (PyUnicode_KIND(args[1]) != PyUnicode_1BYTE_KIND)) return PyErr_Format(PyExc_TypeError, "name must be utf8");

    PyBType *btype = (PyBType *) args[0];
    char *name = (char *) PyUnicode_1BYTE_DATA(args[1]);
    btypeid_t btypeid = tm_name_as(self->tm, btype->btypeid, name);
    return btypeid ? Py_NewRef(args[0]) : PyErr_Format(PyExc_ValueError, "Name already in use");
}

// ---------------------------------------------------------------------------------------------------------------------
// nominal: (name:str) -> PyBType + PyException
// ---------------------------------------------------------------------------------------------------------------------
pvt PyObject * PyTM_nominal(struct PyTM *self, PyObject **args, Py_ssize_t nargs) {
    // answer a new nominal with the given name, or an exception if already taken
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyUnicode_Check(args[0]) || (PyUnicode_KIND(args[0]) != PyUnicode_1BYTE_KIND)) return PyErr_Format(PyExc_TypeError, "name must be utf8");
    char *name = (char *) PyUnicode_1BYTE_DATA(args[0]);
    btypeid_t btypeid = tm_nominal(self->tm, name, 0);
    if (btypeid) {
        PyBType *answer = (PyBType *) ((&PyBTypeCls)->tp_alloc(&PyBTypeCls, 0));
        answer->btypeid = btypeid;
        return (PyObject *) answer;
    } else
        return PyErr_Format(PyExc_TypeError, "name is taken by another type");  // OPEN: better error
}

// ---------------------------------------------------------------------------------------------------------------------
// schemavar: (name:str) -> tSchemavar + PyException
// ---------------------------------------------------------------------------------------------------------------------
pvt PyObject * PyTM_schemavar(struct PyTM *self, PyObject **args, Py_ssize_t nargs) {
    // answer a new schema variable with the given name, or an exception if already taken
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyUnicode_Check(args[0]) || (PyUnicode_KIND(args[0]) != PyUnicode_1BYTE_KIND)) return PyErr_Format(PyExc_TypeError, "name must be utf8");
    char *name = (char *) PyUnicode_1BYTE_DATA(args[0]);
    btypeid_t btypeid = tm_schemavar(self->tm, name, 0);
    if (btypeid) {
        PyBType *answer = (PyBType *) ((&PyBTypeCls)->tp_alloc(&PyBTypeCls, 0));
        answer->btypeid = btypeid;
        return (PyObject *) answer;
    } else
        return PyErr_Format(PyExc_TypeError, "name is taken by another type");  // OPEN: better error
}

// ---------------------------------------------------------------------------------------------------------------------
// seq: (constained:btype) -> PyBType + PyException
// ---------------------------------------------------------------------------------------------------------------------
pvt PyObject * PyTM_seq(struct PyTM *self, PyObject **args, Py_ssize_t nargs) {
    BK_TP tp;  Buckets *buckets;  BucketsCheckpoint cp;  PyBType *answer;

    // answer a new sequence for the given contained btype
    if (nargs != 1) return PyErr_Format(PyExc_TypeError, "Must provide just one BType");
    checkpointBuckets((buckets = self->tm->buckets), &cp);

    if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) {
        resetToCheckpoint(buckets, &cp);
        return PyErr_Format(PyExc_TypeError, "arg1 is not a BType");
    }

    btypeid_t btypeid = tm_seq(self->tm, ((PyBType *) args[0])->btypeid, 0);

    if (btypeid) {
        answer = (PyBType *) ((&PyBTypeCls)->tp_alloc(&PyBTypeCls, 0));
        answer->btypeid = btypeid;
        resetToCheckpoint(buckets, &cp);
        return (PyObject *) answer;
    } else {
        TP_init(&tp, 0, buckets);
        PyErr_Format(PyExc_TypeError, "Undertermined error");
        resetToCheckpoint(buckets, &cp);
        return 0;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
// seqT: (seq:btype) -> (PyBType1,...) + PyException
// ---------------------------------------------------------------------------------------------------------------------
pvt PyObject * PyTM_seqT(struct PyTM *self, PyObject **args, Py_ssize_t nargs) {
    btypeid_t btypeid;  PyBType *answer;

    // answer a new PyBType which is the type list of the given union
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "btype is not a BType");
    btypeid = tm_seq_t(self->tm, ((PyBType *) args[0])->btypeid);
    if (btypeid == 0) return PyErr_Format(PyExc_TypeError, "btype is not a union type");
    answer = (PyBType *) ((&PyBTypeCls)->tp_alloc(&PyBTypeCls, 0));
    answer->btypeid = btypeid;
    return (PyObject *) answer;
}

// ---------------------------------------------------------------------------------------------------------------------
// struct: (str, btype1, str, btype2...) -> PyBType + PyException
// ---------------------------------------------------------------------------------------------------------------------
pvt PyObject * PyTM_struct(struct PyTM *self, PyObject **args, Py_ssize_t nargs) {
    // create a typelist and symlist of the correct length
    // call sm_struct which will return a btypeid
    // free the typelist and symlist
    // return btype
    Py_RETURN_NONE;
}

// ---------------------------------------------------------------------------------------------------------------------
// structTl: btype -> (PyBType1, ...) + PyException
// ---------------------------------------------------------------------------------------------------------------------
pvt PyObject * PyTM_structTl(struct PyTM *self, PyObject **args, Py_ssize_t nargs) {
    Py_RETURN_NONE;
}

// ---------------------------------------------------------------------------------------------------------------------
// tuple: (btype1, btype2, ...) -> PyBType + PyException
// ---------------------------------------------------------------------------------------------------------------------
pvt PyObject * PyTM_tuple(struct PyTM *self, PyObject **args, Py_ssize_t nargs) {
    // answers a new tuple type of the given btypes
    BK_TP tp;  Buckets *buckets;  BucketsCheckpoint cp;  btypeid_t *tl;
    if (nargs == 0) return PyErr_Format(PyExc_TypeError, "Must provide at least one type");
    checkpointBuckets((buckets = self->tm->buckets), &cp);

    // create a type list of the correct length
    tl = allocInBuckets(buckets, ((1 + nargs) * sizeof(btypeid_t)), bk_alignof(btypeid_t));
    tl[0] = (btypeid_t) nargs;
    for (int i=1; i <= nargs; i++) {
        if (!PyObject_IsInstance(args[i-1], (PyObject *) &PyBTypeCls)) {
            resetToCheckpoint(buckets, &cp);
            return PyErr_Format(PyExc_TypeError, "arg%i is not a BType", i);
        }
        tl[i] = ((PyBType *) args[i-1])->btypeid;
    }

    btypeid_t btypeid = tm_tuple(self->tm, tl, 0);

    if (btypeid) {
        PyBType *answer = (PyBType *) ((&PyBTypeCls)->tp_alloc(&PyBTypeCls, 0));
        answer->btypeid = btypeid;
        resetToCheckpoint(buckets, &cp);
        return (PyObject *) answer;
    } else {
        TP_init(&tp, 0, buckets);
        PyErr_Format(PyExc_TypeError, "Error creating tuple (%s)", tm_s8_typelist(self->tm, &tp, tl).cs);
        resetToCheckpoint(buckets, &cp);
        return 0;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
// tupleTl: btype -> (PyBType1, ...) + PyException
// ---------------------------------------------------------------------------------------------------------------------
pvt PyObject * PyTM_tupleTl(struct PyTM *self, PyObject **args, Py_ssize_t nargs) {
    PyBType *btype;  btypeid_t *tl;  PyObject *answer;
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "btype is not a BType");
    tl = tm_tuple_tl(self->tm, ((PyBType *) args[0])->btypeid);
    if (tl == 0) return PyErr_Format(PyExc_TypeError, "btype is not a tuple type");
    answer = PyTuple_New(tl[0]);
    for (btypeid_t i = 0; i < tl[0]; i++) {
        btype = (PyBType *) ((&PyBTypeCls)->tp_alloc(&PyBTypeCls, 0));
        btype->btypeid = tl[i+1];
        PyTuple_SET_ITEM(answer, i, btype);
    }
    return answer;
}

// ---------------------------------------------------------------------------------------------------------------------
// union: (btype1, btype2, ...) -> PyBType + PyException
// ---------------------------------------------------------------------------------------------------------------------
pvt PyObject * PyTM_union(struct PyTM *self, PyObject **args, Py_ssize_t nargs) {
    // answer a new union of the given btypes
    BK_TP tp;  Buckets *buckets;  BucketsCheckpoint cp;
    if (nargs == 0) return PyErr_Format(PyExc_TypeError, "Must provide at least one type");
    if (nargs == 1) {
        if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "arg is not a BType");
        return Py_NewRef(args[0]);
    }
    checkpointBuckets((buckets = self->tm->buckets), &cp);

    // create a type list of the correct length
    btypeid_t *tl = allocInBuckets(buckets, ((1 + nargs) * sizeof(btypeid_t)), bk_alignof(btypeid_t));
    tl[0] = (btypeid_t) nargs;
    for (int i=1; i <= nargs; i++) {
        if (!PyObject_IsInstance(args[i-1], (PyObject *) &PyBTypeCls)) {
            resetToCheckpoint(buckets, &cp);
            return PyErr_Format(PyExc_TypeError, "arg%i is not a BType", i);
        }
        tl[i] = ((PyBType *) args[i-1])->btypeid;
    }

    btypeid_t btypeid = tm_union(self->tm, tl, 0);

    if (btypeid) {
        PyBType *answer = (PyBType *) ((&PyBTypeCls)->tp_alloc(&PyBTypeCls, 0));
        answer->btypeid = btypeid;
        resetToCheckpoint(buckets, &cp);
        return (PyObject *) answer;
    } else {
        TP_init(&tp, 0, buckets);
        PyErr_Format(PyExc_TypeError, "There are conflicts within (%s)", tm_s8_typelist(self->tm, &tp, tl).cs);
        resetToCheckpoint(buckets, &cp);
        return 0;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
// unionTl: (union:btype) -> (PyBType1,...) + PyException
// ---------------------------------------------------------------------------------------------------------------------
pvt PyObject * PyTM_unionTl(struct PyTM *self, PyObject **args, Py_ssize_t nargs) {
    // answer a new PyTuple of new PyBTypes which is the type list of the given union
    PyBType *btype;  btypeid_t *tl;  PyObject *answer;
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "btype is not a BType");
    tl = tm_union_tl(self->tm, ((PyBType *) args[0])->btypeid);
    if (tl == 0) return PyErr_Format(PyExc_TypeError, "btype is not a union type");
    answer = PyTuple_New(tl[0]);
    for (btypeid_t i=1; i <= tl[0]; i++) {
        btype = (PyBType *) ((&PyBTypeCls)->tp_alloc(&PyBTypeCls, 0));
        btype->btypeid = tl[i];
        PyTuple_SET_ITEM(answer, i - 1, btype);
    }
    return answer;
}


// ---------------------------------------------------------------------------------------------------------------------

pvt PyMethodDef PyTM_methods[] = {
    {"btype",               (PyCFunction) PyTM_btype, METH_FASTCALL, "btype(name)\n\nanswers the btype called 'name' or None"},
    {"exclusionCat",        (PyCFunction) PyTM_exclusionCat, METH_FASTCALL,
        "exclusionCat(name, exclusionCat)\n\n"
        "Answers the exclusionCat with name, creating it if it doesn't exist, or throws an error if it already does and exclusionCat is different."
    },
    {"exclusiveNominal",    (PyCFunction) PyTM_exclusiveNominal, METH_FASTCALL,
        "exclusiveNominal(t)\n\n"
        "Answers the exclusive nominal called 'name', creating it if it doesn't exist, or raises an error if the name is used by another type."
    },
    {"exists",              (PyCFunction) PyTM_exists, METH_FASTCALL,
        "exists(name)\n\n"
        "Answers if the type with <name> exists or not."
    },
    {"fn",                  (PyCFunction) PyTM_fn, METH_FASTCALL,
        "fn((t1, t2, ...), tRet)\n\n"
        "Answers the fn(t1 & t2 & ...) -> tRet type."
    },
    {"fnTArgs",              (PyCFunction) PyTM_fnTArgs, METH_FASTCALL,
        "fnTArgs(tFn)\n\n"
        "Answers a tuple of the argument types of a function."
    },
    {"fnTRet",              (PyCFunction) PyTM_fnTRet, METH_FASTCALL,
        "fnRetT(tFn)\n\n"
        "Answers tRet of a function."
    },
    {"intersection",        (PyCFunction) PyTM_intersection, METH_FASTCALL,
        "intersection(t1, t2, ...)\n\n"
        "Answers the intersection t1 & t2 & ... type."
    },
    {"intersectionTl",      (PyCFunction) PyTM_intersectionTl, METH_FASTCALL,
        "intersectionTl(t)\n\n"
        "Answers a tuple of the types in the intersection t."
    },
    {"map",                 (PyCFunction) PyTM_map, METH_FASTCALL,
        "map(tK, tV)\n\n"
        "Answers the type of the map tK -> tV."
    },
    {"mapTK",               (PyCFunction) PyTM_mapTK, METH_FASTCALL,
        "mapTK(tMap)\n\n"
        "Answers tK of tMap."
    },
    {"mapTV",               (PyCFunction) PyTM_mapTV, METH_FASTCALL,
        "mapTV(tMap)\n\n"
        "Answers tV of tMap."
    },
    {"name",                (PyCFunction) PyTM_name, METH_FASTCALL,
        "name(t)\n\n"
        "Answers the name of the type if it exists else throws an error."
    },
    {"nameAs",              (PyCFunction) PyTM_nameAs, METH_FASTCALL,
        "nameAs(t, name)\n\n"
        "Names a type, raising an error if <name> has already been taken."},
    {"nominal",             (PyCFunction) PyTM_nominal, METH_FASTCALL,
        "nominal(name)\n\n"
        "Answers the nominal called <name>, creating it if it doesn't exist, or raising an error if "
        "<name> is used by another type."
    },
    {"schemavar",           (PyCFunction) PyTM_schemavar, METH_FASTCALL,
        "schemavar(name)\n\n"
        "Answers the schema variable called <name>, creating it if it doesn't exist, or raising an error if "
        "<name> is used by another type."
    },
    {"seq",                 (PyCFunction) PyTM_seq, METH_FASTCALL,
        "seq(t)\n\n"
        "Answers the sequence of t type."
    },
    {"seqT",                (PyCFunction) PyTM_seqT, METH_FASTCALL,
        "seqT(t)\n\n"
        "Answers the contained type."
    },
    {"struct",              (PyCFunction) PyTM_struct, METH_FASTCALL,
        "struct(f1, t1, f2, t2, ...)\n\n"
        "Answers the struct {f1:t1, f2:t2, ...} type."
    },
    {"structTl",             (PyCFunction) PyTM_structTl, METH_FASTCALL,
        "structTl(t)\n\n"
        "Answers a tuple of the names and types in the struct t."
    },
    {"tuple",               (PyCFunction) PyTM_tuple, METH_FASTCALL,
        "tuple(t1, t2, ...)\n\n"
        "Answers the tuple (t1, t2, ...) type."
    },
    {"tupleTl",             (PyCFunction) PyTM_tupleTl, METH_FASTCALL,
        "tupleTl(t)\n\n"
        "Answers a tuple of the types in the tuple t."
    },
    {"union",               (PyCFunction) PyTM_union, METH_FASTCALL,
        "union(t1, t2, ...)\n\n"
        "Answers the union t1 + t2 + ... type."
    },
    {"unionTl",             (PyCFunction) PyTM_unionTl, METH_FASTCALL,
        "unionTl(t)\n\n"
        "Answers a tuple of the types in the union t."
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