// ---------------------------------------------------------------------------------------------------------------------
// Copyright 2025 David Briant, https://github.com/coppertop-bones. Licensed under the Apache License, Version 2.0
//
// PYTM - PYTHON INTERFACE TO TYPE MANAGER
// ---------------------------------------------------------------------------------------------------------------------

#ifndef SRC_JONES_PYTM_C
#define SRC_JONES_PYTM_C "jones/pytm.c"


#include <stdlib.h>
#include "jones.h"
#include "lib/pyutils.h"
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
        _num_btypes += 1024;
        while (_num_btypes <= btypeid) _num_btypes += 1024;
        _btypes = realloc(_btypes, sizeof(PyBType *) * _num_btypes);
        memset(_btypes + oldNum, 0, sizeof(PyObject *) * (_num_btypes - oldNum));  // zero the new memory
//        PP(info, "newPyBTypeRef - #1 _btypes: %p", _btypes);
    }
    if (_btypes[btypeid] == 0) {
//        PP(info, "newPyBTypeRef - #2 _btypes: %p", _btypes);
        PyBType *new = (PyBType *) ((&PyBTypeCls)->tp_alloc(&PyBTypeCls, 0));
        new->btypeid = btypeid;
        _btypes[btypeid] = (PyObject *) new;
    }
//    PP(info, "newPyBTypeRef - #3 _btypes[%i] joe: %p", btypeid, _btypes[btypeid]);
    return Py_NewRef(_btypes[btypeid]);
}

// ---------------------------------------------------------------------------------------------------------------------
// initAtom: replaceWith: (btype, pybtype) -> PyObject + PyException

pvt PyObject * PyTM_replaceWith(PyTM *pyTm, PyObject *const *args, Py_ssize_t nargs) {
    btypeid_t btypeid;
    __CHECK(nargs == 2, PyExc_TypeError, "replaceWith: (btype, pybtype) takes 2 args but %i %s given", nargs, nargs == 1 ? "was": "were");
    __CHECK(PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls), PyExc_TypeError, "btype must be a BType");
    // __CHECK(!PyObject_IsInstance(args[1], (PyObject *) &PyBTypeCls), PyExc_TypeError, "pybtype must not be a BType");
    btypeid = ((PyBType *) args[0])->btypeid;
    if (btypeid == B_NAT) return PyErr_Format(PyBTypeError, "btype must not be NAT");
    Py_DECREF(_btypes[btypeid]);
    _btypes[btypeid] = Py_NewRef(args[1]);
    return Py_NewRef(args[1]);
}

// ---------------------------------------------------------------------------------------------------------------------
// PyTM
// ---------------------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------------------
// initAtom: (**, btype:PyBType, explicit:bool, implicitly:PyBType, space:PyBType) -> PyBType+PyException

pvt PyObject * PyTM_initAtom(PyTM *pyTm, PyObject *const *args, Py_ssize_t nargs, PyObject *argnames) {
    // Answers a BType ref, handling three cases:
    // 1) a brand new atom is being allocated with the provided attributes
    // 2) a TBC is being defined with the provided attributes (the TBC must not have any conflicting attributes already set)
    // 3) the definition of an existing atom is being checked that the attributes don't conflict
    //      a) if current.space is already set then space may be the same or missing
    //      b) if current.implicitly is already set then implicitly may be the same or missing
    //      c) if current.explicit is true then explicit may be True or missing
    PyObject *pyBType = 0, *pySpace = 0, *pyExplicit = 0, *pyImplicitly = 0;
    btypeid_t btypeid = B_NEW, implicitid = 0, spaceid = 0, nextid;  bool explicit = false;

    __CHECK(nargs == 0, PyExc_TypeError, "initAtom: (**, btype, explicit, implicitly, space) takes no args but %i %s given", nargs, nargs == 1 ? "was": "were");
    if (argnames) {
        for (int i = 0; i < PyTuple_Size(argnames); i++) {
            __GET_KWARG("btype", PyTuple_GET_ITEM(argnames, i), pyBType = args[i + nargs])
            __OR_GET_KWARG("explicit", PyTuple_GET_ITEM(argnames, i), pyExplicit = args[i + nargs])
            __OR_GET_KWARG("implicitly", PyTuple_GET_ITEM(argnames, i), pyImplicitly = args[i + nargs])
            __OR_GET_KWARG("space", PyTuple_GET_ITEM(argnames, i), pySpace = args[i + nargs])
            __ELSE_RAISE(PyExc_TypeError, "Unknown keyword argument \"%s\"", PyUnicode_AsUTF8(PyTuple_GET_ITEM(argnames, i)))
        }
        if (pyBType && !Py_IsNone(pyBType)) {
            __CHECK(PyObject_IsInstance(pyBType, (PyObject *) &PyBTypeCls), PyExc_TypeError, "`btypeid` must be a BType");
            btypeid = ((PyBType *) pyBType)->btypeid;
        }
        if (pyExplicit && !Py_IsNone(pyExplicit)) {
            __CHECK(PyBool_Check(pyExplicit), PyExc_TypeError, "`explicit` must be a bool");
            explicit = (pyExplicit == Py_True);
        }
        if (pyImplicitly && !Py_IsNone(pyImplicitly)) {
            __CHECK(PyObject_IsInstance(pyImplicitly, (PyObject *) &PyBTypeCls), PyExc_TypeError, "`implicitly` must be a BType");
            implicitid = ((PyBType *) pyImplicitly)->btypeid;
        }
        if (pySpace && !Py_IsNone(pySpace)) {
            __CHECK(PyObject_IsInstance(pySpace, (PyObject *) &PyBTypeCls), PyExc_TypeError, "`space` must be a BType");
            spaceid = ((PyBType *) pySpace)->btypeid;
        }
    }
//    if (spaceid) __CHECK(pyBType, PyExc_TypeError, "`btypeid` must be provided as well if `space` is provided");
    if (spaceid && (nextid=tm_space_would_deeply_recurse(pyTm->tm, btypeid, spaceid))) {
        return PyErr_Format(PyBTypeError, "PyTM_initAtom: btypeid = %i and spaceid = %i would deeply recurse", nextid, spaceid);
    }
    // PP(info, "PyTM_initAtom - #1 btypeid=%i, spaceid=%i", btypeid, spaceid);
    btypeid = tm_init_atom(pyTm->tm, btypeid, implicitid, explicit);
    __CHECK(btypeid, PyBTypeError, "error calling tm_init_atom(...)");
    if (spaceid) btypeid = tm_set_spaceid(pyTm->tm, btypeid, spaceid);
    __CHECK(btypeid, PyBTypeError, "error calling tm_set_spaceid(...)");
    return newPyBTypeRef(btypeid);
}

// ---------------------------------------------------------------------------------------------------------------------
// bind: (name:str, btype:PyBType) -> PyBType+PyException

pvt PyObject * PyTM_bind(PyTM *pyTm, PyObject **args, Py_ssize_t nargs) {
    // binds 'name' to btype and answers a BType ref, throwing BTypeError if the name has already been bound
    char const *name, *existingName;  btypeid_t btypeid, res;

    __CHECK(nargs == 2, PyExc_TypeError, "bind: (name) takes 2 args but %i %s given", nargs, nargs == 1 ? "was": "were");
    __CHECK(PyUnicode_Check(args[0]) && (PyUnicode_KIND(args[0]) == PyUnicode_1BYTE_KIND), PyExc_TypeError, "name must be a utf8 string");
    __CHECK(PyObject_IsInstance(args[1], (PyObject *) &PyBTypeCls), PyExc_TypeError, "btype must be a BType");

    name = (char *) PyUnicode_AsUTF8(args[0]);       // OPEN: why move from PyUnicode_1BYTE_DATA?
    btypeid = ((PyBType *) args[1])->btypeid;
//    PP(info, "PyTM_bind - #1 bind \"%s\" to %i", name, btypeid);
    if ((res = tm_bind(pyTm->tm, name, btypeid))) {
        // bound okay
//        PP(info, "PyTM_bind - #2a bind \"%s\" to %i", name, btypeid);
        return newPyBTypeRef(res);
    } else if ((existingName = tm_name_of(pyTm->tm, btypeid))) {
        if (strcmp(name, existingName) == 0) {
            // rebinding name to btype - effectively a no-op
//            PP(info, "PyTM_bind - #2b bind \"%s\" to %i", name, btypeid);
            return newPyBTypeRef(btypeid);
        } else {
            // another name refers to btype
//            PP(info, "PyTM_bind - #2c bind \"%s\" to %i", name, btypeid);
            return PyErr_Format(
                PyBTypeError,
                "Cannot bind \"%s\" to t%i since it is already bound to by \"%s\"", name, btypeid, existingName);
        }
    } else {
        // only other possibility is that name is already bound to another btype
//        PP(info, "PyTM_bind - #2d bind \"%s\" to %i", name, btypeid);
        btypeid = tm_lookup(pyTm->tm, name);
        return PyErr_Format(PyBTypeError, "\"%s\" is already bound to t%i", name, btypeid);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
// bmetatypeid: (btype) -> PyLong + PyException

pvt PyObject * PyTM_bmetatypeid(PyTM *pyTm, PyObject **args, Py_ssize_t nargs) {
    // answer the bmetatypeid of the given btype

    __CHECK(nargs == 1, PyExc_TypeError, "bmetatypeid(btype) takes 1 arg but %i %s given", nargs, nargs == 1 ? "was": "were");
    if (Py_IsNone(args[0])) return PyLong_FromLong(0);
    __CHECK(PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls), PyExc_TypeError, "btype must be a BType");

    return PyLong_FromLong(
        (long) tm_bmetatypeid(pyTm->tm, ((PyBType *) args[0])->btypeid) >> 28   // btatm starts at 0x10000000
    );
}

// ---------------------------------------------------------------------------------------------------------------------
// checkAtom: (btype:PyBType, **, explicit:Bool, implicitly:PyBType, space:PyBType) -> PyBType+PyException

pvt PyObject * PyTM_checkAtom(PyTM *pyTm, PyObject *const *args, Py_ssize_t nargs, PyObject *argnames) {
    // An existing atom is being defined a second time check that the attributes don't conflict
    //      a) if current.space is already set then space may be the same or missing
    //      b) if current.implicitly is already set then implicitly may be the same or missing
    //      c) if current.explicit is true then explicit may be True or missing
    PyObject *pySpace = 0, *pyExplicit = 0, *pyImplicitly = 0;
    btypeid_t btypeid, implicitid = 0, spaceid = 0;  bool explicit = false;

    __CHECK(nargs == 1, PyExc_TypeError, "checkAtom(btype, **, explicit, implicitly, space) takes 1 arg but %i %s given", nargs, nargs == 1 ? "was": "were");
    __CHECK(PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls), PyExc_TypeError, "btype must be a BType");
    btypeid = ((PyBType *) args[0])->btypeid;

    if (argnames) {
        for (int i = 0; i < PyTuple_Size(argnames); i++) {
            __GET_KWARG("explicit", PyTuple_GET_ITEM(argnames, i), pyExplicit = args[i + nargs])
            __OR_GET_KWARG("implicitly", PyTuple_GET_ITEM(argnames, i), pyImplicitly = args[i + nargs])
            __OR_GET_KWARG("space", PyTuple_GET_ITEM(argnames, i), pySpace = args[i + nargs])
            __ELSE_RAISE(PyExc_TypeError, "Unknown keyword argument \"%s\"", PyUnicode_AsUTF8(PyTuple_GET_ITEM(argnames, i)))
        }
        if (pyExplicit && !Py_IsNone(pyExplicit)) {
            __CHECK(PyBool_Check(pyExplicit), PyExc_TypeError, "explicit must be a bool or None");
            explicit = (pyExplicit == Py_True);
        }
        if (pyImplicitly && !Py_IsNone(pyImplicitly)) {
            __CHECK(PyObject_IsInstance(pyImplicitly, (PyObject *) &PyBTypeCls), PyExc_TypeError, "implicitly must be a BType or None");
            implicitid = ((PyBType *) pyImplicitly)->btypeid;
        }
        if (pySpace && !Py_IsNone(pySpace)) {
            __CHECK(PyObject_IsInstance(pySpace, (PyObject *) &PyBTypeCls), PyExc_TypeError, "space must be a BType or None");
            spaceid = ((PyBType *) pySpace)->btypeid;
        }
    }
    btypeid = tm_check_atom(pyTm->tm, btypeid, implicitid, explicit, spaceid);
    if (!btypeid) {
        return PyErr_Format(PyBTypeError, "error calling tm_check_atom(...)");
    } else {
        return newPyBTypeRef(btypeid);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
// checkIntersection: (btype:PyBType, **, space:PyBType) -> PyBType + PyException

#define __WAS_WERE(N)                                                                                       \
(((N) == 1) ? "was" : "were")

pvt PyObject * PyTM_checkIntersection(PyTM *pyTm, PyObject *const *args, Py_ssize_t nargs, PyObject *argnames) {
    // An existing intersection is being defined a second time check that the attributes don't conflict
    //      a) if current.space is already set then space may be the same or missing
    PyObject *pySpace = 0;  btypeid_t btypeid, spaceid = 0;

    __CHECK(nargs == 1, PyExc_TypeError, "checkIntersection(btype, **, space) takes 1 arg but %i %s given", nargs, __WAS_WERE(nargs));
    __CHECK(PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls), PyExc_TypeError, "btype must be a BType");
    btypeid = ((PyBType *) args[0])->btypeid;

    if (argnames) {
        for (int i = 0; i < PyTuple_Size(argnames); i++) {
            __GET_KWARG("space", PyTuple_GET_ITEM(argnames, i), pySpace = args[i + nargs])
            __ELSE_RAISE(PyExc_TypeError, "Unknown keyword argument \"%s\"", PyUnicode_AsUTF8(PyTuple_GET_ITEM(argnames, i)))
        }
        if (pySpace && !Py_IsNone(pySpace)) {
            __CHECK(PyObject_IsInstance(pySpace, (PyObject *) &PyBTypeCls), PyExc_TypeError, "space must be a BType or None");
            spaceid = ((PyBType *) pySpace)->btypeid;
        }
    }
    btypeid = tm_check_inter(pyTm->tm, btypeid, spaceid);
    if (!btypeid) return PyErr_Format(PyBTypeError, "error calling tm_check_intersection(...)");
    return newPyBTypeRef(btypeid);
}

// ---------------------------------------------------------------------------------------------------------------------
// fn: ((btype1, btype2, ...), tRet) -> PyBType + PyException

pvt PyObject * PyTM_fn(PyTM *pyTm, PyObject **args, Py_ssize_t nargs, PyObject *argnames) {
    // answer the fn type corresponding to tArgs and tRet
    BK_TP tp;  Buckets *buckets;  BucketsCheckpoint cp;  btypeid_t *tl, tArgs, tRet, btypeid = B_NEW;  int n;
    PyObject *tup, *e, *pyBType = 0;  TM_TLID_T tlid;

    if (nargs != 2) return PyErr_Format(PyExc_TypeError, "Must provide tArgs and tRet");

    if (argnames) {
        for (int i = 0; i < PyTuple_Size(argnames); i++) {
            __GET_KWARG("btype", PyTuple_GET_ITEM(argnames, i), pyBType = args[i + nargs])
            __ELSE_RAISE(PyExc_TypeError, "Unknown keyword argument \"%s\"", PyUnicode_AsUTF8(PyTuple_GET_ITEM(argnames, i)))
        }
        if (pyBType && !Py_IsNone(pyBType)) {
            __CHECK(PyObject_IsInstance(pyBType, (PyObject *) &PyBTypeCls), PyExc_TypeError, "btypeid must be a BType");
            btypeid = ((PyBType *) pyBType)->btypeid;
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
        tlid = tm_tlid_for(pyTm->tm, tl);
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

    btypeid = tm_fn(pyTm->tm, btypeid, tArgs, tRet);

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

pvt PyObject * PyTM_fnTArgs(PyTM *pyTm, PyObject **args, Py_ssize_t nargs) {
    // answer tArgs of the given tFn
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "btype is not a BType");
    btypeid_t tFn = ((PyBType *) args[0])->btypeid;
    if (tm_bmetatypeid(pyTm->tm, tFn) != bmtfnc) return PyErr_Format(PyBTypeError, "btype is not a fn type");
    return newPyBTypeRef(tm_fn_targs_tret(pyTm->tm, tFn).tArgs);
}

// ---------------------------------------------------------------------------------------------------------------------
// fnRetT: (tFn) -> tRet + PyException

pvt PyObject * PyTM_fnTRet(PyTM *pyTm, PyObject **args, Py_ssize_t nargs) {
    // answer tRet of the given tFn
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "btype is not a BType");
    btypeid_t tFn = ((PyBType *) args[0])->btypeid;
    if (tm_bmetatypeid(pyTm->tm, tFn) != bmtfnc) return PyErr_Format(PyBTypeError, "btype is not a fn type");
    return newPyBTypeRef(tm_fn_targs_tret(pyTm->tm, tFn).tRet);
}

// ---------------------------------------------------------------------------------------------------------------------
// fromId: (btypeid) -> PyBType + PyException

pvt PyObject * PyTM_fromId(PyTM *pyTm, PyObject **args, Py_ssize_t nargs) {
    // answer a PyBType given its btypeid
    int overflow;
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyLong_Check(args[0])) return PyErr_Format(PyExc_TypeError, "btypeid must be an int");
    btypeid_t btypeid = (btypeid_t) PyLong_AsLongAndOverflow(args[0], &overflow);
    if (btypeid == B_NAT) return newPyBTypeRef(btypeid);;
    if (overflow != 0 || btypeid < TM_FIRST_VALID_BTYPEID || btypeid >= pyTm->tm->next_btypeId) return PyErr_Format(PyBTypeError, "btypeid is outside the range of all BTypes");
    return newPyBTypeRef(btypeid);
}

// ---------------------------------------------------------------------------------------------------------------------
// hasT: (btype) -> PyBool + PyException

pvt PyObject * PyTM_hasT(PyTM *pyTm, PyObject **args, Py_ssize_t nargs) {
    // answer if the given btype contains a schemavar
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "btype is not a BType");
    return tm_hasT(pyTm->tm, ((PyBType *) args[0])->btypeid) ? Py_NewRef(Py_True) : Py_NewRef(Py_False);
}

// ---------------------------------------------------------------------------------------------------------------------
// intersection: (btype1, btype2, ... ** btype, space) -> PyBType + PyException

pvt PyObject * PyTM_intersectionImpl(PyTM *pyTm, PyObject *const *args, Py_ssize_t nargs, PyObject *argnames, bool check) {
    // answer a new intersection of btype1, btype2, ...
    BK_TP tp;  Buckets *buckets;  BucketsCheckpoint cp;  PyObject *pyBType = 0, *pySpace = 0;
    btypeid_t *tl, btypeid = B_NEW, spaceid = 0, nextid;

    __CHECK(nargs > 0, PyExc_TypeError, "checkIntersection(*btype, **, btype, space) takes at least 1 btype but %i %s given", nargs, __WAS_WERE(nargs));
    if (nargs == 1) {
        if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "First arg is not a BType");
        return Py_NewRef(args[0]);
    }
    if (argnames) {
        for (int i = 0; i < PyTuple_Size(argnames); i++) {
            __GET_KWARG("btype", PyTuple_GET_ITEM(argnames, i), pyBType = args[i + nargs])
            __OR_GET_KWARG("space", PyTuple_GET_ITEM(argnames, i), pySpace = args[i + nargs])
            __ELSE_RAISE(PyExc_TypeError, "Unknown keyword argument \"%s\"", PyUnicode_AsUTF8(PyTuple_GET_ITEM(argnames, i)))
        }
        if (pyBType && !Py_IsNone(pyBType)) {
            __CHECK(PyObject_IsInstance(pyBType, (PyObject *) &PyBTypeCls), PyExc_TypeError, "btype must be a BType or None");
            btypeid = ((PyBType *) pyBType)->btypeid;
        }
        if (pySpace && !Py_IsNone(pySpace)) {
            __CHECK(PyObject_IsInstance(pySpace, (PyObject *) &PyBTypeCls), PyExc_TypeError, "space must be a BType or None");
            spaceid = ((PyBType *) pySpace)->btypeid;
        }
    }

    if (spaceid && (nextid = tm_space_would_deeply_recurse(pyTm->tm, btypeid, spaceid))) {
        return PyErr_Format(PyBTypeError, "PyTM_intersection: btypeid = %i and spaceid = %i would deeply recurse", nextid, spaceid);
    }

    // PP(info, "PyTM_intersection - #1 btypeid=%i, spaceid=%i", btypeid, spaceid);
    checkpointBuckets((buckets = pyTm->tm->buckets), &cp);
    // PP(info, "PyTM_intersection - #2");

    // create a type list of the correct length
    tl = (btypeid_t *) allocInBuckets(buckets, ((1 + nargs) * sizeof(btypeid_t)), bk_alignof(btypeid_t));
    // PP(info, "PyTM_intersection - #3");
    tl[0] = (btypeid_t) nargs;
    for (int i=1; i <= nargs; i++) {

        if (!PyObject_IsInstance(args[i-1], (PyObject *) &PyBTypeCls)) {
            resetToCheckpoint(buckets, &cp);
            return PyErr_Format(PyExc_TypeError, "arg%i is not a BType", i);
        }
        PyBType *pybtype = (PyBType *) args[i-1];
        tl[i] = pybtype->btypeid;
        // PP(info, "PyTM_intersection - #4 - tl[%i] = t%i, is PyBType=%i", i, tl[i], PyObject_IsInstance(args[i-1], (PyObject *) &PyBTypeCls));
    }

    // PP(info, "PyTM_intersection - #5 - btypeid: %i", btypeid);
    btypeid = tm_inter_impl(pyTm->tm, btypeid, tl, check);

    if (btypeid) {
        // PP(info, "PyTM_intersection - #6");
        if (spaceid) btypeid = tm_set_spaceid(pyTm->tm, btypeid, spaceid);       // OPEN: move this to tm_inter
        if (btypeid) {
            resetToCheckpoint(buckets, &cp);
            // PP(info, "PyTM_intersection - #7");
            return newPyBTypeRef(btypeid);
        }
    }
    TP_init(&tp, 0, buckets);
    // PP(info, "PyTM_intersection - #8 - btypeid: %i", btypeid);
    PyErr_Format(PyBTypeError, "There are exclusion conflicts within (%s)", tm_s8_typelist(pyTm->tm, &tp, tl).cs);
    resetToCheckpoint(buckets, &cp);
    return 0;
}

pvt PyObject * PyTM_intersection(PyTM *pyTm, PyObject *const *args, Py_ssize_t nargs, PyObject *argnames) {
    return PyTM_intersectionImpl(pyTm, args, nargs, argnames, true);
}

// ---------------------------------------------------------------------------------------------------------------------
// intersectionNoCheck: (btype1, btype2, ...) -> PyBType + PyException

pvt PyObject * PyTM_intersectionNoCheck(PyTM *pyTm, PyObject *const *args, Py_ssize_t nargs) {
    return PyTM_intersectionImpl(pyTm, args, nargs, 0, false);
}

// ---------------------------------------------------------------------------------------------------------------------
// intersectionTl: (btype) -> (PyBype1, ...) + PyException

pvt PyObject * PyTM_intersectionTl(PyTM *pyTm, PyObject **args, Py_ssize_t nargs) {
    // answer a new PyTuple of new PyBTypes which is the type list of the given intersection btype
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "btype is not a BType");
    btypeid_t *tl = tm_inter_tl(pyTm->tm, ((PyBType *) args[0])->btypeid);
    if (tl == 0) return PyErr_Format(PyBTypeError, "btype is not an intersection type");
    PyObject *answer = PyTuple_New(tl[0]);
    for (btypeid_t i = 1; i <= tl[0]; i++) {
        PyTuple_SET_ITEM(answer, i - 1, newPyBTypeRef(tl[i]));
    }
    return answer;
}

// ---------------------------------------------------------------------------------------------------------------------
// intersectionTlidFor: (btype1, btype2, ...) -> PyLong + PyException

pvt PyObject * PyTM_intersectionTlidFor(PyTM *pyTm, PyObject *const *args, Py_ssize_t nargs) {
    // answer the intersection tlid of btype1, btype2, ...
    BK_TP tp;  Buckets *buckets;  BucketsCheckpoint cp;  TM_TLID_T tlid;  btypeid_t *tl;

    __CHECK(nargs > 0, PyExc_TypeError, "checkIntersection(*btype, **, btype, space) takes at least 1 btype but %i %s given", nargs, __WAS_WERE(nargs));
    if (nargs == 1) {
        if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "btype1 is not a BType");
        return Py_NewRef(args[0]);
    }

    // PP(info, "PyTM_intersectionTlidFor - #1 btypeid=%i, spaceid=%i", btypeid, spaceid);
    checkpointBuckets((buckets = pyTm->tm->buckets), &cp);
    // PP(info, "PyTM_intersectionTlidFor - #2");

    // create a type list of the correct length
    tl = (btypeid_t *) allocInBuckets(buckets, ((1 + nargs) * sizeof(btypeid_t)), bk_alignof(btypeid_t));
    // PP(info, "PyTM_intersectionTlidFor - #3");
    tl[0] = (btypeid_t) nargs;
    for (int i=1; i <= nargs; i++) {

        if (!PyObject_IsInstance(args[i-1], (PyObject *) &PyBTypeCls)) {
            resetToCheckpoint(buckets, &cp);
            return PyErr_Format(PyExc_TypeError, "arg%i is not a BType", i);
        }
        PyBType *pybtype = (PyBType *) args[i-1];
        tl[i] = pybtype->btypeid;
        // PP(info, "PyTM_intersectionTlidFor - #4 - tl[%i] = t%i, is PyBType=%i", i, tl[i], PyObject_IsInstance(args[i-1], (PyObject *) &PyBTypeCls));
    }

    // PP(info, "PyTM_intersectionTlidFor - #5 - btypeid: %i", btypeid);
    tlid = tm_inter_tlid_for(pyTm->tm, tl);

    if (tlid) {
        // PP(info, "PyTM_intersectionTlidFor - #6");
        resetToCheckpoint(buckets, &cp);
        // PP(info, "PyTM_intersectionTlidFor - #7");
        return PyLong_FromLong(tlid);
    }
    TP_init(&tp, 0, buckets);
    // PP(info, "PyTM_intersectionTlidFor - #8 - btypeid: %i", btypeid);
    PyErr_Format(PyBTypeError, "There are exclusion conflicts within (%s)", tm_s8_typelist(pyTm->tm, &tp, tl).cs);
    resetToCheckpoint(buckets, &cp);
    return 0;
}

// ---------------------------------------------------------------------------------------------------------------------
// isExplicit: (btype) -> PyBool + PyException

pvt PyObject * PyTM_isExplicit(PyTM *pyTm, PyObject **args, Py_ssize_t nargs) {
    // answer if the given btype requires an explicit match
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "btype is not a BType");
    bool explicit = TM_IS_EXPLICIT(pyTm->tm->btsummary_by_btypeid[((PyBType *) args[0])->btypeid]);
    return PyBool_FromLong(explicit);
}

// ---------------------------------------------------------------------------------------------------------------------
// isRecursive: (btype) -> PyBool + PyException

pvt PyObject * PyTM_isRecursive(PyTM *pyTm, PyObject **args, Py_ssize_t nargs) {
    // answer if the given btype is recursive
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "btype is not a BType");
    bool isRec = TM_IS_RECURSIVE(pyTm->tm->btsummary_by_btypeid[((PyBType *) args[0])->btypeid]);
    return PyBool_FromLong(isRec);
}

// ---------------------------------------------------------------------------------------------------------------------
// lookup: (name:str) -> PyBType + PyException

pvt PyObject * PyTM_lookup(PyTM *pyTm, PyObject **args, Py_ssize_t nargs) {
    // answer the BType for the given name, or B_NAT if the name has not been bound
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyUnicode_Check(args[0]) || (PyUnicode_KIND(args[0]) != PyUnicode_1BYTE_KIND)) return PyErr_Format(PyExc_TypeError, "name must be utf8");
    char *name = (char *) PyUnicode_AsUTF8(args[0]);
//    PP(info, "PyTM_lookup #1 - name: \"%s\"", name);
    btypeid_t btypeid = tm_lookup(pyTm->tm, name);
//    PP(info, "PyTM_lookup #2 - btypeid: %i", btypeid);
    return newPyBTypeRef(btypeid);
}

// ---------------------------------------------------------------------------------------------------------------------
// map: (tK, tV, **, btype) -> tMap + PyException

pvt PyObject * PyTM_map(PyTM *pyTm, PyObject **args, Py_ssize_t nargs, PyObject *argnames) {
    // answer the tMap corresponding to tK and tV
    BK_TP tp;  Buckets *buckets;  BucketsCheckpoint cp;  btypeid_t tK, tV, tMap, btypeid = B_NEW;  PyObject *pyBType = 0;

    if (nargs != 2) return PyErr_Format(PyExc_TypeError, "Must provide tK and tV");
    if (argnames) {
        for (int i = 0; i < PyTuple_Size(argnames); i++) {
            __GET_KWARG("btype", PyTuple_GET_ITEM(argnames, i), pyBType = args[i + nargs])
            __ELSE_RAISE(PyExc_TypeError, "Unknown keyword argument \"%s\"", PyUnicode_AsUTF8(PyTuple_GET_ITEM(argnames, i)))
        }
        if (pyBType && !Py_IsNone(pyBType)) {
            __CHECK(PyObject_IsInstance(pyBType, (PyObject *) &PyBTypeCls), PyExc_TypeError, "btypeid must be a BType");
            btypeid = ((PyBType *) pyBType)->btypeid;
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

    tMap = tm_map(pyTm->tm, btypeid, tK, tV);

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

pvt PyObject * PyTM_mapTK(PyTM *pyTm, PyObject **args, Py_ssize_t nargs) {
    // answer tK of the given tMap
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "btype is not a BType");
    btypeid_t tMap = ((PyBType *) args[0])->btypeid;
    if (tm_bmetatypeid(pyTm->tm, tMap) != bmtmap) return PyErr_Format(PyBTypeError, "btype is not a map type");
    return newPyBTypeRef(tm_map_tk_tv(pyTm->tm, tMap).tK);
}

// ---------------------------------------------------------------------------------------------------------------------
// mapTV: (tMap) -> tV + PyException

pvt PyObject * PyTM_mapTV(PyTM *pyTm, PyObject **args, Py_ssize_t nargs) {
    // answer tV of the given tMap
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "btype is not a BType");
    btypeid_t tMap = ((PyBType *) args[0])->btypeid;
    if (tm_bmetatypeid(pyTm->tm, tMap) != bmtmap) return PyErr_Format(PyBTypeError, "btype is not a map type");
    return newPyBTypeRef(tm_map_tk_tv(pyTm->tm, tMap).tV);
}

// ---------------------------------------------------------------------------------------------------------------------
// minus: (btype, btype) -> btype + PyException

pvt PyObject * PyTM_minus(PyTM *pyTm, PyObject **args, Py_ssize_t nargs) {
    // answer the name of the given btype
    if (nargs != 2) return jErrWrongNumberOfArgs(FN_NAME, 2, nargs);
    if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "A is not a BType");
    bmetatypeid_t bmetatypeid = tm_bmetatypeid(pyTm->tm, ((PyBType *) args[0])->btypeid);
    if (bmetatypeid != bmtint && bmetatypeid != bmtuni) return PyErr_Format(PyBTypeError, "A is not an intersection nor a union");
    if (!PyObject_IsInstance(args[1], (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "B is not a BType");
    btypeid_t btypeid = tm_minus(pyTm->tm, B_NEW, ((PyBType *) args[0])->btypeid, ((PyBType *) args[1])->btypeid);
    if (btypeid == 0)
        return PyErr_Format(PyBTypeError, "Error doing A minus B.");
    else
        return newPyBTypeRef(btypeid);
}

// ---------------------------------------------------------------------------------------------------------------------
// name: (btype) -> PyStr + PyException

pvt PyObject * PyTM_nameOf(PyTM *pyTm, PyObject **args, Py_ssize_t nargs) {
    // answer the name of the given btype or None if not bound
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "btype is not a BType");
    // OPEN: what to do if there is no name (use t123?) - 0 means invalid type?
    char const *name = tm_name_of(pyTm->tm, ((PyBType *) args[0])->btypeid);
    if (name == 0) Py_RETURN_NONE;
    return PyUnicode_FromString(name);
}

// ---------------------------------------------------------------------------------------------------------------------
// space: (btype) -> PyBType + None

pvt PyObject * PyTM_space(PyTM *pyTm, PyObject **args, Py_ssize_t nargs) {
    // answer the space of the given btype
    btypeid_t btypeid;
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "btype is not a BType");
    // OPEN: what to do if there is no name (use t123?) - 0 means invalid type?
    btypeid = tm_spaceid(pyTm->tm, ((PyBType *) args[0])->btypeid);
    if (btypeid == B_NAT) Py_RETURN_NONE;
    return newPyBTypeRef(btypeid);   // OPEN: something like PyBType_FromBTypeId(btypeid)?
}

// ---------------------------------------------------------------------------------------------------------------------
// rootSpace: (btype) -> PyBType + None

pvt PyObject * PyTM_rootSpace(PyTM *pyTm, PyObject **args, Py_ssize_t nargs) {
    // answer the space of the given btype
    btypeid_t btypeid;
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "btype is not a BType");
    // OPEN: what to do if there is no name (use t123?) - 0 means invalid type?
    btypeid = tm_root_spaceid(pyTm->tm, ((PyBType *) args[0])->btypeid);
    if (btypeid == B_NAT) Py_RETURN_NONE;
    return newPyBTypeRef(btypeid);   // OPEN: something like PyBType_FromBTypeId(btypeid)?
}

// ---------------------------------------------------------------------------------------------------------------------
// reserve: (*, btype=Missing, space=Missing) -> PyBType + PyException

pvt PyObject * PyTM_reserve(PyTM *pyTm, PyObject *const *args, Py_ssize_t nargs, PyObject *argnames) {
    // answers a new uninitialised btype with the given options
    PyObject *pyBType = 0, *pySpace = 0;
    btypeid_t btypeid = B_NEW, spaceid = 0;

    __CHECK(nargs == 0, PyExc_TypeError, "options(**kwargs) takes no args but %i were given", nargs);

    if (argnames) {
        for (int i = 0; i < PyTuple_Size(argnames); i++) {
            __GET_KWARG("btype", PyTuple_GET_ITEM(argnames, i), pyBType = args[i + nargs])
            __OR_GET_KWARG("space", PyTuple_GET_ITEM(argnames, i), pySpace = args[i + nargs])
            __ELSE_RAISE(PyExc_TypeError, "Unknown keyword argument \"%s\"", PyUnicode_AsUTF8(PyTuple_GET_ITEM(argnames, i)))
        }
        if (pyBType && !Py_IsNone(pyBType)) {
            __CHECK(PyObject_IsInstance(pyBType, (PyObject *) &PyBTypeCls), PyExc_TypeError, "btype must be a BType");
            btypeid = ((PyBType *) pyBType)->btypeid;
        }
        if (pySpace && !Py_IsNone(pySpace)) {
            __CHECK(PyObject_IsInstance(pySpace, (PyObject *) &PyBTypeCls), PyExc_TypeError, "space must be a BType or None");
            spaceid = ((PyBType *) pySpace)->btypeid;
        }
    }

    if (spaceid && tm_space_would_deeply_recurse(pyTm->tm, btypeid, spaceid)) return PyErr_Format(PyBTypeError, "tm_space_would_deeply_recurse");
    btypeid = (btypeid == B_NEW) ? tm_reserve_tbc(pyTm->tm) : tm_set_tbc(pyTm->tm, btypeid);
    if (spaceid) btypeid = tm_set_spaceid(pyTm->tm, btypeid, spaceid);
    if (btypeid)
        return newPyBTypeRef(btypeid);
    else
        return PyErr_Format(PyBTypeError, "error creating options");
}

// ---------------------------------------------------------------------------------------------------------------------
// schemavar: (*, btype=Missing) -> PyBType + PyException

pvt PyObject * PyTM_schemavar(PyTM *pyTm, PyObject **args, Py_ssize_t nargs, PyObject *argnames) {
    // answer a new schema variable with the given name, or an exception if already taken
    PyObject *pyBType;  btypeid_t btypeid = B_NEW;
    if (nargs != 0) return jErrWrongNumberOfArgs(FN_NAME, 0, nargs);
    if (argnames) {
        for (int i = 0; i < PyTuple_Size(argnames); i++) {
            __GET_KWARG("btype", PyTuple_GET_ITEM(argnames, i), pyBType = args[i + nargs])
            __ELSE_RAISE(PyExc_TypeError, "Unknown keyword argument \"%s\"", PyUnicode_AsUTF8(PyTuple_GET_ITEM(argnames, i)))
        }
        if (pyBType && !Py_IsNone(pyBType)) {
            __CHECK(PyObject_IsInstance(pyBType, (PyObject *) &PyBTypeCls), PyExc_TypeError, "btype must be a BType");
            btypeid = ((PyBType *) pyBType)->btypeid;
        }
    }
    btypeid = tm_schemavar(pyTm->tm, btypeid);
    __CHECK(btypeid, PyBTypeError, "error calling tm_schemavar(...)");
    return newPyBTypeRef(btypeid);
}

// ---------------------------------------------------------------------------------------------------------------------
// seq: (contained:btype, *, btype=Missing) -> PyBType + PyException

pvt PyObject * PyTM_seq(PyTM *pyTm, PyObject **args, Py_ssize_t nargs, PyObject *argnames) {
    // answer a new sequence for the given contained btype
    BK_TP tp;  Buckets *buckets;  BucketsCheckpoint cp;  btypeid_t btypeid = B_NEW;  PyObject *pyBType;

    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (argnames) {
        for (int i = 0; i < PyTuple_Size(argnames); i++) {
            __GET_KWARG("btype", PyTuple_GET_ITEM(argnames, i), pyBType = args[i + nargs])
            __ELSE_RAISE(PyExc_TypeError, "Unknown keyword argument \"%s\"", PyUnicode_AsUTF8(PyTuple_GET_ITEM(argnames, i)))
        }
        if (pyBType && !Py_IsNone(pyBType)) {
            __CHECK(PyObject_IsInstance(pyBType, (PyObject *) &PyBTypeCls), PyExc_TypeError, "btype must be a BType or None");
            btypeid = ((PyBType *) pyBType)->btypeid;
        }
    }
    checkpointBuckets((buckets = pyTm->tm->buckets), &cp);

    if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) {
        resetToCheckpoint(buckets, &cp);
        return PyErr_Format(PyExc_TypeError, "arg is not a BType");
    }

    btypeid = tm_seq(pyTm->tm, btypeid, ((PyBType *) args[0])->btypeid);

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

pvt PyObject * PyTM_seqT(PyTM *pyTm, PyObject **args, Py_ssize_t nargs) {
    // answer the btype of the contained type for the given sequence type
    btypeid_t btypeid;
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "btype is not a BType");
    btypeid = tm_seq_t(pyTm->tm, ((PyBType *) args[0])->btypeid);
    if (btypeid == 0) return PyErr_Format(PyBTypeError, "btype is not a sequence type");
    return newPyBTypeRef(btypeid);
}

// ---------------------------------------------------------------------------------------------------------------------
// struct: (names, btypes, *, btype) -> PyBType + PyException

pvt PyObject * PyTM_struct(PyTM *pyTm, PyObject **args, Py_ssize_t nargs, PyObject *argnames) {
    // answers a new struct type given the names and btypes
    BK_TP tp;  Buckets *buckets;  BucketsCheckpoint cp;  btypeid_t *tl;  symid_t *sl, btypeid = B_NEW;  int n;
    PyObject *s, *btype, *pyBType = 0;  char const *name;

    if (nargs != 2) return PyErr_Format(PyExc_TypeError, "Must provide 2 args: names and btypes");
    if (argnames) {
        for (int i = 0; i < PyTuple_Size(argnames); i++) {
            __GET_KWARG("btype", PyTuple_GET_ITEM(argnames, i), pyBType = args[i + nargs])
            __ELSE_RAISE(PyExc_TypeError, "Unknown keyword argument \"%s\"", PyUnicode_AsUTF8(PyTuple_GET_ITEM(argnames, i)))
        }
        if (pyBType && !Py_IsNone(pyBType)) {
            __CHECK(PyObject_IsInstance(pyBType, (PyObject *) &PyBTypeCls), PyExc_TypeError, "btype must be a BType");
            btypeid = ((PyBType *) pyBType)->btypeid;
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
    TM_TLID_T tlid = tm_tlid_for(pyTm->tm, tl);
    btypeid = tm_struct(pyTm->tm, btypeid, slid, tlid);

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
// structNames: btype -> (sym1, ...) + PyException

pvt PyObject * PyTM_structNames(PyTM *pyTm, PyObject **args, Py_ssize_t nargs) {
    // answer a new PyTuple of str - the field names of the given struct btype
    symid_t *sl;  PyObject *answer, *pyName;  SM_SLID_T slid;  BK_TM *tm;  const char *name;
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "btype is not a BType");
    if (tm_bmetatypeid(tm=pyTm->tm, ((PyBType *) args[0])->btypeid) != bmtstr) return PyErr_Format(PyBTypeError, "btype is not a struct type");
    slid = tm_struct_slid(tm, ((PyBType *) args[0])->btypeid);
    if (slid == 0) return PyErr_Format(PyBTypeError, "btype has no fields");
    sl = tm->sm->symlist_buf + tm->sm->slrp_by_slid[slid];
    if (sl == 0) return PyErr_Format(PyBTypeError, "btype has no fields");
    answer = PyTuple_New(sl[0]);
    for (btypeid_t i = 1; i <= sl[0]; i++) {
        name = sm_name(tm->sm, sl[i]);
        if (name == 0) {
            PyErr_Format(PyBTypeError, "field%i has no name (memory corruption?)", i);
            Py_DECREF(answer);
            return 0;
        }
        pyName = PyUnicode_FromString(name);
        PyTuple_SET_ITEM(answer, i - 1, pyName);
    }
    return answer;
}

// ---------------------------------------------------------------------------------------------------------------------
// structSl: btype -> (sym1, ...) + PyException

pvt PyObject * PyTM_structSl(PyTM *pyTm, PyObject **args, Py_ssize_t nargs) {
    // answer a new PyTuple of PySym - the field names of the given struct btype
    Py_RETURN_NONE;
}

// ---------------------------------------------------------------------------------------------------------------------
// structTl: btype -> (PyBType1, ...) + PyException

pvt PyObject * PyTM_structTl(PyTM *pyTm, PyObject **args, Py_ssize_t nargs) {
    // answer a new PyTuple of PyBType - the field types of the given struct btype
    btypeid_t *tl;  PyObject *answer;  TM_TLID_T tlid;
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "btype is not a BType");
    if (tm_bmetatypeid(pyTm->tm, ((PyBType *) args[0])->btypeid) != bmtstr) return PyErr_Format(PyBTypeError, "btype is not a struct type");
    tlid = tm_struct_tlid(pyTm->tm, ((PyBType *) args[0])->btypeid);
    if (tlid == 0) return PyErr_Format(PyBTypeError, "btype has no fields");
    tl = pyTm->tm->typelist_buf + pyTm->tm->tlrp_by_tlid[tlid];
    if (tl == 0) return PyErr_Format(PyBTypeError, "btype has no fields");
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
    btypeid_t btypeid = B_NEW;  PyObject *pyBType = 0;
    BK_TP tp;  Buckets *buckets;  BucketsCheckpoint cp;  btypeid_t *tl;
    if (argnames) {
        for (int i = 0; i < PyTuple_Size(argnames); i++) {
            __GET_KWARG("btype", PyTuple_GET_ITEM(argnames, i), pyBType = args[i + nargs])
            __ELSE_RAISE(PyExc_TypeError, "Unknown keyword argument \"%s\"", PyUnicode_AsUTF8(PyTuple_GET_ITEM(argnames, i)))
        }
        if (pyBType && !Py_IsNone(pyBType)) {
            __CHECK(PyObject_IsInstance(pyBType, (PyObject *) &PyBTypeCls), PyExc_TypeError, "btype must be a BType or None");
            btypeid = ((PyBType *) pyBType)->btypeid;
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

    btypeid = tm_tuple(pyTm->tm, btypeid, tm_tlid_for(pyTm->tm, tl));    // OPEN: don't do this as it may create a tuple with the wrong attributes

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

pvt PyObject * PyTM_tupleTl(PyTM *pyTm, PyObject **args, Py_ssize_t nargs) {
    btypeid_t *tl;  PyObject *answer;
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "btype is not a BType");
    tl = pyTm->tm->typelist_buf + pyTm->tm->tlrp_by_tlid[tm_tuple_tlid(pyTm->tm, ((PyBType *) args[0])->btypeid)];
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
    BK_TP tp;  Buckets *buckets;  BucketsCheckpoint cp;  btypeid_t btypeid = B_NEW;  PyObject *pyBType = 0;
    if (nargs == 0) return PyErr_Format(PyExc_TypeError, "Must provide at least one type");
    if (nargs == 1) {
        if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "arg is not a BType");
        return Py_NewRef(args[0]);
    }
    if (argnames) {
        for (int i = 0; i < PyTuple_Size(argnames); i++) {
            __GET_KWARG("btype", PyTuple_GET_ITEM(argnames, i), pyBType = args[i + nargs])
            __ELSE_RAISE(PyExc_TypeError, "Unknown keyword argument \"%s\"", PyUnicode_AsUTF8(PyTuple_GET_ITEM(argnames, i)))
        }
        if (pyBType && !Py_IsNone(pyBType)) {
            __CHECK(PyObject_IsInstance(pyBType, (PyObject *) &PyBTypeCls), PyExc_TypeError, "btype must be a BType or None");
            btypeid = ((PyBType *) pyBType)->btypeid;
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

    btypeid = tm_union(pyTm->tm, btypeid, tl);

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
    BK_TP tp;  Buckets *buckets;  BucketsCheckpoint cp;  btypeid_t btypeid = B_NEW;  btypeid_t *tl;  TM_TLID_T tlid;
    PyObject *pyBType = 0;

    if (nargs != 1) return PyErr_Format(PyExc_TypeError, "Must provide at least one type");
    if (!PyLong_Check(args[0])) return PyErr_Format(PyExc_TypeError, "tlid must be int");

    if (argnames) {
        for (int i = 0; i < PyTuple_Size(argnames); i++) {
            __GET_KWARG("btype", PyTuple_GET_ITEM(argnames, i), pyBType = args[i + nargs])
            __ELSE_RAISE(PyExc_TypeError, "Unknown keyword argument \"%s\"", PyUnicode_AsUTF8(PyTuple_GET_ITEM(argnames, i)))
        }
        if (pyBType && !Py_IsNone(pyBType)) {
            __CHECK(PyObject_IsInstance(pyBType, (PyObject *) &PyBTypeCls), PyExc_TypeError, "btype must be a BType or None");
            btypeid = ((PyBType *) pyBType)->btypeid;
        }
    }
    tlid = PyLong_AsLong(args[0]);
    btypeid = tm_union_for_tlid_or_create(pyTm->tm, btypeid, tlid);

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

pvt PyObject * PyTM_union_get_for_tlid(PyTM *pyTm, PyObject **args, Py_ssize_t nargs) {
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
        return PyErr_Format(PyExc_TypeError, "union_tlid_for does not take keyword args");
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

    TM_TLID_T tlid = tm_union_tlid_for(pyTm->tm, tl);

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

pvt PyObject * PyTM_unionTl(PyTM *pyTm, PyObject **args, Py_ssize_t nargs) {
    // answer a new PyTuple of new PyBTypes which is the type list of the given union
    btypeid_t *tl;  PyObject *answer;
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyObject_IsInstance(args[0], (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "btype is not a BType");
    tl = tm_union_tl(pyTm->tm, ((PyBType *) args[0])->btypeid);
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
    {"bind",                (PyCFunction) PyTM_bind, METH_FASTCALL,
        "bind(name, btype)\n\n"
        "Answers `btype` after binding `name` to it, raising an error if `name` is already bound to another btype."
    },
    {"bmetatypeid",         (PyCFunction) PyTM_bmetatypeid, METH_FASTCALL,
        "bmetatypeid(btype)\n\n"
        "Answers the bmetatypeid of `btype`."
    },
    {"checkAtom",           (PyCFunction) PyTM_checkAtom, METH_FASTCALL | METH_KEYWORDS,
        "checkAtom(btype, *, [explicit], [implicitly], [space])\n\n"
        "Checks that the atom `btype` is compatible with explicit, implicitly and space, returning `btype` if so else "
        "raising a BTypeError if not."
    },
    {"checkIntersection",   (PyCFunction) PyTM_checkIntersection, METH_FASTCALL | METH_KEYWORDS,
        "checkIntersection(*, btype, [space])\n\n"
        "Checks that intersection `btype` is compatible with `space` returning `btype` if so else raising a BTypeError if not."
    },
    {"fn",                  (PyCFunction) PyTM_fn, METH_FASTCALL | METH_KEYWORDS,
        "fn((t1, t2, ...), tRet, *, btype=Missing,)\n\n"
        "Answers the function btype for (`t1` * `t2` * ...) -> `tRet`."
    },
    {"fnTArgs",             (PyCFunction) PyTM_fnTArgs, METH_FASTCALL,
        "fnTArgs(tFn)\n\n"
        "Answers a tuple of the argument btypes of function `tFn`."
    },
    {"fnTRet",              (PyCFunction) PyTM_fnTRet, METH_FASTCALL,
        "fnRetT(tFn)\n\n"
        "Answers tRet for the function `tFn`."
    },
    {"fromId",              (PyCFunction) PyTM_fromId, METH_FASTCALL,
        "fromId(btypeid)\n\n"
        "Answers the btype corresponding to `btypeid` if it exists, throwing an error if not."
    },
    {"hasT",                (PyCFunction) PyTM_hasT, METH_FASTCALL,
        "hasT(btype)\n\n"
        "Answers True if `btype` has a schema variable."
    },
    {"initAtom",            (PyCFunction) PyTM_initAtom, METH_FASTCALL | METH_KEYWORDS,
        "initAtom(*, explicit, implicitly, space, btype=Missing)\n\n"
        "Creates a new atom or initialises `btype` with the provided properties, raising a BTypeError if `btype` is already initialised."
    },
    {"intersection",        (PyCFunction) PyTM_intersection, METH_FASTCALL | METH_KEYWORDS,
        "intersection(t1, t2, ..., *, btype=Missing, space=Missing)\n\n"
        "Answers the intersection of `t1` & `t2` & ..."
    },
    {"intersectionNoCheck", (PyCFunction) PyTM_intersectionNoCheck, METH_FASTCALL,
        "intersectionNoCheck(t1, t2, ...)\n\n"
        "Answers the intersection of `t1` & `t2` & ..."
    },
    {"intersectionTl",      (PyCFunction) PyTM_intersectionTl, METH_FASTCALL,
        "intersectionTl(btype)\n\n"
        "Answers a tuple of the btypes in the intersection `btype`."
    },
   {"intersectionTlidFor",  (PyCFunction) PyTM_intersectionTlidFor, METH_FASTCALL,
       "intersectionTlidFor(t1, t2, ...)\n\n"
       "Answers an intersection typelist id for the given tuple of types."
   },
    {"isExplicit",          (PyCFunction) PyTM_isExplicit, METH_FASTCALL,
        "isExplicit(btype)\n\n"
        "Answers True if `btype` requires an explicit match explicit."
    },
    {"isRecursive",         (PyCFunction) PyTM_isRecursive, METH_FASTCALL,
        "isRecursive(btype)\n\n"
        "Answers True if `btype` is recursive."
    },
    {"lookup",              (PyCFunction) PyTM_lookup, METH_FASTCALL,
        "lookup(name)\n\n"
        "Answers the btype that `name` is bound to else B_NAT if unbound."
    },
    {"map",                 (PyCFunction) PyTM_map, METH_FASTCALL | METH_KEYWORDS,
        "map(tK, tV, *, btype=Missing)\n\n"
        "Answers the btype of the map `tK` -> `tV` creating if necessary."
    },
    {"mapTK",               (PyCFunction) PyTM_mapTK, METH_FASTCALL,
        "mapTK(tMap)\n\n"
        "Answers tK of `tMap`."
    },
    {"mapTV",               (PyCFunction) PyTM_mapTV, METH_FASTCALL,
        "mapTV(tMap)\n\n"
        "Answers tV of `tMap`."
    },
    {"minus",               (PyCFunction) PyTM_minus, METH_FASTCALL,
        "minus(tA, tB)\n\n"
        "Answers the type of `tA` minus `tB`."
    },
    {"nameOf",              (PyCFunction) PyTM_nameOf, METH_FASTCALL,
        "nameOf(btype)\n\n"
        "Answers the name of `btype` if bound or None if not bound."
    },
    {"space",               (PyCFunction) PyTM_space, METH_FASTCALL,
        "space(btype)\n\n"
        "Answers the space of `btype` or None."
    },
    {"replaceWith",         (PyCFunction) PyTM_replaceWith, METH_FASTCALL,
        "replaceWith(btype, pybtype)\n\n"
        "Replaces the C btype object with a Python btype object and answers the latter."
    },
    {"reserve",             (PyCFunction) PyTM_reserve, METH_FASTCALL | METH_KEYWORDS,
        "reserve(*, btype=Missing, space=Missing)\n\n"
        "Answers an uninitialised recursive type to be initialised later - optionally with `name` & `space`."
    },
    {"rootSpace",           (PyCFunction) PyTM_rootSpace, METH_FASTCALL,
        "rootSpace(btype)\n\n"
        "Answers the root space of `btype` or None if it is not in a space."
    },
    {"schemavar",           (PyCFunction) PyTM_schemavar, METH_FASTCALL | METH_KEYWORDS,
        "schemavar(*, btype=Missing)\n\n"
        "Creates a new schema variable or initialises `btype`, raising a BTypeError if `btype` is already initialised."
    },
    {"seq",                 (PyCFunction) PyTM_seq, METH_FASTCALL | METH_KEYWORDS,
        "seq(contained, *, btype=Missing)\n\n"
        "Answers the type of the sequence `btype`."
    },
    {"seqT",                (PyCFunction) PyTM_seqT, METH_FASTCALL,
        "seqT(btype)\n\n"
        "Answers the contained type of the sequence `btype`."
    },
    {"struct",              (PyCFunction) PyTM_struct, METH_FASTCALL | METH_KEYWORDS,
        "struct((f1, f2,...), (t1, t2, ...), *, btype=Missing)\n\n"
        "Answers the struct type {`f1`:`t1`, `f2`:`t2`, ...}."
    },
    {"structNames",         (PyCFunction) PyTM_structNames, METH_FASTCALL,
        "structNames(btype)\n\n"
        "Answers a tuple of str - the names of the given struct `btype`."
    },
    {"structSl",             (PyCFunction) PyTM_structSl, METH_FASTCALL,
        "structSl(btype)\n\n"
        "Answers a tuple of sym - the names of the given struct `btype`."
    },
    {"structTl",             (PyCFunction) PyTM_structTl, METH_FASTCALL,
        "structTl(btype)\n\n"
        "Answers a tuple of btype - the types of the given struct `btype`."
    },
    {"tuple",               (PyCFunction) PyTM_tuple, METH_FASTCALL | METH_KEYWORDS,
        "tuple(t1, t2, ..., *, btype=Missing)\n\n"
        "Answers the tuple btype for (`t1`, `t2`, ...)."
    },
    {"tupleTl",             (PyCFunction) PyTM_tupleTl, METH_FASTCALL,
        "tupleTl(btype)\n\n"
        "Answers a (Python) tuple of the btypes for the given tuple `btype`."
    },
    {"union",               (PyCFunction) PyTM_union, METH_FASTCALL | METH_KEYWORDS,
         "union(t1, t2, ..., *, btype=Missing)\n\n"
         "Answers the union btype for `t1` + `t2` + ..."
    },
    {"union_tlid_for",      (PyCFunction) PyTM_union_tlid_for, METH_FASTCALL | METH_KEYWORDS,
        "union_tlid_for(t1, t2, ...)\n\n"
        "Answers the union typelistid for `t1` + `t2` + ..."
    },
    {"union_for",           (PyCFunction) PyTM_union_for, METH_FASTCALL | METH_KEYWORDS,
         "union_for(tlid, **, [btype])\n\n"
         "Answers the union btype for the given typelistid `tlid`."
    },
    {"union_get_for",       (PyCFunction) PyTM_union_get_for_tlid, METH_FASTCALL,
        "union_get_for(tlid)\n\n"
        "Answers the union type for `tlid` or None."
    },
    {"unionTl",             (PyCFunction) PyTM_unionTl, METH_FASTCALL,
        "unionTl(btype)\n\n"
        "Answers a tuple of the types in the union `btype`."
    },
    {0}
};

pvt PyTypeObject PyTMCls = {
    PyVarObject_HEAD_INIT(0, 0)
    .tp_name = "jones.TM",
    .tp_doc = PyDoc_STR("OPEN: write some docs"),
    .tp_basicsize = sizeof(PyTM),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_methods = PyTM_methods,
};



#endif  // SRC_JONES_PYTM_C