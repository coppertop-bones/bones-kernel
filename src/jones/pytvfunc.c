// ---------------------------------------------------------------------------------------------------------------------
// Copyright 2025 David Briant, https://github.com/coppertop-bones. Licensed under the Apache License, Version 2.0
//
// PYFNS - PYTHON NULLARY, UNARY, BINARY, TERNARY FUNCTION CLASSES
// ---------------------------------------------------------------------------------------------------------------------

#ifndef SRC_JONES_PYTVFUNC_C
#define SRC_JONES_PYTVFUNC_C "jones/pytvfunc.c"

#include "jones.h"
#include "../bk/pp.c"


#define XDEFREF_ALL(decs, n) \
    do { \
        for (Py_ssize_t _i = 0; _i < (n); _i++) { \
            Py_XDECREF((decs)[_i]); \
        } \
    } while (0)

#define RETURN(decs, n, answer) \
    do { \
        XDEFREF_ALL(decs, n); \
        return answer; \
    } while (0)

#define RETURN_NEW_ERR(decs, n, errtype, msg) \
    do { \
        XDEFREF_ALL(decs, n); \
        return PyErr_Format((errtype), (msg)); \
    } while (0)

#define RETURN_PRIOR_ERR(decs, n) \
    do { \
        XDEFREF_ALL(decs, n); \
        return NULL; \
    } while (0)


pvt PyObject *threadingModule, *sysModule, *_BType_py = 0;

pvt PyObject *_typeOf_pyfn = 0;
pvt PyObject *_distancesEtAl_pyfn = 0;          // OPEN: do in C
pvt PyObject *_fitsWithin_pyfn = 0;                 // OPEN: do in C
pvt PyObject *_tvfuncErrorCallback1_pyfn = 0;
pvt PyObject *_tvfuncErrorCallback2_pyfn = 0;
pvt bool _disableReturnCheck = false;

pvt PyObject * set_typeOf_pyfn(PyObject *mod, PyObject **args, Py_ssize_t nargs) {
    if (nargs != 1) return PyErr_Format(PyExc_TypeError, "set_typeOf takes 1 argument but %zd were given", nargs);
    PyObject *fn = args[0];
    if (!PyFunction_Check(fn)) return PyErr_Format(PyExc_TypeError, "typeOfFn must be callable got a \"%s\"", Py_TYPE(fn)->tp_name);
    if (_typeOf_pyfn) return PyErr_Format(PyExc_TypeError, "typeOfFn already set");
    _typeOf_pyfn = Py_NewRef(fn);
    return Py_None;
}

pvt PyObject *set_distancesEtAl_pyfn(PyObject *mod, PyObject **args, Py_ssize_t nargs) {
    if (nargs != 1) return PyErr_Format(PyExc_TypeError, "set_typeOf takes 1 argument but %zd were given", nargs);
    PyObject *fn = args[0];
    if (!(PyCallable_Check(fn) || PyFunction_Check(fn))) return PyErr_Format(PyExc_TypeError, "distancesEtAl must be callable");
    if (_distancesEtAl_pyfn) return PyErr_Format(PyExc_TypeError, "distancesEtAl already set");
    _distancesEtAl_pyfn = Py_NewRef(fn);
    return Py_None;
}

pvt PyObject *set_fitsWithin_pyfn(PyObject *mod, PyObject **args, Py_ssize_t nargs) {
    if (nargs != 1) return PyErr_Format(PyExc_TypeError, "set_typeOf takes 1 argument but %zd were given", nargs);
    PyObject *fn = args[0];
    if (!(PyCallable_Check(fn) || PyFunction_Check(fn))) return PyErr_Format(PyExc_TypeError, "fitsWithin must be callable");
    if (_fitsWithin_pyfn) return PyErr_Format(PyExc_TypeError, "fitsWithin already set");
    _fitsWithin_pyfn = Py_NewRef(fn);
    return Py_None;
}

pvt PyObject *set_tvfuncErrorCallback1_pyfn(PyObject *mod, PyObject **args, Py_ssize_t nargs) {
    if (nargs != 1) return PyErr_Format(PyExc_TypeError, "set_typeOf takes 1 argument but %zd were given", nargs);
    PyObject *fn = args[0];
    if (!(PyCallable_Check(fn) || PyFunction_Check(fn))) return PyErr_Format(PyExc_TypeError, "tvfuncErrorCallback1 must be callable");
    if (_tvfuncErrorCallback1_pyfn) return PyErr_Format(PyExc_TypeError, "tvfuncErrorCallback1 already set");
    _tvfuncErrorCallback1_pyfn = Py_NewRef(fn);
    return Py_None;
}

pvt PyObject *set_tvfuncErrorCallback2_pyfn(PyObject *mod, PyObject **args, Py_ssize_t nargs) {
    if (nargs != 1) return PyErr_Format(PyExc_TypeError, "set_typeOf takes 1 argument but %zd were given", nargs);
    PyObject *fn = args[0];
    if (!(PyCallable_Check(fn) || PyFunction_Check(fn))) return PyErr_Format(PyExc_TypeError, "tvfuncErrorCallback2 must be callable");
    if (_tvfuncErrorCallback2_pyfn) return PyErr_Format(PyExc_TypeError, "tvfuncErrorCallback2 already set");
    _tvfuncErrorCallback2_pyfn = Py_NewRef(fn);
    return Py_None;
}

pvt PyObject *set_BType_py(PyObject *mod, PyObject **args, Py_ssize_t nargs) {
    if (nargs != 1) return PyErr_Format(PyExc_TypeError, "set_typeOf takes 1 argument but %zd were given", nargs);
    PyObject *btype = args[0];
    // if (!PyCallable_Check(btype)) return PyErr_Format(PyExc_TypeError, "tvfuncErrorCallback2 must be a callable");
    if (btype) return PyErr_Format(PyExc_TypeError, "tvfuncErrorCallback2 already set");
    _BType_py = Py_NewRef(btype);
    return Py_None;
}

pvt PyObject *setDisableReturnCheck(PyObject *b) {
    if (!PyBool_Check(b)) return PyErr_Format(PyExc_TypeError, "b must be a bool");
    _disableReturnCheck = b == Py_True;
    return Py_None;
}

pvt PyObject *disableTracing() {
    PyObject *sysTraceFn, *pycharmDebugger_, *_args;
    sysTraceFn =  PyObject_CallMethod(sysModule, "gettrace", NULL);
    // PP(info, "%s@%i", FN_NAME, __LINE__);
    if (PyObject_HasAttrString(sysTraceFn, "_args")) {
        _args = PyObject_GetAttrString(sysTraceFn, "_args");
        pycharmDebugger_ = PyTuple_GET_ITEM(_args, 0);                   // borrowed
        PyObject *result = PyObject_CallMethod(pycharmDebugger_, "disable_tracing", NULL);
        if (result != Py_None) Py_DECREF(result);
        Py_DECREF(_args);
        return sysTraceFn;  // return the sysTraceFn so it can be restored later
    }
    else {
        Py_DECREF(sysTraceFn);
        return NULL;  // no  Pycharm debugger
    }
}

pvt void enableTracingAndDec(PyObject *sysTraceFn) {
    PyObject *pycharmDebugger_, *_args;
    if (sysTraceFn) {
        // PP(info, "%s@%i", FN_NAME, __LINE__);
        _args = PyObject_GetAttrString(sysTraceFn, "_args");
        // PP(info, "%s@%i", FN_NAME, __LINE__);
        pycharmDebugger_ = PyTuple_GET_ITEM(_args, 0);                   // borrowed
        // PP(info, "%s@%i", FN_NAME, __LINE__);
        PyObject *ret = PyObject_CallMethod(pycharmDebugger_, "enable_tracing", NULL);
        // PP(info, "%s@%i", FN_NAME, __LINE__);
        if (ret != Py_None) {
            // PP(info, "%s@%i", FN_NAME, __LINE__);
            PyObject *repr_obj = PyObject_Repr(ret);
            if (repr_obj) {
                const char *repr_str = PyUnicode_AsUTF8(repr_obj);
                // PP(info, "%s@%i - %s", FN_NAME, __LINE__, repr_str);
                Py_DECREF(repr_str);
            }
            // PP(info, "%s@%i", FN_NAME, __LINE__);
            Py_XDECREF(ret);
        }
        Py_DECREF(_args);  Py_DECREF(sysTraceFn);
    }
}


// ---------------------------------------------------------------------------------------------------------------------
// PySelectionResult
// OPEN: rename as JSelectionResult
// ---------------------------------------------------------------------------------------------------------------------

typedef struct {
    PyObject_HEAD
    PyObject *tvfunc;
    PyObject *tByT;
} PySelectionResult;

// ---------------------------------------------------------------------------------------------------------------------
// PySelectionResult lifecycle

pvt void PySelectionResult_dealloc(PySelectionResult *self) {
    Py_XDECREF(self->tvfunc);
    Py_XDECREF(self->tByT);
}

pvt PyObject * PySelectionResult_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    if (PyTuple_GET_SIZE(args) != 2) return PyErr_Format(PyExc_Exception, "Must be created as SelectionResult(tvfunc, tByT)");
    return type->tp_alloc(type, 0);
}

pvt int PySelectionResult_init(PySelectionResult *self, PyObject *args, PyObject *kwds) {
    PyObject *tvfunc, *tByT;
    if (!PyArg_ParseTuple(args, "OO:", &tvfunc, &tByT)) return -1;
    if (!PyCallable_Check(tvfunc)) {PyErr_Format(PyExc_TypeError, "tvfunc is not a callable"); return -1;}
    self->tvfunc = Py_NewRef(tvfunc);
    self->tByT = Py_NewRef(tByT);
    return 0;
}

// ---------------------------------------------------------------------------------------------------------------------
// PySelectionResult members, get/setter, methods

pvt PyMemberDef PySelectionResult_members[] = {
        {"tvfunc", Py_T_OBJECT_EX, offsetof(PySelectionResult, tvfunc), Py_READWRITE, "tvfunc callable"},
        {"tByT", Py_T_OBJECT_EX, offsetof(PySelectionResult, tByT), Py_READWRITE, "dict of BType by BSchemaVar"},
        {0}
};

pvt PyTypeObject PySelectionResultCls = {
        PyVarObject_HEAD_INIT(0, 0)
        .tp_name = "jones.SelectionResult",
        .tp_basicsize = sizeof(PySelectionResult),
        .tp_itemsize = 0,
        .tp_doc = PyDoc_STR("SelectionResult is a struct with a tvfunc and tByT."),
        .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
        .tp_new = PySelectionResult_new,
        .tp_init = (initproc) PySelectionResult_init,
        .tp_dealloc = (destructor) PySelectionResult_dealloc,
        .tp_members = PySelectionResult_members,
        // .tp_methods = PySelectionResult_methods,
        // .tp_getset = PySelectionResult_getsetters,
        // .tp_call = (ternaryfunc) PyVectorcall_Call,
        // .tp_vectorcall_offset = offsetof(PySelectionResult, vectorcall),
};



// ---------------------------------------------------------------------------------------------------------------------
// PyJFunc
// ---------------------------------------------------------------------------------------------------------------------

typedef struct {
    PyObject_HEAD
    PyObject *style;
    PyObject *name;
    PyObject *_t;
    PyObject *modname;
    PyObject *_v;
    PyObject *argNames;
    PyObject *sig;
    PyObject *tArgs;
    PyObject *tRet;
    PyObject *pass_tByT;                            // bool
    PyObject *dispatchEvenIfAllTypes;
    PyObject *typeHelper_pyfn;
    PyObject *__doc__;
    vectorcallfunc vectorcall;
} PyJFunc;

// ---------------------------------------------------------------------------------------------------------------------
// PyJFunc call(...)

//    def __call__(self, *args, schemaVars=Missing):
//         try:
//             if self.pass_tByT:
//                 if schemaVars is Missing:
//                     match, fallback, schemaVars, argDistances = _distancesEtAl([_typeOf(arg) for arg in args], self.sig)
//                 if self.typeHelper:
//                     schemaVars = self.typeHelper(*args, tByT=schemaVars)
//                 ret = self._v(*args, tByT=schemaVars)
//             else:
//                 ret = self._v(*args)
//         except TypeError as ex:
//             _tvfuncErrorCallback1(ex, self)
//
//         if DISABLE_RETURN_CHECK:
//             return ret
//         else:
//             tRet = self.tRet
//             if tRet == py or isinstance(ret, jones.SelectionResult):
//                 return ret
//             else:
//                 # OPEN: BTTuples are products whereas pytuples are exponentials therefore we can reliably type check
//                 # an answered sequence if the return type is BTTuple (and possibly BTStruct) - also BTTuple can be
//                 # coerced by default to a dseq
//                 if fitsWithin(_typeOf(ret), tRet):
//                     return ret
//                 else:
//                     _tvfuncErrorCallback2(self, ret)


//     if (!PyTuple_Check(tup) || PyTuple_GET_SIZE(tup) != 3) RETURN_NEW_ERR(decs, ndecs, PyExc_TypeError, "Expected a tuple of (tvfunc, schemaVars, hasValue) from function selection");
//
//     // unpack tup
//     tvfunc = PyTuple_GET_ITEM(tup, 0);             // No need to add to decs for later DECREF as it's a borrowed reference
//     schemaVars = PyTuple_GET_ITEM(tup, 1);
//     hasValue = PyTuple_GET_ITEM(tup, 2);
//
//     dispatchEvenIfAllTypes = PyObject_GetAttrString(tvfunc, "dispatchEvenIfAllTypes");
//
//     if (hasValue == Py_True || dispatchEvenIfAllTypes == Py_True) {
//         // call the function
//         PyObject *fncargs[nargs + 1];                            // variable-length array
//         for (i =0; i < nargs; i++) fncargs[i] = args[i];     // positional args
//         fncargs[nargs] = schemaVars;                          // keyword arg schemaVars
//         fnkwnames = decs[ndecs++] = PyTuple_Pack(1, PyUnicode_FromString("schemaVars"));
//         if (!fnkwnames) RETURN_PRIOR_ERR(decs, ndecs);
//
//         // PyObject *repr_obj = decs[ndecs++] = PyObject_Repr(tvfunc);
//         // if (repr_obj) {
//         //     const char *repr_str = PyUnicode_AsUTF8(repr_obj);
//         //     PP(info, "%s@%i - nargs: %i - %s", FN_NAME, __LINE__, nargs, repr_str);
//         // }
//         answer = PyObject_Vectorcall(tvfunc, fncargs, nargs, fnkwnames);        // nargs is the number of positional arguments




pvt PyObject * PyJFunc_vectorcall(PyJFunc *self, PyObject *const *args, size_t nargsf, PyObject *kwnames) {
    PyObject *sysTraceFn, *ret, *tRet, *schemaVars, *schemaVarsOld, *fnkwnames;  Py_ssize_t nargs;  int i;
    PyObject *decs[8] = {0,0,0,0,0,0,0,0};  int ndecs = 0;
    nargs = PyVectorcall_NARGS(nargsf);

    // try calling the underlying callable
    // PP(info, "%s@%i - %s(%zd args)", FN_NAME, __LINE__, PyUnicode_AsUTF8(self->name), nargs);
    if (self->pass_tByT == Py_True) {
        // PP(info, "%s@%i - PyJFunc_vectorcall - tByT", FN_NAME, __LINE__);
        if (kwnames && PyTuple_GET_SIZE(kwnames) == 1 && strcmp(PyUnicode_AsUTF8(PyTuple_GET_ITEM(kwnames, 0)), "schemaVars") == 0) {
                schemaVars = args[nargs];
                if (!PyDict_Check(schemaVars)) {
                    RETURN_NEW_ERR(decs, ndecs, PyExc_RuntimeError, "tByT is not a dict");
                }
        } else {
            // match, fallback, schemaVars, argDistances = _distancesEtAl([_typeOf(arg) for arg in args], self.sig)
            RETURN_NEW_ERR(decs, ndecs, PyExc_RuntimeError, "get _distancesEtAl - NotYetImplemented");
        }
        fnkwnames = decs[ndecs++] = PyTuple_Pack(1, PyUnicode_FromString("tByT"));
        if (!fnkwnames) RETURN_NEW_ERR(decs, ndecs, PyExc_RuntimeError, "can't create tuple of kwargnames");
        if (self->typeHelper_pyfn) {
            // PP(info, "%s@%i - %s(%zd args) with typeHelper", FN_NAME, __LINE__, PyUnicode_AsUTF8(self->name), nargs);
            schemaVars = decs[ndecs++] = PyObject_Vectorcall(self->typeHelper_pyfn, args, nargsf, fnkwnames);
            PyObject *fncargs[nargs + 1];                            // variable-length array
            for (i =0; i < nargs; i++) fncargs[i] = args[i];     // positional args
            fncargs[nargs] = schemaVars;                          // keyword arg schemaVars
            ret = PyObject_Vectorcall(self->_v, fncargs, nargs, fnkwnames);        // nargs is the number of positional arguments
        } else {
            // PP(info, "%s@%i - %s(%zd args) with tByT", FN_NAME, __LINE__, PyUnicode_AsUTF8(self->name), nargs);
            ret = PyObject_Vectorcall(self->_v, args, nargsf, fnkwnames);
        }
    } else {
        ret = PyObject_Vectorcall(self->_v, args, nargsf, NULL);
    }

    // except:
    if (!ret) {
        if (!_tvfuncErrorCallback1_pyfn) {
            RETURN_NEW_ERR(decs, ndecs, PyExc_RuntimeError, "No error callback defined for this function");
        } else {
            PyObject *exc_type = NULL, *exc_value = NULL, *exc_tb = NULL;
            PyErr_Fetch(&exc_type, &exc_value, &exc_tb);
            PyErr_NormalizeException(&exc_type, &exc_value, &exc_tb);
            // PP(info, "%s@%i - %s(%zd args) - _tvfuncErrorCallback1_pyfn", FN_NAME, __LINE__, PyUnicode_AsUTF8(self->name), nargs);
            PyObject *err_result = PyObject_CallFunctionObjArgs(_tvfuncErrorCallback1_pyfn, exc_value, self, NULL);
            // PP(info, "%s@%i", FN_NAME, __LINE__);
            PyErr_Restore(exc_type, exc_value, exc_tb);                                 // steals reference so do not DECREF
            // PP(info, "%s@%i", FN_NAME, __LINE__);
            return NULL;
        }
    }

    // check the return type
    if (_disableReturnCheck) {
        RETURN (decs, ndecs, ret);
    } else {
        tRet = self->tRet;
        RETURN (decs, ndecs, ret);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
// PyJFunc lifecycle, members, get/setter, methods

pvt void PyJFunc_dealloc(PyJFunc *self) {
    Py_XDECREF(self->style);
    Py_XDECREF(self->name);
    Py_XDECREF(self->_t);
    Py_XDECREF(self->modname);
    Py_XDECREF(self->_v);
    Py_XDECREF(self->argNames);
    Py_XDECREF(self->sig);
    Py_XDECREF(self->tArgs);
    Py_XDECREF(self->tRet);
    Py_XDECREF(self->pass_tByT);
    Py_XDECREF(self->dispatchEvenIfAllTypes);
    Py_XDECREF(self->typeHelper_pyfn);
    Py_XDECREF(self->__doc__);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

pvt PyObject * PyJFunc_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    PyJFunc *inst;
    // if (PyTuple_GET_SIZE(args) != 9) return PyErr_Format(PyExc_Exception, "Must be created as Fn(name, bmod, d, TBCSentinel)");
    inst = (PyJFunc *) type->tp_alloc(type, 0);
    inst->style = Py_None;
    inst->name = Py_None;
    inst->_t = Py_None;
    inst->modname = Py_None;
    inst->_v = Py_None;
    inst->argNames = Py_None;
    inst->style = Py_None;
    inst->sig = Py_None;
    inst->tArgs = Py_None;
    inst->tRet = Py_None;
    inst->pass_tByT = Py_None;
    inst->dispatchEvenIfAllTypes = Py_None;
    inst->typeHelper_pyfn = 0;
    inst->__doc__ = Py_None;
    inst->vectorcall = (vectorcallfunc) PyJFunc_vectorcall;
    return (PyObject *) inst;
}

pvt int PyJFunc_init(PyJFunc *self, PyObject *args, PyObject *kwds) {
    return 0;
}

pvt PyObject * PyJFunc_get_doc(PyJFunc *self, void *closure) {
    return 0;
    // return PyUnicode_FromString("Fn...");
}

pvt PyObject * PyJFunc_get_d(PyJFunc *self, void *closure) {
    return  0;
    // return Py_NewRef(self->d);
}

pvt int PyJFunc_set_d(PyJFunc *self, PyObject *d, void* closure) {
    // if (!PyCallable_Check(d)) {
    //     PyErr_Format(PyExc_TypeError, "d is not a callable");
    //     return -1;
    // }
    // Py_XDECREF(self->d);
    // self->d = Py_NewRef(d);
    return 0;
}

pvt PyGetSetDef PyJFunc_getsetters[] = {
        // {"d", (getter) PyJFunc_get_d, (setter) PyJFunc_set_d, "dispatcher", 0},
        // {"__doc__", (getter) PyJFunc_get_doc, 0, 0, 0},
        {0}
};

// https://docs.python.org/3/c-api/structures.html#c.Py_READONLY
pvt PyMemberDef PyJFunc_members[] = {
        {"style",                                Py_T_OBJECT_EX, offsetof(PyJFunc, style), Py_READWRITE, "style"},
        {"name",                               Py_T_OBJECT_EX, offsetof(PyJFunc, name), Py_READWRITE, "name"},
        {"_t",                                     Py_T_OBJECT_EX, offsetof(PyJFunc, _t), Py_READWRITE, "_t"},
        {"modname",                        Py_T_OBJECT_EX, offsetof(PyJFunc, modname), Py_READWRITE, "modname"},
        {"_v",                                     Py_T_OBJECT_EX, offsetof(PyJFunc, _v), Py_READWRITE, "_v"},
        {"argNames",                        Py_T_OBJECT_EX, offsetof(PyJFunc, argNames), Py_READWRITE, "argNames"},
        {"sig",                                    Py_T_OBJECT_EX, offsetof(PyJFunc, sig), Py_READWRITE, "sig"},
        {"tArgs",                                Py_T_OBJECT_EX, offsetof(PyJFunc, tArgs), Py_READWRITE, "tArgs"},
        {"tRet",                                  Py_T_OBJECT_EX, offsetof(PyJFunc, tRet), Py_READWRITE, "tRet"},
        {"pass_tByT",                       Py_T_OBJECT_EX, offsetof(PyJFunc, pass_tByT), Py_READWRITE, "pass_tByT"},
        {"dispatchEvenIfAllTypes", Py_T_OBJECT_EX, offsetof(PyJFunc, dispatchEvenIfAllTypes), Py_READWRITE, "dispatchEvenIfAllTypes"},
        {"typeHelper",                      Py_T_OBJECT_EX, offsetof(PyJFunc, typeHelper_pyfn), Py_READWRITE, "typeHelper"},
        {"__doc__",                          Py_T_OBJECT_EX, offsetof(PyJFunc, __doc__), Py_READWRITE, "__doc__"},
        {0}
};

pvt PyTypeObject PyJFuncCls = {
        PyVarObject_HEAD_INIT(0, 0)
        .tp_name = "jones.JFunc",
        .tp_basicsize = sizeof(PyJFunc),
        .tp_itemsize = 0,
        .tp_doc = PyDoc_STR("JFunc"),
        .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
        .tp_new = PyJFunc_new,
        .tp_init = (initproc) PyJFunc_init,
        .tp_dealloc = (destructor) PyJFunc_dealloc,
        .tp_members = PyJFunc_members,
        // .tp_methods = PyJFunc_methods,
        .tp_getset = PyJFunc_getsetters,
        .tp_call = (ternaryfunc) PyVectorcall_Call,
        .tp_vectorcall_offset = offsetof(PyJFunc, vectorcall),
};



// ---------------------------------------------------------------------------------------------------------------------
// PyJOverload
// ---------------------------------------------------------------------------------------------------------------------

typedef struct {
    PyObject_HEAD
    PyObject *_tvfuncBySig;                 // dict of JFunc by sig
    PyObject *name;                              // str
    PyObject *numargs;
    PyObject *_selectFunctionCallback; 
    vectorcallfunc vectorcall;
} PyJOverload;

// ---------------------------------------------------------------------------------------------------------------------
// PyJOverload call(...)

//     def __call__(self, *args):
//         if DISABLE_ARG_CHECK_FOR_SOLE_FN and len(self._tvfuncBySig) == 1:
//             tvfunc = firstValue(self._tvfuncBySig)
//             return tvfunc(*args)
//         with disable_tracing():
//             tvfunc, schemaVars, hasValue = self.selectFunction(*args)        // call back into Python to select the function
//         if hasValue or tvfunc.dispatchEvenIfAllTypes:
//             return tvfunc(*args, schemaVars=schemaVars)
//         else:
//             return SelectionResult(tvfunc, schemaVars)

pvt PyObject * PyJOverload_vectorcall(PyJOverload *self, PyObject *const *args, size_t nargsf, PyObject *kwnames) {
    PyObject *sysTraceFn, *tup, *tvfunc, *schemaVars, * hasValue, *dispatchEvenIfAllTypes, *fnkwnames, *answer;  Py_ssize_t nargs;  int i;
    PyObject *decs[8] = {0,0,0,0,0,0,0,0};  int ndecs = 0;
    nargs = PyVectorcall_NARGS(nargsf);
    if (kwnames) return PyErr_Format(PyExc_RuntimeError, "PyJOverload_vectorcall: kwargs are not allowed");

    // select the function to call
    // PP(info, "%s@%i - %s(%zd args)", FN_NAME, __LINE__, PyUnicode_AsUTF8(self->name), nargs);
    sysTraceFn = disableTracing();
    tup = decs[ndecs++] = PyObject_Vectorcall(self->_selectFunctionCallback, args, nargsf, NULL);
    // PP(info, "%s@%i - %s(%zd args)", FN_NAME, __LINE__, PyUnicode_AsUTF8(self->name), nargs);
    enableTracingAndDec(sysTraceFn);
    // PP(info, "%s@%i - %s(%zd args)", FN_NAME, __LINE__, PyUnicode_AsUTF8(self->name), nargs);

    // check the result
    if (!tup) RETURN_PRIOR_ERR(decs, ndecs);
    if (!PyTuple_Check(tup) || PyTuple_GET_SIZE(tup) != 3) RETURN_NEW_ERR(decs, ndecs, PyExc_TypeError, "Expected a tuple of (tvfunc, schemaVars, hasValue) from function selection");

    // unpack tup
    tvfunc = PyTuple_GET_ITEM(tup, 0);             // No need to add to decs for later DECREF as it's a borrowed reference
    schemaVars = PyTuple_GET_ITEM(tup, 1);
    hasValue = PyTuple_GET_ITEM(tup, 2);
    
    dispatchEvenIfAllTypes = PyObject_GetAttrString(tvfunc, "dispatchEvenIfAllTypes");
    
    if (hasValue == Py_True || dispatchEvenIfAllTypes == Py_True) {
        // call the function
        PyObject *fncargs[nargs + 1];                            // variable-length array
        for (i =0; i < nargs; i++) fncargs[i] = args[i];     // positional args
        fncargs[nargs] = schemaVars;                          // keyword arg schemaVars
        fnkwnames = decs[ndecs++] = PyTuple_Pack(1, PyUnicode_FromString("schemaVars"));
        if (!fnkwnames) RETURN_PRIOR_ERR(decs, ndecs);

        // PyObject *repr_obj = decs[ndecs++] = PyObject_Repr(tvfunc);
        // if (repr_obj) {
        //     const char *repr_str = PyUnicode_AsUTF8(repr_obj);
        //     PP(info, "%s@%i - nargs: %i - %s", FN_NAME, __LINE__, nargs, repr_str);
        // }
        answer = PyObject_Vectorcall(tvfunc, fncargs, nargs, fnkwnames);        // nargs is the number of positional arguments
        // PP(info, "%s@%i - ndecs = %i", FN_NAME, __LINE__, ndecs);
        // for (i = 0; i < ndecs; i++) {
        //     if (decs[i]) {
        //         PP(info, "%s@%i - decs[%i] - %p ", FN_NAME, __LINE__, i, decs[i]);
        //         PP(info, "    %s", PyUnicode_AsUTF8(PyObject_Repr(decs[i])));
        //     }
        // }
        XDEFREF_ALL(decs, ndecs);
        // PP(info, "%s@%i", FN_NAME, __LINE__);
        return answer;
    }
    else {
        // return SelectionResult(tvfunc, schemaVars)
        // PP(info, "%s@%i", FN_NAME, __LINE__);
        answer = PyObject_CallFunction((PyObject *) &PySelectionResultCls, "OO", tvfunc, schemaVars);
        if (!answer) RETURN_PRIOR_ERR(decs, ndecs);
        XDEFREF_ALL(decs, ndecs);
        return answer;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
// PyJOverload lifecycle, members, get/setter, methods

pvt void PyJOverload_dealloc(PyJOverload *self) {
    Py_XDECREF(self->_tvfuncBySig);
    Py_XDECREF(self->name);
    Py_XDECREF(self->numargs);
    Py_XDECREF(self->_selectFunctionCallback);
}

pvt PyObject * PyJOverload_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    PyJOverload *inst = (PyJOverload *) type->tp_alloc(type, 0);
    inst->vectorcall = (vectorcallfunc) PyJOverload_vectorcall;
    inst->_tvfuncBySig = Py_NewRef(Py_None);
    inst->name = Py_NewRef(Py_None);
    inst->numargs = Py_NewRef(Py_None);
    inst->_selectFunctionCallback = Py_NewRef(Py_None);
    return (PyObject *) inst;
}

pvt int PyJOverload_init(struct Fn *self, PyObject *args, PyObject *kwds) {
    return 0;
}

pvt PyMemberDef PyJOverload_members[] = {
        {"_tvfuncBySig", Py_T_OBJECT_EX, offsetof(PyJOverload, _tvfuncBySig), Py_READWRITE, "_tvfuncBySig description"},
        {"name", Py_T_OBJECT_EX, offsetof(PyJOverload, name), Py_READWRITE, "name description"},
        {"numargs", Py_T_OBJECT_EX, offsetof(PyJOverload, numargs), Py_READWRITE, "numargs description"},
        {"_selectFunctionCallback", Py_T_OBJECT_EX, offsetof(PyJOverload, _selectFunctionCallback), Py_READWRITE, "_selectFunctionCallback description"},
        {0}
};

pvt PyTypeObject PyJOverloadCls = {
        PyVarObject_HEAD_INIT(0, 0)
        .tp_name = "jones.JOverload",
        .tp_basicsize = sizeof(PyJOverload),
        .tp_itemsize = 0,
        .tp_doc = PyDoc_STR("JOverload holds a map of JFunc by sig"),
        .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
        .tp_new = PyJOverload_new,
        .tp_init = (initproc) PyJOverload_init,
        .tp_dealloc = (destructor) PyJOverload_dealloc,
        .tp_members = PyJOverload_members,
        // .tp_methods = PyJOverload_methods,
        // .tp_getset = PyJOverload_getsetters,
        .tp_call = (ternaryfunc) PyVectorcall_Call,
        .tp_vectorcall_offset = offsetof(PyJOverload, vectorcall),
};



// ---------------------------------------------------------------------------------------------------------------------
// PyJFamily
// ---------------------------------------------------------------------------------------------------------------------

typedef struct {
    PyObject_HEAD
    PyObject *_overloadByNumArgs;   // list of Overload
    PyObject *name;                              // str
    PyObject *style;
    PyObject *_t;
    PyObject *_doc;
    vectorcallfunc vectorcall;
} PyJFamily;

// ---------------------------------------------------------------------------------------------------------------------
// PyJFamily call(...)

//     def __call__(self, *args):
//         if (numArgs := len(args)) > len(self._overloadByNumArgs) - 1:
//             raise TypeError(f"Too many args passed to  {self.name} - max {len(self._overloadByNumArgs) - 1}, passed {numArgs}")
//         return self._overloadByNumArgs[numArgs](*args)

pvt PyObject * PyJFamily_vectorcall(PyJFamily *self, PyObject *const *args, size_t nargsf, PyObject *kwnames) {
    Py_ssize_t nargs = PyVectorcall_NARGS(nargsf);
    if (kwnames) {
        PyErr_SetString(PyExc_RuntimeError, "PyJFamily_vectorcall: kwargs are not allowed");
        return NULL;
    }
    if (nargs > PyList_GET_SIZE(self->_overloadByNumArgs) - 1) {
        PyErr_Format(PyExc_TypeError, "Too many args passed to %s - max %zd, passed %zd", PyUnicode_AsUTF8(self->name), PyList_GET_SIZE(self->_overloadByNumArgs) - 1, nargs);
        return NULL;
    }
    PyObject *overload = PyList_GET_ITEM(self->_overloadByNumArgs, nargs);
    if (Py_IsNone(overload)) {
        PyErr_Format(PyExc_TypeError, "No overload for %zd args", nargs);
        return NULL;
    } else {
        return PyObject_Vectorcall(overload, args, nargsf, NULL);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
// PyJFamily lifecycle, members, get/setter, methods

pvt void PyJFamily_dealloc(PyJFamily *self) {
    Py_XDECREF(self->_overloadByNumArgs);
    Py_XDECREF(self->name);
    Py_XDECREF(self->style);
    Py_XDECREF(self->_t);
    Py_XDECREF(self->_doc);
}

pvt PyObject * PyJFamily_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    PyJFamily *inst = (PyJFamily *) type->tp_alloc(type, 0);
    inst->vectorcall = (vectorcallfunc) PyJFamily_vectorcall;
    inst->_overloadByNumArgs = Py_NewRef(Py_None);
    inst->name = Py_NewRef(Py_None);
    inst->style = Py_NewRef(Py_None);
    inst->_t = Py_NewRef(Py_None);
    inst->_doc = Py_NewRef(Py_None);
    return (PyObject *) inst;
}

pvt int PyJFamily_init(struct Fn *self, PyObject *args, PyObject *kwds) {
    // PyObject *name, *bmod, *d, *TBCSentinel;
    // if (!PyArg_ParseTuple(args, "UUOO:", &name, &bmod, &d, &TBCSentinel)) return -1;
    // // OPEN: check type of other args
    // self->name = Py_NewRef(name);
    // self->bmod = Py_NewRef(bmod);
    // if (!PyCallable_Check(d)) {PyErr_Format(PyExc_TypeError, "d is not a callable"); return -1;}
    // self->d = Py_NewRef(d);
    // self->TBCSentinel = Py_NewRef(TBCSentinel);
    return 0;
}

pvt PyMemberDef PyJFamily_members[] = {
        {"_overloadByNumArgs", Py_T_OBJECT_EX, offsetof(PyJFamily, _overloadByNumArgs), Py_READWRITE, "list of JOverload"},
        {"name", Py_T_OBJECT_EX, offsetof(PyJFamily, name), Py_READWRITE, "name"},
        {"style", Py_T_OBJECT_EX, offsetof(PyJFamily, style), Py_READWRITE, "style"},
        {"_t", Py_T_OBJECT_EX, offsetof(PyJFamily, _t), Py_READWRITE, "_t"},
        {"_doc", Py_T_OBJECT_EX, offsetof(PyJFamily, _doc), Py_READWRITE, "_doc"},
        {0}
};

pvt PyTypeObject PyJFamilyCls = {
        PyVarObject_HEAD_INIT(0, 0)
        .tp_name = "jones.JFamily",
        .tp_basicsize = sizeof(PyJFamily),
        .tp_itemsize = 0,
        .tp_doc = PyDoc_STR("JFamily holds a family of JOverload functions indexed by number of args"),
        .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
        .tp_new = PyJFamily_new,
        // .tp_init = (initproc) PyJFamily_init,
        .tp_dealloc = (destructor) PyJFamily_dealloc,
        .tp_members = PyJFamily_members,
        // .tp_methods = PyJFamily_methods,
        // .tp_getset = PyJFamily_getsetters,
        .tp_call = (ternaryfunc) PyVectorcall_Call,
        .tp_vectorcall_offset = offsetof(PyJFamily, vectorcall),
};




// def _distancesEtAl(callerSig, fnSig):
// # OPEN: implement this in C
// # if len(callerSig) == 2 and len(fnSig) == 2 and callerSig[0].id == 108 and callerSig[1].id == 106 and fnSig[0].id == 108 and fnSig[1].id == 106:
// #     pass
//     match = True
//     fallback = False
//     argDistances = []
//     schemaVars = {}
//     for tArg, tFnArg in zip(callerSig, fnSig):
//         if tFnArg == py:
//             fallback = True
//             argDistances.append(0.5)
//         else:
//             fits = fitsWithin(tArg, tFnArg)
//             if not fits:
//                 match = False
//                 break
//             try:
//                 schemaVars, argDistance = updateSchemaVarsWith(schemaVars, 0, fits)
//             except SchemaError:
//                 match = False
//                 break
//             argDistances.append(argDistance)
//     return match, fallback, schemaVars, argDistances





#endif  // SRC_JONES_PYTVFUNC_C