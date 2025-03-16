// ---------------------------------------------------------------------------------------------------------------------
// PYTM - PYTHON INTERFACE TO TYPE MANAGER
// ---------------------------------------------------------------------------------------------------------------------

#ifndef SRC_JONES_PYTM_C
#define SRC_JONES_PYTM_C "jones/pytm.c"


#include <stdlib.h>
#include "../../include/jones/jones.h"
#include "../../include/jones/lib/pyutils.h"
#include "../bk/mm.c"
#include "../bk/sm.c"
#include "../bk/em.c"
#include "../bk/tm.c"
#include "../bk/tp.c"


btypeid_t _num_btypes = 0;
PyObject * *_btypes = 0;        // OPEN: move this to the Python Kernel object?


// ---------------------------------------------------------------------------------------------------------------------
// PyBType cache

pvt PyObject * newPyBTypeRef(btypeid_t btypeid) {
    if (btypeid >= _num_btypes) {
        btypeid_t oldNum = _num_btypes;
        while (_num_btypes <= btypeid) _num_btypes += 1024;
        _btypes = realloc(_btypes, sizeof(PyBType *) * _num_btypes);
        memset(_btypes + oldNum, 0, sizeof(PyObject *) * (_num_btypes - oldNum));  // zero the new memory
//        PP(info, "_num_btypes: %i", _num_btypes);
    }
    if (_btypes[btypeid] == 0) {
        PyBType *new = (PyBType *) ((&PyBTypeCls)->tp_alloc(&PyBTypeCls, 0));
        new->btypeid = btypeid;
        _btypes[btypeid] = (PyObject *) new;
    }
    return Py_NewRef(_btypes[btypeid]);
}



// ---------------------------------------------------------------------------------------------------------------------
// PyTM
// ---------------------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------------------
// atom: (name:str) -> PyBType + PyException
// atom: (name:str, self:PyBType) -> PyBType + PyException

pvt PyObject * PyTM_atom(PyTM *pyTm, PyObject *const *args, Py_ssize_t nargs, PyObject *argnames) {
    // answer a new atom with the given name, or an exception if already taken
    char const *name;  btypeid_t btypeid, self = B_NEW, orthspcid = 0;  PyObject *pySelf = 0, *pyOrthspc = 0;

    __CHECK(nargs == 1, PyExc_TypeError, "atom(name, **kwargs) takes 1 arg but %i were given", nargs);
    if (!PyUnicode_Check(args[0]) || (PyUnicode_KIND(args[0]) != PyUnicode_1BYTE_KIND)) return PyErr_Format(PyExc_TypeError, "name must be utf8");

    if (argnames) {
        for (int i = 0; i < PyTuple_Size(argnames); i++) {
            __GET_KWARG("self", PyTuple_GET_ITEM(argnames, i), pySelf = args[i + nargs])
            __OR_GET_KWARG("orthspc", PyTuple_GET_ITEM(argnames, i), pyOrthspc = args[i + nargs])
            __ELSE_RAISE(PyExc_TypeError, "Unknown keyword argument \"%s\"", PyUnicode_AsUTF8(PyTuple_GET_ITEM(argnames, i)))
        }
        if (pySelf) {
            __CHECK(PyObject_IsInstance(pySelf, (PyObject *) &PyBTypeCls), PyExc_TypeError, "self must be a BType");
            self = ((PyBType *) pySelf)->btypeid;
        }
        if (pyOrthspc) {
            __CHECK(PyObject_IsInstance(pyOrthspc, (PyObject *) &PyBTypeCls), PyExc_TypeError, "orthspc must be a BType");
            orthspcid = ((PyBType *) pyOrthspc)->btypeid;
            __CHECK(pySelf, PyExc_TypeError, "self must be provided as well if orthspc is provided");
        }
    }
    name = (char *) PyUnicode_AsUTF8(args[0]);
    btypeid = tm_atom(pyTm->tm, self, name);
    if (orthspcid) pyTm->tm->orthspcid_by_btypeid[btypeid] = orthspcid;
    if (!btypeid) return PyErr_Format(PyBTypeError, "error calling tm_atom(...)");
    return newPyBTypeRef(btypeid);
}

// ---------------------------------------------------------------------------------------------------------------------
// bmetatypeid: (btype) -> PyLong + PyException

pvt PyObject * PyTM_bmetatypeid(PyTM *self, PyObject **args, Py_ssize_t nargs) {
    // answer the bmetatypeid of the given btype
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "btype is not a BType");
    // OPEN: what to do if there is no name (use t123?) - 0 means invalid type?
    long bmtid = (long) tm_bmetatypeid(self->tm, ((PyBType *) args[0])->btypeid);
    return PyLong_FromLong(bmtid);
}

// ---------------------------------------------------------------------------------------------------------------------
// exists: (name:str) -> PyBoolean

pvt PyObject * PyTM_exists(PyTM *self, PyObject **args, Py_ssize_t nargs) {
    // answer True if there is a btype with the given name, else False
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyUnicode_Check(args[0]) || (PyUnicode_KIND(args[0]) != PyUnicode_1BYTE_KIND)) return PyErr_Format(PyExc_TypeError, "name must be utf8");
    char *name = (char *) PyUnicode_AsUTF8(args[0]);
    return PyBool_FromLong(tm_btypeid(self->tm, name));
}

// ---------------------------------------------------------------------------------------------------------------------
// fn: ((btype1, btype2, ...), tRet) -> PyBType + PyException

pvt PyObject * PyTM_fn(PyTM *pyTm, PyObject **args, Py_ssize_t nargs, PyObject *argnames) {
    // answer the fn type corresponding to tArgs and tRet
    BK_TP tp;  Buckets *buckets;  BucketsCheckpoint cp;  btypeid_t *tl, tArgs, tRet, self = B_NEW;  int n;
    PyObject *tup, *e, *pySelf = 0;  TM_TLID_T tlid;

    if (nargs != 2) return PyErr_Format(PyExc_TypeError, "Must provide tArgs and tRet");

    if (argnames) {
        for (int i = 0; i < PyTuple_Size(argnames); i++) {
            __GET_KWARG("self", PyTuple_GET_ITEM(argnames, i), pySelf = args[i + nargs])
            __ELSE_RAISE(PyExc_TypeError, "Unknown keyword argument \"%s\"", PyUnicode_AsUTF8(PyTuple_GET_ITEM(argnames, i)))
        }
        if (pySelf) {
            __CHECK(PyObject_IsInstance(pySelf, (PyObject *) &PyBTypeCls), PyExc_TypeError, "self must be a BType");
            self = ((PyBType *) pySelf)->btypeid;
        }
    }

    checkpointBuckets((buckets = pyTm->tm->buckets), &cp);

    // check tArgs
    if (PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) {
        tArgs = ((PyBType *) args[0])->btypeid;
        if (tm_bmetatypeid(pyTm->tm, tArgs) != bmttup) {
            resetToCheckpoint(buckets, &cp);
            return PyErr_Format(PyBTypeError, "tArgs is not a BType nor a tuple BType");
        }
    }
    else if (PyObject_IsInstance(args[0], (PyObject *) &PyTuple_Type)) {
        tup = args[0];
        n = PyTuple_Size(tup);
        tl = (btypeid_t *) allocInBuckets(buckets, ((1 + n) * sizeof(btypeid_t)), bk_alignof(btypeid_t));
        tl[0] = (btypeid_t) n;
        for (int i=0; i < n; i++) {
            e = PyTuple_GetItem(tup, i);
            if (!PyObject_IsInstance(e, (PyObject *) &PyBTypeCls)) {
                resetToCheckpoint(buckets, &cp);
                return PyErr_Format(PyExc_TypeError, "element %i of tuple is not a BType", i);
            }
            tl[i+1] = ((PyBType *) e)->btypeid;
        }
        tlid = tm_tlid(pyTm->tm, tl);
        tArgs = tm_tuple(pyTm->tm, B_NEW, tlid);
        if (!tArgs) {
            resetToCheckpoint(buckets, &cp);
            return PyErr_Format(PyBTypeError, "Error creating tArgs from Python tuple of BTypes");
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

    btypeid_t btypeid = tm_fn(pyTm->tm, self, tArgs, tRet);

    if (btypeid) {
        resetToCheckpoint(buckets, &cp);
        return newPyBTypeRef(btypeid);
    } else {
        TP_init(&tp, 0, buckets);
        PyErr_Format(PyBTypeError, "Error creating (%s) -> %s",
            tm_s8_typelist(pyTm->tm, &tp, tl).cs,
            tm_s8(pyTm->tm, &tp, tRet).cs
        );
        resetToCheckpoint(buckets, &cp);
        return 0;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
// fnArgT: (tFn) -> tArgs + PyException

pvt PyObject * PyTM_fnTArgs(PyTM *self, PyObject **args, Py_ssize_t nargs) {
    // answer tArgs of the given tFn
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "btype is not a BType");
    btypeid_t tFn = ((PyBType *) args[0])->btypeid;
    if (tm_bmetatypeid(self->tm, tFn) != bmtfnc) return PyErr_Format(PyBTypeError, "btype is not a fn type");
    return newPyBTypeRef(tm_fn_targs_tret(self->tm, tFn).tArgs);
}

// ---------------------------------------------------------------------------------------------------------------------
// fnRetT: (tFn) -> tRet + PyException

pvt PyObject * PyTM_fnTRet(PyTM *self, PyObject **args, Py_ssize_t nargs) {
    // answer tRet of the given tFn
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "btype is not a BType");
    btypeid_t tFn = ((PyBType *) args[0])->btypeid;
    if (tm_bmetatypeid(self->tm, tFn) != bmtfnc) return PyErr_Format(PyBTypeError, "btype is not a fn type");
    return newPyBTypeRef(tm_fn_targs_tret(self->tm, tFn).tRet);
}

// ---------------------------------------------------------------------------------------------------------------------
// fromId: (btypeid) -> PyBType + PyException

pvt PyObject * PyTM_fromId(PyTM *self, PyObject **args, Py_ssize_t nargs) {
    // answer a PyBType given its btypeid
    int overflow;
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyLong_Check(args[0])) return PyErr_Format(PyExc_TypeError, "btypeid must be an int");
    btypeid_t btypeid = (btypeid_t) PyLong_AsLongAndOverflow(args[0], &overflow);
    if (overflow != 0 || btypeid <= 0 || btypeid >= self->tm->next_btypeId) return PyErr_Format(PyBTypeError, "btypeid is outside the range of all BTypes");
    return newPyBTypeRef(btypeid);
}

// ---------------------------------------------------------------------------------------------------------------------
// fromName: (name:str) -> PyBType + PyException

pvt PyObject * PyTM_fromName(PyTM *self, PyObject **args, Py_ssize_t nargs) {
    // answer a new BType given a name or a TypeError if there is no btype with that name
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyUnicode_Check(args[0]) || (PyUnicode_KIND(args[0]) != PyUnicode_1BYTE_KIND)) return PyErr_Format(PyExc_TypeError, "name must be utf8");
    char *name = (char *) PyUnicode_AsUTF8(args[0]);
    btypeid_t btypeid = tm_btypeid(self->tm, name);
    if (!btypeid) return PyErr_Format(PyBTypeError, "Unknown type '%s'", name);
    return newPyBTypeRef(btypeid);
}

// ---------------------------------------------------------------------------------------------------------------------
// hasT: (btype) -> PyBool + PyException

pvt PyObject * PyTM_hasT(PyTM *self, PyObject **args, Py_ssize_t nargs) {
    // answer if the given btype contains a schemavar
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "btype is not a BType");
    return tm_hasT(self->tm, ((PyBType *) args[0])->btypeid) ? Py_NewRef(Py_True) : Py_NewRef(Py_False);
}

// ---------------------------------------------------------------------------------------------------------------------
// intersection: (btype1, btype2, ...) -> PyBType + PyException

pvt PyObject * PyTM_intersection(PyTM *pyTm, PyObject *const *args, Py_ssize_t nargs, PyObject *argnames) {
    // answer a new intersection of the given btypes
    BK_TP tp;  Buckets *buckets;  BucketsCheckpoint cp;  btypeid_t *tl, btypeid, self = B_NEW, orthspcid = 0;
    PyObject *pySelf = 0, *pyOrthspc = 0;

    if (nargs == 0) return PyErr_Format(PyExc_TypeError, "Must provide at least one type");
    if (nargs == 1) {
        if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "arg is not a BType");
        return Py_NewRef(args[0]);
    }
    if (argnames) {
        for (int i = 0; i < PyTuple_Size(argnames); i++) {
            __GET_KWARG("self", PyTuple_GET_ITEM(argnames, i), pySelf = args[i + nargs])
            __OR_GET_KWARG("orthspc", PyTuple_GET_ITEM(argnames, i), pyOrthspc = args[i + nargs])
            __ELSE_RAISE(PyExc_TypeError, "Unknown keyword argument \"%s\"", PyUnicode_AsUTF8(PyTuple_GET_ITEM(argnames, i)))
        }
        if (pySelf) {
            __CHECK(PyObject_IsInstance(pySelf, (PyObject *) &PyBTypeCls), PyExc_TypeError, "self must be a BType");
            self = ((PyBType *) pySelf)->btypeid;
        }
        if (pyOrthspc) {
            __CHECK(PyObject_IsInstance(pyOrthspc, (PyObject *) &PyBTypeCls), PyExc_TypeError, "orthspc must be a BType");
            orthspcid = ((PyBType *) pyOrthspc)->btypeid;
        }
    }

    checkpointBuckets((buckets = pyTm->tm->buckets), &cp);

    // create a type list of the correct length
    tl = (btypeid_t *) allocInBuckets(buckets, ((1 + nargs) * sizeof(btypeid_t)), bk_alignof(btypeid_t));
    tl[0] = (btypeid_t) nargs;
    for (int i=1; i <= nargs; i++) {
        if (!PyObject_IsInstance(args[i-1], (PyObject *) &PyBTypeCls)) {
            resetToCheckpoint(buckets, &cp);
            return PyErr_Format(PyExc_TypeError, "arg%i is not a BType", i);
        }
        tl[i] = ((PyBType *) args[i-1])->btypeid;
    }

    btypeid = tm_inter(pyTm->tm, self, tl);
    if (orthspcid) pyTm->tm->orthspcid_by_btypeid[btypeid] = orthspcid;

    if (btypeid) {
        resetToCheckpoint(buckets, &cp);
        return newPyBTypeRef(btypeid);
    } else {
        TP_init(&tp, 0, buckets);
        PyErr_Format(PyBTypeError, "There are exclusion conflicts within (%s)", tm_s8_typelist(pyTm->tm, &tp, tl).cs);
        resetToCheckpoint(buckets, &cp);
        return 0;
    }
}



// ---------------------------------------------------------------------------------------------------------------------
// intersectionTl: (btype) -> (PyBype1, ...) + PyException

pvt PyObject * PyTM_intersectionTl(PyTM *self, PyObject **args, Py_ssize_t nargs) {
    // answer a new PyTuple of new PyBTypes which is the type list of the given intersection btype
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "btype is not a BType");
    btypeid_t *tl = tm_inter_tl(self->tm, ((PyBType *) args[0])->btypeid);
    if (tl == 0) return PyErr_Format(PyBTypeError, "btype is not an intersection type");
    PyObject *answer = PyTuple_New(tl[0]);
    for (btypeid_t i = 1; i <= tl[0]; i++) {
        PyTuple_SET_ITEM(answer, i - 1, newPyBTypeRef(tl[i]));
    }
    return answer;
}


// PyTM_intersection_tlid
// tm_inter_tlid


// ---------------------------------------------------------------------------------------------------------------------
// isExplicit: (btype) -> PyBool + PyException

pvt PyObject * PyTM_isExplicit(PyTM *self, PyObject **args, Py_ssize_t nargs) {
    // answer if the given btype requires an explicit match
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "btype is not a BType");
    bool isExplicit = TM_IS_EXPLICIT(self->tm->btsummary_by_btypeid[((PyBType *) args[0])->btypeid]);
    return PyBool_FromLong(isExplicit);
}

// ---------------------------------------------------------------------------------------------------------------------
// isRecursive: (btype) -> PyBool + PyException

pvt PyObject * PyTM_isRecursive(PyTM *self, PyObject **args, Py_ssize_t nargs) {
    // answer if the given btype is recursive
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "btype is not a BType");
    bool isRec = TM_IS_RECURSIVE(self->tm->btsummary_by_btypeid[((PyBType *) args[0])->btypeid]);
    return PyBool_FromLong(isRec);
}

// ---------------------------------------------------------------------------------------------------------------------
// map: (tK, tV) -> tMap + PyException

pvt PyObject * PyTM_map(PyTM *pyTm, PyObject **args, Py_ssize_t nargs, PyObject *argnames) {
    // answer the tMap corresponding to tK and tV
    BK_TP tp;  Buckets *buckets;  BucketsCheckpoint cp;  btypeid_t tK, tV, tMap, self = B_NEW;  PyObject *pySelf = 0;

    if (nargs != 2) return PyErr_Format(PyExc_TypeError, "Must provide tK and tV");
    if (argnames) {
        for (int i = 0; i < PyTuple_Size(argnames); i++) {
            __GET_KWARG("self", PyTuple_GET_ITEM(argnames, i), pySelf = args[i + nargs])
            __ELSE_RAISE(PyExc_TypeError, "Unknown keyword argument \"%s\"", PyUnicode_AsUTF8(PyTuple_GET_ITEM(argnames, i)))
        }
        if (pySelf) {
            __CHECK(PyObject_IsInstance(pySelf, (PyObject *) &PyBTypeCls), PyExc_TypeError, "self must be a BType");
            self = ((PyBType *) pySelf)->btypeid;
        }
    }

    checkpointBuckets((buckets = pyTm->tm->buckets), &cp);

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

    tMap = tm_map(pyTm->tm, self, tK, tV);

    if (tMap) {
        resetToCheckpoint(buckets, &cp);
        return newPyBTypeRef(tMap);
    } else {
        TP_init(&tp, 0, buckets);
        PyErr_Format(PyBTypeError, "Error creating map for %s -> %s", tm_s8(pyTm->tm, &tp, tK).cs, tm_s8(pyTm->tm, &tp, tV).cs);
        resetToCheckpoint(buckets, &cp);
        return 0;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
// mapTK: (tMap) -> tK + PyException

pvt PyObject * PyTM_mapTK(PyTM *self, PyObject **args, Py_ssize_t nargs) {
    // answer tK of the given tMap
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "btype is not a BType");
    btypeid_t tMap = ((PyBType *) args[0])->btypeid;
    if (tm_bmetatypeid(self->tm, tMap) != bmtmap) return PyErr_Format(PyBTypeError, "btype is not a map type");
    return newPyBTypeRef(tm_map_tk_tv(self->tm, tMap).tK);
}

// ---------------------------------------------------------------------------------------------------------------------
// mapTV: (tMap) -> tV + PyException

pvt PyObject * PyTM_mapTV(PyTM *self, PyObject **args, Py_ssize_t nargs) {
    // answer tV of the given tMap
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "btype is not a BType");
    btypeid_t tMap = ((PyBType *) args[0])->btypeid;
    if (tm_bmetatypeid(self->tm, tMap) != bmtmap) return PyErr_Format(PyBTypeError, "btype is not a map type");
    return newPyBTypeRef(tm_map_tk_tv(self->tm, tMap).tV);
}

// ---------------------------------------------------------------------------------------------------------------------
// minus: (btype, btype) -> btype + PyException

pvt PyObject * PyTM_minus(PyTM *self, PyObject **args, Py_ssize_t nargs) {
    // answer the name of the given btype
    if (nargs != 2) return jErrWrongNumberOfArgs(FN_NAME, 2, nargs);
    if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "A is not a BType");
    if (!PyObject_IsInstance(args[1], (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "B is not a BType");
    btypeid_t btypeid = tm_minus(self->tm, B_NEW, ((PyBType *) args[0])->btypeid, ((PyBType *) args[1])->btypeid);
    if (btypeid == 0)
        return PyErr_Format(PyBTypeError, "Error doing A minus B.");
    else
        return newPyBTypeRef(btypeid);
}

// ---------------------------------------------------------------------------------------------------------------------
// name: (btype) -> PyStr + PyException

pvt PyObject * PyTM_name(PyTM *self, PyObject **args, Py_ssize_t nargs) {
    // answer the name of the given btype
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "btype is not a BType");
    // OPEN: what to do if there is no name (use t123?) - 0 means invalid type?
    char const *name = tm_name(self->tm, ((PyBType *) args[0])->btypeid);
    if (name == 0) Py_RETURN_NONE;
    return PyUnicode_FromString(name);
}

// ---------------------------------------------------------------------------------------------------------------------
// nameAs: (btype, name:str) -> PyBType + PyException

pvt PyObject * PyTM_nameAs(PyTM *self, PyObject **args, Py_ssize_t nargs) {
    // answer a new reference to the given btype, naming it in the process if it hasn't already got one
    char const *newname, *oldname;  btypeid_t btypeid;

    if (nargs != 2) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "btype is not a BType");
    if (!PyUnicode_Check(args[1]) || (PyUnicode_KIND(args[1]) != PyUnicode_1BYTE_KIND)) return PyErr_Format(PyExc_TypeError, "name must be utf8");

    PyBType *btype = (PyBType *) args[0];
    newname = (char *) PyUnicode_AsUTF8(args[1]);       // OPEN: why move from PyUnicode_1BYTE_DATA?
    if ((btypeid = tm_name_as(self->tm, btype->btypeid, newname))) {
        return newPyBTypeRef(btypeid);
    } else {
        if ((oldname = tm_name(self->tm, btype->btypeid))) {
            return strcmp(newname, oldname) == 0 ? newPyBTypeRef(btypeid) : PyErr_Format(PyBTypeError, "t%i is already named \"%s\"", btype->btypeid, oldname);
        } else {
            return PyErr_Format(PyBTypeError, "\"%s\" is already in use by another btype", newname);
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
// orthspc: (btype) -> PyBType + None

pvt PyObject * PyTM_orthspc(PyTM *self, PyObject **args, Py_ssize_t nargs) {
    // answer the orthspc of the given btype
    btypeid_t btypeid;
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "btype is not a BType");
    // OPEN: what to do if there is no name (use t123?) - 0 means invalid type?
    btypeid = tm_orthspcid(self->tm, ((PyBType *) args[0])->btypeid);
    if (btypeid == B_NAT) Py_RETURN_NONE;
    return newPyBTypeRef(btypeid);   // OPEN: something like PyBType_FromBTypeId(btypeid)?
}

// ---------------------------------------------------------------------------------------------------------------------
// rootOrthspc: (btype) -> PyBType + None

pvt PyObject * PyTM_rootOrthspc(PyTM *self, PyObject **args, Py_ssize_t nargs) {
    // answer the orthspc of the given btype
    btypeid_t btypeid;
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "btype is not a BType");
    // OPEN: what to do if there is no name (use t123?) - 0 means invalid type?
    btypeid = tm_root_orthspcid(self->tm, ((PyBType *) args[0])->btypeid);
    if (btypeid == B_NAT) Py_RETURN_NONE;
    return newPyBTypeRef(btypeid);   // OPEN: something like PyBType_FromBTypeId(btypeid)?
}

// ---------------------------------------------------------------------------------------------------------------------
// options: ([self:btype], [explicit:bool], [implicitly:btype], [orthspc:btype], [name:str]) -> PyBType + PyException

pvt PyObject * PyTM_reserve(PyTM *pyTm, PyObject *const *args, Py_ssize_t nargs, PyObject *argnames) {
    // answers a new uninitialised btype with the given options
    PyObject *pySelf = 0, *pyExplicit = 0, *pyImplicitly = 0, *pyOrthspc = 0, *pyName = 0;
    btypeid_t btypeid, self = B_NEW, implicitly = 0, orthspc = 0;  bool explicit = false;  char const *name = 0;

    __CHECK(nargs == 0, PyExc_TypeError, "options(**kwargs) takes no args but %i were given", nargs);

    if (argnames) {
        for (int i = 0; i < PyTuple_Size(argnames); i++) {
            __GET_KWARG("self", PyTuple_GET_ITEM(argnames, i), pySelf = args[i + nargs])
            __OR_GET_KWARG("explicit", PyTuple_GET_ITEM(argnames, i), pyExplicit = args[i + nargs])
            __OR_GET_KWARG("implicitly", PyTuple_GET_ITEM(argnames, i), pyImplicitly = args[i + nargs])
            __OR_GET_KWARG("orthspc", PyTuple_GET_ITEM(argnames, i), pyOrthspc = args[i + nargs])
            __OR_GET_KWARG("name", PyTuple_GET_ITEM(argnames, i), pyName = args[i + nargs])
            __ELSE_RAISE(PyExc_TypeError, "Unknown keyword argument \"%s\"", PyUnicode_AsUTF8(PyTuple_GET_ITEM(argnames, i)))
        }
        if (pySelf) {
            __CHECK(PyObject_IsInstance(pySelf, (PyObject *) &PyBTypeCls), PyExc_TypeError, "self must be a BType");
            self = ((PyBType *) pySelf)->btypeid;
        }
        if (pyExplicit) {
            __CHECK(PyBool_Check(pyExplicit), PyExc_TypeError, "explicit must be a bool");
            explicit = (pyExplicit == Py_True);
        }
        if (pyImplicitly) {
            __CHECK(PyObject_IsInstance(pyImplicitly, (PyObject *) &PyBTypeCls), PyExc_TypeError, "implicitly must be a BType");
            implicitly = ((PyBType *) pyImplicitly)->btypeid;
        }
        if (pyOrthspc) {
            __CHECK(PyObject_IsInstance(pyOrthspc, (PyObject *) &PyBTypeCls), PyExc_TypeError, "orthspc must be a BType");
            orthspc = ((PyBType *) pyOrthspc)->btypeid;
        }
        if (pyName) {
            __CHECK(PyUnicode_Check(pyName), PyExc_TypeError, "name must be a utf8");
            __CHECK(PyUnicode_KIND(pyName) == PyUnicode_1BYTE_KIND, PyExc_TypeError, "name must be a str");
            name = (char const *) PyUnicode_AsUTF8(pyName);
        }
    }

    btypeid = tm_reserve(pyTm->tm, self, orthspc, explicit, implicitly);
    if (name) btypeid = tm_name_as(pyTm->tm, btypeid, name);
    if (btypeid)
        return newPyBTypeRef(btypeid);
    else
        return PyErr_Format(PyBTypeError, "error creating options");
}

// ---------------------------------------------------------------------------------------------------------------------
// schemavar: (name:str) -> tSchemavar + PyException

pvt PyObject * PyTM_schemavar(PyTM *pyTm, PyObject **args, Py_ssize_t nargs, PyObject *argnames) {
    // answer a new schema variable with the given name, or an exception if already taken
    PyObject *pySelf;  btypeid_t self = B_NEW;
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyUnicode_Check(args[0]) || (PyUnicode_KIND(args[0]) != PyUnicode_1BYTE_KIND)) return PyErr_Format(PyExc_TypeError, "name must be utf8");
    if (argnames) {
        for (int i = 0; i < PyTuple_Size(argnames); i++) {
            __GET_KWARG("self", PyTuple_GET_ITEM(argnames, i), pySelf = args[i + nargs])
            __ELSE_RAISE(PyExc_TypeError, "Unknown keyword argument \"%s\"", PyUnicode_AsUTF8(PyTuple_GET_ITEM(argnames, i)))
        }
        if (pySelf) {
            __CHECK(PyObject_IsInstance(pySelf, (PyObject *) &PyBTypeCls), PyExc_TypeError, "self must be a BType");
            self = ((PyBType *) pySelf)->btypeid;
        }
    }
    char *name = (char *) PyUnicode_AsUTF8(args[0]);
    btypeid_t btypeid = tm_schemavar(pyTm->tm, self, name);
    if (btypeid)
        return newPyBTypeRef(btypeid);
    else
        return PyErr_Format(PyBTypeError, "name is taken by another type");  // OPEN: better error
}

// ---------------------------------------------------------------------------------------------------------------------
// seq: (contained:btype) -> PyBType + PyException

pvt PyObject * PyTM_seq(PyTM *pyTm, PyObject **args, Py_ssize_t nargs, PyObject *argnames) {
    // answer a new sequence for the given contained btype
    BK_TP tp;  Buckets *buckets;  BucketsCheckpoint cp;  btypeid_t self = B_NEW;  PyObject *pySelf;

    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (argnames) {
        for (int i = 0; i < PyTuple_Size(argnames); i++) {
            __GET_KWARG("self", PyTuple_GET_ITEM(argnames, i), pySelf = args[i + nargs])
            __ELSE_RAISE(PyExc_TypeError, "Unknown keyword argument \"%s\"", PyUnicode_AsUTF8(PyTuple_GET_ITEM(argnames, i)))
        }
        if (pySelf) {
            __CHECK(PyObject_IsInstance(pySelf, (PyObject *) &PyBTypeCls), PyExc_TypeError, "self must be a BType");
            self = ((PyBType *) pySelf)->btypeid;
        }
    }
    checkpointBuckets((buckets = pyTm->tm->buckets), &cp);

    if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) {
        resetToCheckpoint(buckets, &cp);
        return PyErr_Format(PyExc_TypeError, "arg is not a BType");
    }

    btypeid_t btypeid = tm_seq(pyTm->tm, self, ((PyBType *) args[0])->btypeid);

    if (btypeid) {
        resetToCheckpoint(buckets, &cp);
        return newPyBTypeRef(btypeid);
    } else {
        TP_init(&tp, 0, buckets);
        PyErr_Format(PyBTypeError, "Undetermined error");
        resetToCheckpoint(buckets, &cp);
        return 0;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
// seqT: (seq_type:btype) -> PyBType + PyException

pvt PyObject * PyTM_seqT(PyTM *self, PyObject **args, Py_ssize_t nargs) {
    // answer the btype of the contained type for the given sequence type
    btypeid_t btypeid;
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "btype is not a BType");
    btypeid = tm_seq_t(self->tm, ((PyBType *) args[0])->btypeid);
    if (btypeid == 0) return PyErr_Format(PyBTypeError, "btype is not a sequence type");
    return newPyBTypeRef(btypeid);
}

// ---------------------------------------------------------------------------------------------------------------------
// struct: (str, btype1, str, btype2...) -> PyBType + PyException

pvt PyObject * PyTM_struct(PyTM *pyTm, PyObject **args, Py_ssize_t nargs, PyObject *argnames) {
    // answers a new struct type given the names and btypes
    BK_TP tp;  Buckets *buckets;  BucketsCheckpoint cp;  btypeid_t *tl;  symid_t *sl, self = B_NEW;  int n;
    PyObject *s, *btype, *pySelf = 0;  char const *name;

    if (nargs != 2) return PyErr_Format(PyExc_TypeError, "Must provide names and types");
    if (argnames) {
        for (int i = 0; i < PyTuple_Size(argnames); i++) {
            __GET_KWARG("self", PyTuple_GET_ITEM(argnames, i), pySelf = args[i + nargs])
            __ELSE_RAISE(PyExc_TypeError, "Unknown keyword argument \"%s\"", PyUnicode_AsUTF8(PyTuple_GET_ITEM(argnames, i)))
        }
        if (pySelf) {
            __CHECK(PyObject_IsInstance(pySelf, (PyObject *) &PyBTypeCls), PyExc_TypeError, "self must be a BType");
            self = ((PyBType *) pySelf)->btypeid;
        }
    }
    if (!PyObject_IsInstance(args[0], (PyObject *) &PyTuple_Type)) return PyErr_Format(PyExc_TypeError, "names is not a tuple");
    if (!PyObject_IsInstance(args[1], (PyObject *) &PyTuple_Type)) return PyErr_Format(PyExc_TypeError, "types is not a tuple");
    n = PyTuple_Size(args[0]);
    if (n != PyTuple_Size(args[1])) return PyErr_Format(PyExc_TypeError, "names is not same size as types");

    checkpointBuckets((buckets = pyTm->tm->buckets), &cp);

    // create a sym list and type list of the correct length
    tl = (btypeid_t *) allocInBuckets(buckets, ((1 + n) * sizeof(btypeid_t)), bk_alignof(btypeid_t));
    sl = (btypeid_t *) allocInBuckets(buckets, ((1 + n) * sizeof(btypeid_t)), bk_alignof(btypeid_t));
    tl[0] = (btypeid_t) n;
    sl[0] = (symid_t) n;
    for (int i=1; i <= n; i++) {
        s = PyTuple_GetItem(args[0], i - 1);
        btype = PyTuple_GetItem(args[1], i - 1);
        if (!PyUnicode_Check(s) || (PyUnicode_KIND(s) != PyUnicode_1BYTE_KIND)) {
            resetToCheckpoint(buckets, &cp);
            return PyErr_Format(PyExc_TypeError, "name%1 is not utf8", i);
        }
        if (!PyObject_IsInstance(btype, (PyObject *) &PyBTypeCls)) {
            resetToCheckpoint(buckets, &cp);
            return PyErr_Format(PyExc_TypeError, "type%i is not a BType", i);
        }
        name = (char *) PyUnicode_AsUTF8(s);
        sl[i] = sm_id(pyTm->tm->sm, name);
        tl[i] = ((PyBType *) btype)->btypeid;
    }

    SM_SLID_T slid = sm_slid(pyTm->tm->sm, sl);
    btypeid_t tupid = tm_tuple(pyTm->tm, B_NEW, tm_tlid(pyTm->tm, tl));    // OPEN: don't do this as it may create a tuple with the wrong attributes
    btypeid_t btypeid = tm_struct(pyTm->tm, self, slid, tupid);

    if (btypeid) {
        resetToCheckpoint(buckets, &cp);
        return newPyBTypeRef(btypeid);
    } else {
        TP_init(&tp, 0, buckets);
        PyErr_Format(
            PyBTypeError,
            "Error creating struct (%s, (%s))",
            sm_s8_symlist(pyTm->tm->sm, &tp, sl).cs,
            tm_s8_typelist(pyTm->tm, &tp, tl).cs
        );
        resetToCheckpoint(buckets, &cp);
        return 0;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
// structSl: btype -> (PySym1, ...) + PyException

pvt PyObject * PyTM_structSl(PyTM *self, PyObject **args, Py_ssize_t nargs) {
    Py_RETURN_NONE;
}

// ---------------------------------------------------------------------------------------------------------------------
// structTl: btype -> (PyBType1, ...) + PyException

pvt PyObject * PyTM_structTl(PyTM *self, PyObject **args, Py_ssize_t nargs) {
    // answer a new PyTuple of new PyBTypes which is the type list of the fields of the given struct btype
    btypeid_t *tl;  PyObject *answer;
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "btype is not a BType");
    tl = tm_struct_tl(self->tm, ((PyBType *) args[0])->btypeid);
    if (tl == 0) return PyErr_Format(PyBTypeError, "btype is not a struct type");
    answer = PyTuple_New(tl[0]);
    for (btypeid_t i = 0; i < tl[0]; i++) {
        PyTuple_SET_ITEM(answer, i, newPyBTypeRef(tl[i+1]));
    }
    return answer;
}

// ---------------------------------------------------------------------------------------------------------------------
// tuple: (btype1, btype2, ...) -> PyBType + PyException

pvt PyObject * PyTM_tuple(PyTM *pyTm, PyObject **args, Py_ssize_t nargs, PyObject *argnames) {
    // answers a new tuple type of the given btypes
    btypeid_t self = B_NEW;  PyObject *pySelf = 0;
    BK_TP tp;  Buckets *buckets;  BucketsCheckpoint cp;  btypeid_t *tl;
    if (argnames) {
        for (int i = 0; i < PyTuple_Size(argnames); i++) {
            __GET_KWARG("self", PyTuple_GET_ITEM(argnames, i), pySelf = args[i + nargs])
            __ELSE_RAISE(PyExc_TypeError, "Unknown keyword argument \"%s\"", PyUnicode_AsUTF8(PyTuple_GET_ITEM(argnames, i)))
        }
        if (pySelf) {
            __CHECK(PyObject_IsInstance(pySelf, (PyObject *) &PyBTypeCls), PyExc_TypeError, "self must be a BType");
            self = ((PyBType *) pySelf)->btypeid;
        }
    }
    checkpointBuckets((buckets = pyTm->tm->buckets), &cp);

    // create a type list of the correct length
    tl = (btypeid_t *) allocInBuckets(buckets, ((1 + nargs) * sizeof(btypeid_t)), bk_alignof(btypeid_t));
    tl[0] = (btypeid_t) nargs;
    for (int i=1; i <= nargs; i++) {
        if (!PyObject_IsInstance(args[i-1], (PyObject *) &PyBTypeCls)) {
            resetToCheckpoint(buckets, &cp);
            return PyErr_Format(PyExc_TypeError, "arg%i is not a BType", i);
        }
        tl[i] = ((PyBType *) args[i-1])->btypeid;
    }

    btypeid_t btypeid = tm_tuple(pyTm->tm, B_NEW, tm_tlid(pyTm->tm, tl));    // OPEN: don't do this as it may create a tuple with the wrong attributes

    if (btypeid) {
        resetToCheckpoint(buckets, &cp);
        return newPyBTypeRef(btypeid);
    } else {
        TP_init(&tp, 0, buckets);
        PyErr_Format(PyBTypeError, "Error creating tuple (%s)", tm_s8_typelist(pyTm->tm, &tp, tl).cs);
        resetToCheckpoint(buckets, &cp);
        return 0;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
// tupleTl: btype -> (PyBType1, ...) + PyException

pvt PyObject * PyTM_tupleTl(PyTM *self, PyObject **args, Py_ssize_t nargs) {
    btypeid_t *tl;  PyObject *answer;
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "btype is not a BType");
    tl = tm_tuple_tl(self->tm, ((PyBType *) args[0])->btypeid);
    if (tl == 0) return PyErr_Format(PyBTypeError, "btype is not a tuple type");
    answer = PyTuple_New(tl[0]);
    for (btypeid_t i = 0; i < tl[0]; i++) {
        PyTuple_SET_ITEM(answer, i, newPyBTypeRef(tl[i+1]));
    }
    return answer;
}

// ---------------------------------------------------------------------------------------------------------------------
// union: (btype1, btype2, ...) -> PyBType + PyException

pvt PyObject * PyTM_union(PyTM *pyTm, PyObject **args, Py_ssize_t nargs, PyObject *argnames) {
    // answer a new union of the given btypes
    BK_TP tp;  Buckets *buckets;  BucketsCheckpoint cp;  btypeid_t self = B_NEW;  PyObject *pySelf = 0;
    if (nargs == 0) return PyErr_Format(PyExc_TypeError, "Must provide at least one type");
    if (nargs == 1) {
        if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "arg is not a BType");
        return Py_NewRef(args[0]);
    }
    if (argnames) {
        for (int i = 0; i < PyTuple_Size(argnames); i++) {
            __GET_KWARG("self", PyTuple_GET_ITEM(argnames, i), pySelf = args[i + nargs])
            __ELSE_RAISE(PyExc_TypeError, "Unknown keyword argument \"%s\"", PyUnicode_AsUTF8(PyTuple_GET_ITEM(argnames, i)))
        }
        if (pySelf) {
            __CHECK(PyObject_IsInstance(pySelf, (PyObject *) &PyBTypeCls), PyExc_TypeError, "self must be a BType");
            self = ((PyBType *) pySelf)->btypeid;
        }
    }
    checkpointBuckets((buckets = pyTm->tm->buckets), &cp);

    // create a type list of the correct length
    btypeid_t *tl = (btypeid_t *) allocInBuckets(buckets, ((1 + nargs) * sizeof(btypeid_t)), bk_alignof(btypeid_t));
    tl[0] = (btypeid_t) nargs;
    for (int i=1; i <= nargs; i++) {
        if (!PyObject_IsInstance(args[i-1], (PyObject *) &PyBTypeCls)) {
            resetToCheckpoint(buckets, &cp);
            return PyErr_Format(PyExc_TypeError, "arg%i is not a BType", i);
        }
        tl[i] = ((PyBType *) args[i-1])->btypeid;
    }

    btypeid_t btypeid = tm_union(pyTm->tm, self, tl);

    if (btypeid) {
        resetToCheckpoint(buckets, &cp);
        return newPyBTypeRef(btypeid);
    } else {
        TP_init(&tp, 0, buckets);
        PyErr_Format(PyBTypeError, "There are conflicts within (%s)", tm_s8_typelist(pyTm->tm, &tp, tl).cs);
        resetToCheckpoint(buckets, &cp);
        return 0;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
// union_for: (tlid) -> PyBType + PyNone

pvt PyObject * PyTM_union_for(PyTM *pyTm, PyObject **args, Py_ssize_t nargs, PyObject *argnames) {
    // answer the current union for the given type list id if it exists else None
    BK_TP tp;  Buckets *buckets;  BucketsCheckpoint cp;  btypeid_t btypeid, self=B_NEW;  btypeid_t *tl;  TM_TLID_T tlid;
    PyObject *pySelf = 0;

    if (nargs != 1) return PyErr_Format(PyExc_TypeError, "Must provide at least one type");
    if (!PyLong_Check(args[0])) return PyErr_Format(PyExc_TypeError, "tlid must be int");

    if (argnames) {
        for (int i = 0; i < PyTuple_Size(argnames); i++) {
            __GET_KWARG("self", PyTuple_GET_ITEM(argnames, i), pySelf = args[i + nargs])
            __ELSE_RAISE(PyExc_TypeError, "Unknown keyword argument \"%s\"", PyUnicode_AsUTF8(PyTuple_GET_ITEM(argnames, i)))
        }
        if (pySelf) {
            __CHECK(PyObject_IsInstance(pySelf, (PyObject *) &PyBTypeCls), PyExc_TypeError, "self must be a BType");
            self = ((PyBType *) pySelf)->btypeid;
        }
    }
    tlid = PyLong_AsLong(args[0]);
    btypeid = tm_union_for_tlid_or_create(pyTm->tm, self, tlid);

    if (btypeid) {
        return newPyBTypeRef(btypeid);
    } else {
        checkpointBuckets((buckets = pyTm->tm->buckets), &cp);
        TP_init(&tp, 0, buckets);
        tl = pyTm->tm->typelist_buf + pyTm->tm->tlrp_by_tlid[tlid];
        PyErr_Format(PyBTypeError, "There are conflicts within (%s)", tm_s8_typelist(pyTm->tm, &tp, tl).cs);
        resetToCheckpoint(buckets, &cp);
        return 0;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
// union_get_for: (tlid) -> PyBType + PyNone

pvt PyObject * PyTM_union_get_for(PyTM *pyTm, PyObject **args, Py_ssize_t nargs) {
    // answer the current union for the given type list id if it exists else None
    btypeid_t btypeid;
    if (nargs != 1) return PyErr_Format(PyExc_TypeError, "Must provide at least one type");
    if (!PyLong_Check(args[0])) return PyErr_Format(PyExc_TypeError, "tlid must be int");

    btypeid = tm_union_for_tlid(pyTm->tm, PyLong_AsLong(args[0]));

    if (btypeid) {
        return newPyBTypeRef(btypeid);
    } else {
        Py_RETURN_NONE;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
// union_tlid_for: (btype1, btype2, ...) -> PyBType + PyException

pvt PyObject * PyTM_union_tlid_for(PyTM *pyTm, PyObject **args, Py_ssize_t nargs, PyObject *argnames) {
    // answer a new union of the given btypes
    BK_TP tp;  Buckets *buckets;  BucketsCheckpoint cp;  btypeid_t *tl;
    if (nargs == 0) return PyErr_Format(PyExc_TypeError, "Must provide at least one type");
    if (nargs == 1) {
        if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "arg is not a BType");
        return Py_NewRef(args[0]);
    }
    if (argnames) {
        for (int i = 0; i < PyTuple_Size(argnames); i++) {
            __GET_KWARG("self", PyTuple_GET_ITEM(argnames, i), )
            __ELSE_RAISE(PyExc_TypeError, "Unknown keyword argument \"%s\"", PyUnicode_AsUTF8(PyTuple_GET_ITEM(argnames, i)))
        }
    }
    checkpointBuckets((buckets = pyTm->tm->buckets), &cp);

    // create a type list of the correct length
    tl = (btypeid_t *) allocInBuckets(buckets, ((1 + nargs) * sizeof(btypeid_t)), bk_alignof(btypeid_t));
    tl[0] = (btypeid_t) nargs;
    for (int i=1; i <= nargs; i++) {
        if (!PyObject_IsInstance(args[i-1], (PyObject *) &PyBTypeCls)) {
            resetToCheckpoint(buckets, &cp);
            return PyErr_Format(PyExc_TypeError, "arg%i is not a BType", i);
        }
        tl[i] = ((PyBType *) args[i-1])->btypeid;
    }

    TM_TLID_T tlid = tm_union_tlid(pyTm->tm, tl);

    if (tlid) {
        resetToCheckpoint(buckets, &cp);
        return PyLong_FromLong(tlid);
    } else {
        TP_init(&tp, 0, buckets);
        PyErr_Format(PyBTypeError, "There are conflicts within (%s)", tm_s8_typelist(pyTm->tm, &tp, tl).cs);
        resetToCheckpoint(buckets, &cp);
        return 0;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
// unionTl: (union:btype) -> (PyBType1,...) + PyException

pvt PyObject * PyTM_unionTl(PyTM *self, PyObject **args, Py_ssize_t nargs) {
    // answer a new PyTuple of new PyBTypes which is the type list of the given union
    btypeid_t *tl;  PyObject *answer;
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "btype is not a BType");
    tl = tm_union_tl(self->tm, ((PyBType *) args[0])->btypeid);
    if (tl == 0) return PyErr_Format(PyBTypeError, "btype is not a union type");
    answer = PyTuple_New(tl[0]);
    for (btypeid_t i=1; i <= tl[0]; i++) {
        PyTuple_SET_ITEM(answer, i - 1, newPyBTypeRef(tl[i]));
    }
    return answer;
}


// ---------------------------------------------------------------------------------------------------------------------
// PyTMCls
// ---------------------------------------------------------------------------------------------------------------------

pvt PyMethodDef PyTM_methods[] = {
    {"atom",                (PyCFunction) PyTM_atom, METH_FASTCALL | METH_KEYWORDS,
        "atom(name, **[self])\n\n"
        "Answers the atom called <name>, creating it if it doesn't exist and orthspc is not given. If orthspc is "
        "provided, completes the initialisation of <self> and places self in <orthspc>. Raises an error if <name> "
        "is used by another type."
    },
    {"bmetatypeid",         (PyCFunction) PyTM_bmetatypeid, METH_FASTCALL,
        "bmetatypeid(t)\n\n"
        "Answers the bmetatypeid of the type if it exists else throws an error."
    },
    {"exists",              (PyCFunction) PyTM_exists, METH_FASTCALL,
        "exists(name)\n\n"
        "Answers if the type with <name> exists or not."
    },
    {"fn",                  (PyCFunction) PyTM_fn, METH_FASTCALL | METH_KEYWORDS,
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
    {"fromName",            (PyCFunction) PyTM_fromName, METH_FASTCALL,
        "fromName(name)\n\nanswers the btype called 'name' else throws an error."
    },
    {"fromId",              (PyCFunction) PyTM_fromId, METH_FASTCALL,
        "fromId(btypeid)\n\n"
        "Answers the type corresponding to btypeid if it exists else throws an error."
    },
    {"hasT",                (PyCFunction) PyTM_hasT, METH_FASTCALL,
        "hasT(btypeid)\n\n"
        "Answers True if the type has a schema variable."
    },
    {"intersection",        (PyCFunction) PyTM_intersection, METH_FASTCALL | METH_KEYWORDS,
            "intersection(t1, t2, ...)\n\n"
            "Answers the intersection t1 & t2 & ... type."
    },
    {"intersectionTl",      (PyCFunction) PyTM_intersectionTl, METH_FASTCALL,
            "intersectionTl(t)\n\n"
            "Answers a tuple of the types in the intersection t."
    },
    {"intersection_tlid",    (PyCFunction) PyTM_intersection_tlid, METH_FASTCALL,
            "intersection_tlid(t1, t2, ...)\n\n"
            "Answers an intersection typelist id for the given tuple of types."
    },
    {"isExplicit",          (PyCFunction) PyTM_isExplicit, METH_FASTCALL,
            "isExplicit(t)\n\n"
            "Answers if the type requires an explicit match explicit."
    },
    {"isRecursive",         (PyCFunction) PyTM_isRecursive, METH_FASTCALL,
            "isRecursive(t)\n\n"
            "Answers if the type is recursive."
    },
    {"map",                 (PyCFunction) PyTM_map, METH_FASTCALL | METH_KEYWORDS,
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
    {"minus",               (PyCFunction) PyTM_minus, METH_FASTCALL,
        "minus(A, B)\n\n"
        "Answers the type of A minus B."
    },
    {"name",                (PyCFunction) PyTM_name, METH_FASTCALL,
        "name(t)\n\n"
        "Answers the name of the type if it exists else throws an error."
    },
    {"nameAs",              (PyCFunction) PyTM_nameAs, METH_FASTCALL,
     "nameAs(t, name)\n\n"
     "Names a type, raising an error if <name> has already been taken."
    },
    {"orthspc",             (PyCFunction) PyTM_orthspc, METH_FASTCALL,
     "orthspc(t)\n\n"
     "Answers the othogonal space the given type's is in or None if it is not in any."
    },
    {"reserve",             (PyCFunction) PyTM_reserve, METH_FASTCALL | METH_KEYWORDS,
        "reserve(** name, orthspc, isexplicit, implicitly)\n\n"
        "Answers an uninitialised recursive type to be initialised later - optionally with name, orthspc, isexplicit & implicitly."
    },
    {"rootOrthspc",         (PyCFunction) PyTM_rootOrthspc, METH_FASTCALL,
     "rootOrthspc(t)\n\n"
     "Answers the root othogonal space the given type's is in or None if it is not in any."
    },
    {"schemavar",           (PyCFunction) PyTM_schemavar, METH_FASTCALL,
        "schemavar(name)\n\n"
        "Answers the schema variable called <name>, creating it if it doesn't exist, or raising an error if "
        "<name> is used by another type."
    },
    {"seq",                 (PyCFunction) PyTM_seq, METH_FASTCALL | METH_KEYWORDS,
        "seq(t)\n\n"
        "Answers the sequence of t type."
    },
    {"seqT",                (PyCFunction) PyTM_seqT, METH_FASTCALL,
        "seqT(t)\n\n"
        "Answers the contained type."
    },
    {"struct",              (PyCFunction) PyTM_struct, METH_FASTCALL | METH_KEYWORDS,
        "struct((f1, f2,...), (t1, t2, ...))\n\n"
        "Answers the struct {f1:t1, f2:t2, ...} type."
    },
    {"structTl",             (PyCFunction) PyTM_structTl, METH_FASTCALL,
        "structTl(t)\n\n"
        "Answers a tuple of the names and types in the struct t."
    },
    {"tuple",               (PyCFunction) PyTM_tuple, METH_FASTCALL | METH_KEYWORDS,
        "tuple(t1, t2, ...)\n\n"
        "Answers the tuple (t1, t2, ...) type."
    },
    {"tupleTl",             (PyCFunction) PyTM_tupleTl, METH_FASTCALL,
        "tupleTl(t)\n\n"
        "Answers a tuple of the types in the tuple t."
    },
    {"union",               (PyCFunction) PyTM_union, METH_FASTCALL | METH_KEYWORDS,
     "union(t1, t2, ...)\n\n"
     "Answers the union type for t1 + t2 + ..."
    },
    {"union_tlid_for",      (PyCFunction) PyTM_union_tlid_for, METH_FASTCALL | METH_KEYWORDS,
        "union_tlid_for(t1, t2, ...)\n\n"
        "Answers the union typelist id for t1 + t2 + ..."
    },
    {"union_for",           (PyCFunction) PyTM_union_for, METH_FASTCALL | METH_KEYWORDS,
     "union_for(tlid, [self])\n\n"
     "Answers the union type id for tlid"
    },
    {"union_get_for",       (PyCFunction) PyTM_union_get_for, METH_FASTCALL,
        "union_get_for(tlid)\n\n"
        "Answers the union type for tlid or None if it doesn't exist"
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
    .tp_basicsize = sizeof(PyTM),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_methods = PyTM_methods,
};



#endif  // SRC_JONES_PYTM_C