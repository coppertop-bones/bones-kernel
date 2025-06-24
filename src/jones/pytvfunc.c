// ---------------------------------------------------------------------------------------------------------------------
// Copyright 2025 David Briant, https://github.com/coppertop-bones. Licensed under the Apache License, Version 2.0
//
// PYFNS - PYTHON NULLARY, UNARY, BINARY, TERNARY FUNCTION CLASSES
// ---------------------------------------------------------------------------------------------------------------------

#ifndef SRC_JONES_PYTVFUNC_C
#define SRC_JONES_PYTVFUNC_C "jones/pytvfunc.c"

#include "jones.h"




// return PyObject_CallFunctionObjArgs((PyObject *)partial->Fn.d, partial->pipe1, rhs, 0);




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
    PyObject *pass_tByT;
    PyObject *dispatchEvenIfAllTypes;
    PyObject *typeHelper;
    PyObject *__doc__;
    PyObject *(*call)(PyObject *self, PyObject *args, PyObject *kwargs); // __call__ method
} PyJFunc;


// ---------------------------------------------------------------------------------------------------------------------
// PyJFunc lifecycle
// ---------------------------------------------------------------------------------------------------------------------

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
    Py_XDECREF(self->typeHelper);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

pvt PyObject * PyJFunc_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    if (PyTuple_GET_SIZE(args) != 4) return PyErr_Format(PyExc_Exception, "Must be created as Fn(name, bmod, d, TBCSentinel)");
    return type->tp_alloc(type, 0);
}

// def __init__(self, *, name, modname, style, _v, dispatchEvenIfAllTypes, typeHelper, _t, argNames, pass_tByT)

pvt int PyJFunc_init(PyJFunc *self, PyObject *args, PyObject *kwds) {
    // PyObject *pyName, *pyBmod, *pyD, *pyTBCSentinel;
    // if (!PyArg_ParseTuple(args, "UUOO:", &pyName, &pyBmod, &pyD, &pyTBCSentinel)) return -1;
    // // OPEN: check type of other args
    // self->name = Py_NewRef(name);
    // self->bmod = Py_NewRef(bmod);
    // if (!PyCallable_Check(d)) {PyErr_Format(PyExc_TypeError, "d is not a callable"); return -1;}
    // self->d = Py_NewRef(d);
    // self->TBCSentinel = Py_NewRef(TBCSentinel);
    return 0;
}


// ---------------------------------------------------------------------------------------------------------------------
// PyJFunc members, get/setter, methods
// ---------------------------------------------------------------------------------------------------------------------

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
        {"style",               Py_T_OBJECT_EX, offsetof(PyJFunc, name), Py_READWRITE, "style"},
        {"name",            Py_T_OBJECT_EX, offsetof(PyJFunc, name), Py_READWRITE, "name"},
        {"_t",                 Py_T_OBJECT_EX, offsetof(PyJFunc, name), Py_READWRITE, "_t"},
        {"modname",    Py_T_OBJECT_EX, offsetof(PyJFunc, name), Py_READWRITE, "modname"},
        {"_v",                 Py_T_OBJECT_EX, offsetof(PyJFunc, name), Py_READWRITE, "_v"},
        {"argNames",                    Py_T_OBJECT_EX, offsetof(PyJFunc, name), Py_READWRITE, "argNames"},
        {"sig",                                 Py_T_OBJECT_EX, offsetof(PyJFunc, name), Py_READWRITE, "sig"},
        {"tArgs",                               Py_T_OBJECT_EX, offsetof(PyJFunc, name), Py_READWRITE, "tArgs"},
        {"tRet",                                Py_T_OBJECT_EX, offsetof(PyJFunc, name), Py_READWRITE, "tRet"},
        {"pass_tByT",                       Py_T_OBJECT_EX, offsetof(PyJFunc, name), Py_READWRITE, "pass_tByT"},
        {"dispatchEvenIfAllTypes", Py_T_OBJECT_EX, offsetof(PyJFunc, name), Py_READWRITE, "dispatchEvenIfAllTypes"},
        {"typeHelper",                      Py_T_OBJECT_EX, offsetof(PyJFunc, name), Py_READWRITE, "typeHelper"},
        {"__doc__",                         Py_T_OBJECT_EX, offsetof(PyJFunc, name), Py_READWRITE, "__doc__"},
        {0}
};

// pvt PyMethodDef PyJFunc_methods[] = {
//         {"__array_ufunc__", (PyCFunction) _Common__array_ufunc__, METH_VARARGS | METH_KEYWORDS, "__array_ufunc__"},
//         {0}
// };


// ---------------------------------------------------------------------------------------------------------------------
// PyJFunc call(...)

pvt PyObject * PyJFunc__call__(PyJFunc *self, PyObject *args, PyObject *kwds) {
    return PyObject_CallObject(self->_v, args);
}


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
        .tp_call = (ternaryfunc) PyJFunc__call__,
};



// ---------------------------------------------------------------------------------------------------------------------
// PyJOverload
// ---------------------------------------------------------------------------------------------------------------------

typedef struct {
    PyObject_HEAD
    PyObject *_tvfuncBySig;                 // dict of JFunc by sig
    PyObject *name;                              // str
    PyObject *numargs;
    PyObject *_selectFunctionCallback;          // bool
    vectorcallfunc vectorcall;
} PyJOverload;

// ---------------------------------------------------------------------------------------------------------------------
// PyJOverload call(...)

//     def __call__(self, *args):
//         if DISABLE_ARG_CHECK_FOR_SOLE_FN and len(self._tvfuncBySig) == 1:
//             tvfunc = firstValue(self._tvfuncBySig)
//             return tvfunc(*args)
//         db = Fred()
//         db.disable_tracing()
//         tvfunc, schemaVars, hasValue = self.selectFunction(*args)
//         db.enable_tracing()
//         if hasValue or tvfunc.dispatchEvenIfAllTypes:
//             return tvfunc(*args, schemaVars=schemaVars)
//         else:
//             return SelectionResult(tvfunc, schemaVars)


// ---------------------------------------------------------------------------------------------------------------------
// PyJOverload lifecycle

pvt void PyJOverload_dealloc(PyJOverload *self) {
    Py_XDECREF(self->_tvfuncBySig);
    Py_XDECREF(self->name);
    Py_XDECREF(self->numargs);
    Py_XDECREF(self->_selectFunctionCallback);
}

pvt PyObject * PyJOverload_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    PyJOverload *inst = (PyJOverload *) type->tp_alloc(type, 0);
    // inst->vectorcall = (vectorcallfunc) PyJOverload_vectorcall;
    inst->_tvfuncBySig = Py_NewRef(Py_None);
    inst->name = Py_NewRef(Py_None);
    inst->numargs = Py_NewRef(Py_None);
    inst->_selectFunctionCallback = Py_NewRef(Py_None);
    return (PyObject *) inst;
}

pvt int PyJOverload_init(struct Fn *self, PyObject *args, PyObject *kwds) {
    return 0;
}

// ---------------------------------------------------------------------------------------------------------------------
// PyJOverload members, get/setter, methods


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
        // .tp_call = (ternaryfunc) PyVectorcall_Call,
        // .tp_vectorcall_offset = offsetof(PyJOverload, vectorcall),
};



// ---------------------------------------------------------------------------------------------------------------------
// PyJFamily
// ---------------------------------------------------------------------------------------------------------------------

typedef struct {
    PyObject_HEAD
    PyObject *_overloadByNumArgs;   // list of Overload
    PyObject *name;                              // str
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
        PyErr_SetString(PyExc_RuntimeError, "kwargs are not allowed");
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
// PyJFamily lifecycle

pvt void PyJFamily_dealloc(PyJFamily *self) {
    Py_XDECREF(self->_overloadByNumArgs);
    Py_XDECREF(self->name);
}

pvt PyObject * PyJFamily_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    PyJFamily *inst = (PyJFamily *) type->tp_alloc(type, 0);
    inst->vectorcall = (vectorcallfunc) PyJFamily_vectorcall;
    inst->_overloadByNumArgs = Py_NewRef(Py_None);          // PyList_New(8);
    inst->name = Py_NewRef(Py_None);
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


// ---------------------------------------------------------------------------------------------------------------------
// PyJFamily members, get/setter, methods


pvt PyMemberDef PyJFamily_members[] = {
        {"name", Py_T_OBJECT_EX, offsetof(PyJFamily, name), Py_READWRITE, "name"},
        {"_overloadByNumArgs", Py_T_OBJECT_EX, offsetof(PyJFamily, _overloadByNumArgs), Py_READWRITE, "list of JOverload"},
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