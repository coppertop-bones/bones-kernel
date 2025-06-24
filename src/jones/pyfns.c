// ---------------------------------------------------------------------------------------------------------------------
// Copyright 2025 David Briant, https://github.com/coppertop-bones. Licensed under the Apache License, Version 2.0
//
// PYFNS - PYTHON NULLARY, UNARY, BINARY, TERNARY FUNCTION CLASSES
// ---------------------------------------------------------------------------------------------------------------------

#ifndef SRC_JONES_PYFNS_C
#define SRC_JONES_PYFNS_C "jones/pyfns.c"

#include "jones.h"

// we could use 0 as a sentinel instead of _? - nice idea but we still need to detect if an object represents the missing object
// bmod should be a dispatcher attribute? we would use it in error messages and repr, the python module is really
// about where the function came from, but the

// tp_alloc inits refcnt to 1

// https://tenthousandmeters.com/blog/python-behind-the-scenes-6-how-python-object-system-works/

// https://peps.python.org/pep-3123/ on making PyObject C standard compliant

//call maps to tp_call
// https://docs.python.org/3/c-api/call.html
// http://etutorials.org/Programming/Python+tutorial/Part+V+Extending+and+Embedding/Chapter+24.+Extending+and+Embedding+Classic+Python/24.1+Extending+Python+with+Pythons+C+API/

// https://docs.python.org/3/c-api/unicode.html#c.PyUnicode_FromFormat
// https://github.com/codie3611/Python-C-API-Advanced-Examples/tree/master/example1_single-inheritance

// https://docs.python.org/3/c-api/typeobj.html#type-objects
// https://docs.python.org/3/c-api/call.html
// https://docs.python.org/3/c-api/tuple.html
// https://docs.python.org/3/c-api/typeobj.html#number-object-structures


// https://docs.python.org/3/c-api/call.html - describes the call protocols
// PyObject_CallMethodNoArgs
// PyObject_CallMethodOneArg
// PyObject_CallFunctionObjArgs()

//define Py_REFCNT(ob)           (((PyObject*)(ob))->ob_refcnt)
//#define Py_TYPE(ob)             (((PyObject*)(ob))->ob_type)
//#define Py_SIZE(ob)             (((PyVarObject*)(ob)

//    if (PyErr_Occurred()) { return -1;}

// PERFORMANCE NOTE - should be able to borrow a tuple when piping, mutate it (!!!) and catch the syntax error - should
// not mutate clients but only tuples owned in this module, PyObject_CallObject - for moment temporarily mutate partial




// https://stackoverflow.com/questions/1104823/python-c-extension-method-signatures-for-documentation
// https://github.com/MSeifert04/iteration_utilities/tree/master/src/iteration_utilities/_iteration_utilities



#define MAX_ARGS 16

#define IS_FN(p) ((p) == &PyNullaryCls || (p) == &PyUnaryCls || (p) == &PyBinaryCls || (p) == &PyTernaryCls)
#define IS_PARTIAL(p) ((p) == &PyPNullaryCls || (p) == &PyPUnaryCls || (p) == &PyPBinaryCls || (p) == &PyPTernaryCls)
#define NotYetImplemented PyExc_NotImplementedError
#define ProgrammerError PyExc_Exception


pvt int Partial_initFromFn(
    struct Partial *self, PyObject *name, PyObject *bmod, PyObject *d, PyObject *TBCSentinel, unsigned char num_tbc,
    PyObject *pipe1, PyObject *args[]
);


pvt PyObject * _nullary_nb_rshift(PyObject *, PyObject *);
pvt PyObject * _pnullary_nb_rshift(PyObject *, PyObject *);
pvt PyObject * _unary_nb_rshift(PyObject *, PyObject *);
pvt PyObject * _punary_nb_rshift(PyObject *, PyObject *);
pvt PyObject * _binary_nb_rshift(PyObject *, PyObject *);
pvt PyObject * _pbinary_nb_rshift(PyObject *, PyObject *);
pvt PyObject * _ternary_nb_rshift(PyObject *, PyObject *);
pvt PyObject * _pternary_nb_rshift(PyObject *, PyObject *);



// ---------------------------------------------------------------------------------------------------------------------
// nullary pipe dispatch
// ---------------------------------------------------------------------------------------------------------------------

pvt PyObject * _nullary_nb_rshift(PyObject *lhs, PyObject *rhs) {
    PyTypeObject *tLhs = Py_TYPE(lhs);  PyTypeObject *tRhs = Py_TYPE(rhs);
    if (tLhs == &PyNullaryCls) {
        struct Fn *fn = (struct Fn*) lhs;
        return PyErr_Format(PyCoppertopSyntaxError, "Arguments cannot by piped into nullary style fn %s.%s", PyUnicode_DATA(fn->bmod), PyUnicode_DATA(fn->name));
    }
    else if (tRhs == &PyNullaryCls) {
        struct Fn *fn = (struct Fn*) rhs;
        return PyErr_Format(PyCoppertopSyntaxError, "Arguments cannot by piped into nullary style fn %s.%s", PyUnicode_DATA(fn->bmod), PyUnicode_DATA(fn->name));
    }
    else
        return PyErr_Format(ProgrammerError, "_nullary_nb_rshift - unhandled case");
}


pvt PyObject * _pnullary_nb_rshift(PyObject *lhs, PyObject *rhs) {
    PyTypeObject *tLhs = Py_TYPE(lhs);  PyTypeObject *tRhs = Py_TYPE(rhs);
    if (tLhs == &PyPNullaryCls) {
        struct Fn *fn = (struct Fn*) lhs;
        return PyErr_Format(PyCoppertopSyntaxError, "Arguments cannot by piped into nullary style fn %s.%s", PyUnicode_DATA(fn->bmod), PyUnicode_DATA(fn->name));
    }
    else if (tRhs == &PyPNullaryCls) {
        struct Fn *fn = (struct Fn*) rhs;
        return PyErr_Format(PyCoppertopSyntaxError, "Arguments cannot by piped into nullary style fn %s.%s", PyUnicode_DATA(fn->bmod), PyUnicode_DATA(fn->name));
    }
    else
        return PyErr_Format(ProgrammerError, "_pnullary_nb_rshift - unhandled case");
}



// ---------------------------------------------------------------------------------------------------------------------
// unary pipe dispatch
//
// 1) _unary >> argN        - syntax error
// 2) _punary >> argN       - syntax error
// 3) pipe1 >> _unary       - dispatch, arg1 cannot be a class from this module as it would already have been piped
// 4) pipe1 >> _punary      - dispatch, arg1 cannot be a class from this module as it would already have been piped
//
// important check lhs first to catch errors
//
// ---------------------------------------------------------------------------------------------------------------------

pvt PyObject * _unary_nb_rshift(PyObject *lhs, PyObject *rhs) {
    PyTypeObject *tLhs = Py_TYPE(lhs);  PyTypeObject *tRhs = Py_TYPE(rhs);

    if (tLhs == &PyUnaryCls) {
        if (tRhs == &PyUnaryCls);             // falls through to the below
        else if (tRhs == &PyPUnaryCls) return _punary_nb_rshift(lhs, rhs);
        else if (tRhs == &PyBinaryCls) return _binary_nb_rshift(lhs, rhs);
        else if (tRhs == &PyPBinaryCls) return _pbinary_nb_rshift(lhs, rhs);
        else if (tRhs == &PyTernaryCls) return _ternary_nb_rshift(lhs, rhs);
        else if (tRhs == &PyPTernaryCls) return _pternary_nb_rshift(lhs, rhs);
        else {
            // 1. _unary >> argN - syntax error
            struct Fn *fn = (struct Fn*) lhs;
            return PyErr_Format(PyCoppertopSyntaxError, "First arg to unary style fn %s.%s must be piped from the left", PyUnicode_DATA(fn->bmod), PyUnicode_DATA(fn->name));
        }
    }
    if (tRhs == &PyUnaryCls) {
        // 3. arg1 >> _unary - dispatch, arg1 cannot be a class from this module as it would already have been piped
        struct Fn *fn = (struct Fn*) rhs;
        return PyObject_CallOneArg(fn->d, lhs);
    }
    else
        return PyErr_Format(ProgrammerError, "_unary_nb_rshift - unhandled case");
}


pvt PyObject * _punary_nb_rshift(PyObject *lhs, PyObject *rhs) {
    PyTypeObject *tLhs = Py_TYPE(lhs);  PyTypeObject *tRhs = Py_TYPE(rhs);

    if (tLhs == &PyPUnaryCls) {
        if (tRhs == &PyUnaryCls) return _unary_nb_rshift(lhs, rhs);
        else if (tRhs == &PyPUnaryCls);       // falls through to the below
        else if (tRhs == &PyBinaryCls) return _binary_nb_rshift(lhs, rhs);
        else if (tRhs == &PyPBinaryCls) return _pbinary_nb_rshift(lhs, rhs);
        else if (tRhs == &PyTernaryCls) return _ternary_nb_rshift(lhs, rhs);
        else if (tRhs == &PyPTernaryCls) return _pternary_nb_rshift(lhs, rhs);
        else {
            // 2. _punary >> argN - syntax error
            struct Partial *partial = (struct Partial *) lhs;
            return PyErr_Format(PyCoppertopSyntaxError, "First arg to unary style partial fn %s.%s must be piped from the left", PyUnicode_DATA(partial->Fn.bmod), PyUnicode_DATA(partial->Fn.name));
        }
    }
    if (tRhs == &PyPUnaryCls) {
        // 4. pipe1 >> _punary - dispatch, arg1 cannot be a class from this module as it would already have been piped
        // PyObject_CallObject
        struct Partial * partial = (struct Partial *) rhs;
        if (partial->num_tbc > 1)
            return PyErr_Format(PyCoppertopSyntaxError, "Trying to pipe an argument into unary style partial fn %s.%s that needs a total of %u more arguments", PyUnicode_DATA(partial->Fn.bmod), PyUnicode_DATA(partial->Fn.name), partial->num_tbc);
        // this should not be called re-entrantly but we can't stop it - so detect and throw as it would be hard to debug
        int iPipe1 = 32;
        Py_ssize_t num_args = Py_SIZE(partial);
        for (int o=0; o < num_args; o++) {
            if (partial->args[o] == partial->Fn.TBCSentinel) {
                iPipe1 = o;
                break;
            }
        }
        if (iPipe1 == 32) return PyErr_Format(PyCoppertopSyntaxError, "Can't find the slot for the piped argument - check that unary style partial fn %s.%s has not been reentrantly called", PyUnicode_DATA(partial->Fn.bmod), PyUnicode_DATA(partial->Fn.name));
        partial->args[iPipe1] = lhs;
        PyObject * result = PyObject_Vectorcall(partial->Fn.d, partial->args, num_args, 0);
        partial->args[iPipe1] = partial->Fn.TBCSentinel;
        return result;
    }
    else
        return PyErr_Format(ProgrammerError, "_punary_nb_rshift - unhandled case");
}



// ---------------------------------------------------------------------------------------------------------------------
// binary pipe dispatch
//
// 1. _binary >> arg2      - syntax error
// 2. _pbinary >> arg2     - dispatch
// 3. arg1 >> _binary      - create a partial that can pipe one more argument
// 4. arg1 >> _pbinary     - check this is the first arg, then create a partial that can pipe one more argument
// ---------------------------------------------------------------------------------------------------------------------

pvt PyObject * _binary_nb_rshift(PyObject *lhs, PyObject *rhs) {
    PyTypeObject *tLhs = Py_TYPE(lhs);  PyTypeObject *tRhs = Py_TYPE(rhs);

    if (tLhs == &PyBinaryCls) {
        if (tRhs == &PyUnaryCls) return _unary_nb_rshift(lhs, rhs);
        else if (tRhs == &PyPUnaryCls) return _punary_nb_rshift(lhs, rhs);
        else if (tRhs == &PyBinaryCls);       // falls through to the below
        else if (tRhs == &PyPBinaryCls) return _pbinary_nb_rshift(lhs, rhs);
        else if (tRhs == &PyTernaryCls) return _ternary_nb_rshift(lhs, rhs);
        else if (tRhs == &PyPTernaryCls) return _pternary_nb_rshift(lhs, rhs);
        else {
            // 1. _binary >> argN
            struct Fn *fn = (struct Fn *) lhs;
            return PyErr_Format(PyCoppertopSyntaxError, "First arg to binary style fn %s.%s must be piped from the left", PyUnicode_DATA(fn->bmod), PyUnicode_DATA(fn->name));
        }
    }
    if (tRhs == &PyBinaryCls) {
        // 3. arg1 >> _binary - create a partial that can pipe one more argument
        struct Partial *partial = (struct Partial *) ((&PyPBinaryCls)->tp_alloc(&PyPBinaryCls, 0));       // 0 as don't need to catch any args
        if (partial == 0) Py_RETURN_NOTIMPLEMENTED;
        struct Fn *fn = (struct Fn *) rhs;
        Partial_initFromFn(
            partial,
            (PyObject *) fn->name,
            (PyObject *) fn->bmod,
            (PyObject *) fn->d,
            (PyObject *) fn->TBCSentinel,
            (unsigned char) 2,
            lhs,
            0
        );
        return (PyObject *) partial;
    }
    else
        return PyErr_Format(ProgrammerError, "_binary_nb_rshift - unhandled case");
}


pvt PyObject * _pbinary_nb_rshift(PyObject *lhs, PyObject *rhs) {
    PyTypeObject *tLhs = Py_TYPE(lhs);  PyTypeObject *tRhs = Py_TYPE(rhs);

    if (tLhs == &PyPBinaryCls) {
        // 2. _pbinary >> arg2 - dispatch (unless the is the first argument and the rhs is a function)
        struct Partial *partial = (struct Partial *) lhs;
        if (partial->pipe1 == 0 && (tRhs == &PyUnaryCls || tRhs == &PyBinaryCls || tRhs == &PyTernaryCls || tRhs == &PyPUnaryCls || tRhs == &PyPBinaryCls || tRhs == &PyPTernaryCls)) Py_RETURN_NOTIMPLEMENTED;
        if (partial->pipe1 == 0) return PyErr_Format(PyCoppertopSyntaxError, "Trying to pipe the 2nd argument into binary style partial fn %s.%s but the first argument hasn't been piped yet", PyUnicode_DATA(partial->Fn.bmod), PyUnicode_DATA(partial->Fn.name));
        if (Py_SIZE(partial) == 0)
            return PyObject_CallFunctionObjArgs((PyObject *)partial->Fn.d, partial->pipe1, rhs, 0);
        else {
            // this should not be called re-entrantly (and probably won't) but we can't stop it - so detect and throw as it would be hard to debug
            int iPipe1 = 32;  int iPipe2 = 32;
            Py_ssize_t num_args = Py_SIZE(partial);
            for (int o=0; o < num_args; o++)
                if (partial->args[o] == partial->Fn.TBCSentinel) {
                    if (iPipe1 == 32)
                        iPipe1 = o;
                    else {
                        iPipe2 = o;
                        break;
                    }
                }
            if (iPipe1 == 32) return PyErr_Format(PyCoppertopSyntaxError, "Can't find the slot for the first piped argument - check that binary style partial fn %s.%s has not been reentrantly called", PyUnicode_DATA(partial->Fn.bmod), PyUnicode_DATA(partial->Fn.name));
            if (iPipe2 == 32) return PyErr_Format(PyCoppertopSyntaxError, "Can't find the slot for the second piped argument - check that binary style partial fn %s.%s has not been reentrantly called", PyUnicode_DATA(partial->Fn.bmod), PyUnicode_DATA(partial->Fn.name));
            partial->args[iPipe1] = partial->pipe1;
            partial->args[iPipe2] = rhs;
            PyObject * result = PyObject_Vectorcall(partial->Fn.d, partial->args, num_args, 0);
            partial->args[iPipe1] = partial->Fn.TBCSentinel;
            partial->args[iPipe2] = partial->Fn.TBCSentinel;
            return result;
        }
    }
    else if (tRhs == &PyPBinaryCls) {
        // 4. arg1 >> _pbinary - check this is the first arg, then create a partial that can pipe one more argument
        struct Partial *partial = (struct Partial *) rhs;
        if (partial->num_tbc != 2)
            return PyErr_Format(PyCoppertopSyntaxError, "2 arguments will be piped into binary style partial fn %s.%s - but %u required", PyUnicode_DATA(partial->Fn.bmod), PyUnicode_DATA(partial->Fn.name), partial->num_tbc);
        if (partial->pipe1 != 0)
            return PyErr_Format(PyCoppertopSyntaxError, "First argument has already been piped into binary style partial fn %s.%s", PyUnicode_DATA(partial->Fn.bmod), PyUnicode_DATA(partial->Fn.name));
        // we have to copy as
        // fred = add(1, _, _)
        // x = 1 >> fred >> (2 >> fred >> 3)
        // is valid - so the first copy (from partial to piping mode) cannot be finessed
        Py_ssize_t num_args = Py_SIZE(partial);
        struct Partial *newPartial = (struct Partial *) ((&PyPBinaryCls)->tp_alloc(&PyPBinaryCls, num_args));
        if (newPartial == 0) return 0;            // OPEN raise an error
        Partial_initFromFn(
            newPartial,
            (PyObject *) partial->Fn.name,
            (PyObject *) partial->Fn.bmod,
            (PyObject *) partial->Fn.d,
            (PyObject *) partial->Fn.TBCSentinel,
            (unsigned char) 2,
            lhs,
            partial->args
        );
        return (PyObject *) newPartial;
    }
    else
        return PyErr_Format(ProgrammerError, "_pbinary_nb_rshift - unhandled case");
}



// ---------------------------------------------------------------------------------------------------------------------
// ternary pipe dispatch
//
// 1. _ternary >> arg       - syntax error
// 2. _pternary >> arg2Or3  - if 2 is missing then keep it else it must be 3 so dispatch
// 3. arg1 >> _ternary      - create a partial that can pipe two more arguments
// 4. arg1 >> _pternary     - check this is the first arg, then create a partial that can pipe two more arguments
// ---------------------------------------------------------------------------------------------------------------------

pvt PyObject * _ternary_nb_rshift(PyObject *lhs, PyObject *rhs) {
    PyTypeObject *tLhs = Py_TYPE(lhs);  PyTypeObject *tRhs = Py_TYPE(rhs);

    if (tLhs == &PyTernaryCls) {
        if (tRhs == &PyUnaryCls) return _unary_nb_rshift(lhs, rhs);
        else if (tRhs == &PyPUnaryCls) return _punary_nb_rshift(lhs, rhs);
        else if (tRhs == &PyBinaryCls) return _binary_nb_rshift(lhs, rhs);
        else if (tRhs == &PyPBinaryCls) return _pbinary_nb_rshift(lhs, rhs);
        else if (tRhs == &PyTernaryCls);       // falls through to the below
        else if (tRhs == &PyPTernaryCls) return _pternary_nb_rshift(lhs, rhs);
        else {
            // 1. _binary >> argN
            struct Fn *fn = (struct Fn *) lhs;
            return PyErr_Format(PyCoppertopSyntaxError, "First arg to binary style fn %s.%s must be piped from the left", PyUnicode_DATA(fn->bmod), PyUnicode_DATA(fn->name));
        }
    }

    if (tLhs == &PyTernaryCls) {
        // 1. _ternary >> argN
        struct Fn *fn = (struct Fn *) lhs;
        return PyErr_Format(PyCoppertopSyntaxError, "First arg to ternary style fn %s.%s must be piped from the left", PyUnicode_DATA(fn->bmod), PyUnicode_DATA(fn->name));
    }
    else if (tRhs == &PyTernaryCls) {
        // 3. arg1 >> _ternary - create a partial that can pipe two more arguments
        struct Partial *partial = (struct Partial *) ((&PyPTernaryCls)->tp_alloc(&PyPTernaryCls, 0));     // 0 as don't need to catch any args
        if (partial == 0) return 0;
        struct Fn *fn = (struct Fn *) rhs;
        Partial_initFromFn(
            partial,
            (PyObject *) fn->name,
            (PyObject *) fn->bmod,
            (PyObject *) fn->d,
            (PyObject *) fn->TBCSentinel,
            (unsigned char) 3,
            lhs,
            0
        );
        return (PyObject *) partial;
    }
    else
        return PyErr_Format(ProgrammerError, "_ternary_nb_rshift - unhandled case");
}


pvt PyObject * _pternary_nb_rshift(PyObject *lhs, PyObject *rhs) {
    PyTypeObject *tLhs = Py_TYPE(lhs);  PyTypeObject *tRhs = Py_TYPE(rhs);

    if (tLhs == &PyPTernaryCls) {
        // 2. _pternary >> arg2Or3 - if 2 is missing then keep it else it must be 3 so dispatch
        struct Partial *partial = (struct Partial *) lhs;
        if (partial->pipe1 == 0) return PyErr_Format(PyCoppertopSyntaxError, "Trying to pipe the 2nd argument into ternary style partial fn %s.%s but the first argument hasn't been piped yet", PyUnicode_DATA(partial->Fn.bmod), PyUnicode_DATA(partial->Fn.name));
        if (partial->pipe1 != 0 && partial->pipe2 == 0) {
            // keeping argument 2
            partial->pipe2 = Py_NewRef(rhs);
            return Py_NewRef((PyObject *) partial);
        }
        // dispatch
        if (Py_SIZE(partial) == 0)
            return PyObject_CallFunctionObjArgs((PyObject *) partial->Fn.d, partial->pipe1, partial->pipe2, rhs, 0);
        else {
            // this should not be called re-entrantly (and probably won't) but we can't stop it - so detect and throw as it would be hard to debug
            int iPipe1 = 32;  int iPipe2 = 32;  int iPipe3 = 32;
            Py_ssize_t num_args = Py_SIZE(partial);
            for (int o=0; o < num_args; o++)
                if (partial->args[o] == partial->Fn.TBCSentinel) {
                    if (iPipe1 == 32)
                        iPipe1 = o;
                    else if (iPipe2 == 32)
                        iPipe2 = o;
                    else {
                        iPipe3 = o;
                        break;
                    }
                }
            if (iPipe1 == 32) return PyErr_Format(PyCoppertopSyntaxError, "Can't find the slot for the first piped argument - check that ternary style partial fn %s.%s has not been reentrantly called", PyUnicode_DATA(partial->Fn.bmod), PyUnicode_DATA(partial->Fn.name));
            if (iPipe2 == 32) return PyErr_Format(PyCoppertopSyntaxError, "Can't find the slot for the second piped argument - check that ternary style partial fn %s.%s has not been reentrantly called", PyUnicode_DATA(partial->Fn.bmod), PyUnicode_DATA(partial->Fn.name));
            if (iPipe3 == 32) return PyErr_Format(PyCoppertopSyntaxError, "Can't find the slot for the third piped argument - check that ternary style partial fn %s.%s has not been reentrantly called", PyUnicode_DATA(partial->Fn.bmod), PyUnicode_DATA(partial->Fn.name));
            partial->args[iPipe1] = partial->pipe1;
            partial->args[iPipe2] = partial->pipe2;
            partial->args[iPipe3] = rhs;
            PyObject * result = PyObject_Vectorcall(partial->Fn.d, partial->args, num_args, 0);
            partial->args[iPipe1] = partial->Fn.TBCSentinel;
            partial->args[iPipe2] = partial->Fn.TBCSentinel;
            partial->args[iPipe3] = partial->Fn.TBCSentinel;
            return result;
        }
    }
    else if (tRhs == &PyPTernaryCls) {
        // 4. arg1 >> _pternary - check this is the first arg, then create a partial that can pipe one more argument
        struct Partial *partial = (struct Partial *) rhs;
        if (partial->num_tbc != 3)
            return PyErr_Format(PyCoppertopSyntaxError, "3 arguments will be piped into ternary style partial fn %s.%s - but %u required", PyUnicode_DATA(partial->Fn.bmod), PyUnicode_DATA(partial->Fn.name), partial->num_tbc);
        if (partial->pipe1 != 0)
            return PyErr_Format(PyCoppertopSyntaxError, "First argument has already been piped into ternary style partial fn %s.%s", PyUnicode_DATA(partial->Fn.bmod), PyUnicode_DATA(partial->Fn.name));
        // we have to copy as
        // fred = add(1, _, _)
        // x = 1 >> fred >> (2 >> fred >> 3)
        // is valid - so the first copy (from partial to piping mode) cannot be finessed
        Py_ssize_t num_args = Py_SIZE(partial);
        struct Partial *newPartial = (struct Partial *) ((&PyPTernaryCls)->tp_alloc(&PyPTernaryCls, num_args));
        if (newPartial == 0) return 0;            // OPEN raise an error
        Partial_initFromFn(
            newPartial,
            (PyObject *) partial->Fn.name,
            (PyObject *) partial->Fn.bmod,
            (PyObject *) partial->Fn.d,
            (PyObject *) partial->Fn.TBCSentinel,
            (unsigned char) 3,
            lhs,
            partial->args
        );
        return (PyObject *) newPartial;
    }
    else
        return PyErr_Format(ProgrammerError, "_pternary_nb_rshift - unhandled case");
}



// ---------------------------------------------------------------------------------------------------------------------
// Fn(...)
// ---------------------------------------------------------------------------------------------------------------------

pvt PyObject * _Fn__call__(struct Fn *fn, PyObject *args, PyObject *kwds) {
    int num_tbc = 0;  Py_ssize_t num_args = PyTuple_GET_SIZE(args);
    if (kwds != 0 && PyDict_Size(kwds) > 0) return PyErr_Format(PyCoppertopSyntaxError, "%s.%s does not take keyword arguments", PyUnicode_DATA(fn->bmod), PyUnicode_DATA(fn->name));
    if (num_args > MAX_ARGS) return PyErr_Format(PyCoppertopSyntaxError, "Maximum number of args for fn %s.%s is %s", PyUnicode_DATA(fn->bmod), PyUnicode_DATA(fn->name), MAX_ARGS);
    for (Py_ssize_t o=0; o < num_args ; o++)
        num_tbc += PyTuple_GET_ITEM(args, o) == fn->TBCSentinel;
    PyTypeObject *t = Py_TYPE(fn);
    if (num_tbc == 0)
        if (t == &PyNullaryCls && num_args >= 0) return PyObject_CallObject(fn->d, args);
        else if (t == &PyUnaryCls && num_args >= 1) return PyObject_CallObject(fn->d, args);
        else if (t == &PyBinaryCls && num_args >= 2) return PyObject_CallObject(fn->d, args);
        else if (t == &PyTernaryCls && num_args >= 3) return PyObject_CallObject(fn->d, args);
        else return PyErr_Format(PyCoppertopSyntaxError, "Not enough args for fn %s.%s", PyUnicode_DATA(fn->bmod), PyUnicode_DATA(fn->name));
    else {
        struct Partial *partial;
        if (t == &PyNullaryCls && num_args >= 0) partial = (struct Partial *) ((&PyPNullaryCls)->tp_alloc(&PyPNullaryCls, num_args));
        else if (t == &PyUnaryCls && num_args >= 1) partial = (struct Partial *) ((&PyPUnaryCls)->tp_alloc(&PyPUnaryCls, num_args));
        else if (t == &PyBinaryCls && num_args >= 2) partial = (struct Partial *) ((&PyPBinaryCls)->tp_alloc(&PyPBinaryCls, num_args));
        else if (t == &PyTernaryCls && num_args >= 3) partial = (struct Partial *) ((&PyPTernaryCls)->tp_alloc(&PyPTernaryCls, num_args));
        else return PyErr_Format(PyCoppertopSyntaxError, "Not enough args for fn %s.%s", PyUnicode_DATA(fn->bmod), PyUnicode_DATA(fn->name));
        if (partial == 0) return 0;
        Partial_initFromFn(
            partial,
            (PyObject *) fn->name,
            (PyObject *) fn->bmod,
            (PyObject *) fn->d,
            (PyObject *) fn->TBCSentinel,
            (unsigned char) num_tbc,
            0,
            0
        );
        // this loop could be eliminated by taking ownership of the args object by keeping the pointer to it but
        // this maybe less cache friendly. downstream on call completion the tuple could be modified on the strict
        // understanding that the dispatcher doesn't keep it but must copy it - TODO time tp_alloc for different sizes
        for (Py_ssize_t o=0; o < num_args ; o++) {
            partial->args[o] = PyTuple_GET_ITEM(args, o);
            Py_XINCREF(partial->args[o]);
        }
        return (PyObject *) partial;
    }
}



// ---------------------------------------------------------------------------------------------------------------------
// Partial(...)
// ---------------------------------------------------------------------------------------------------------------------

pvt PyObject * _Partial__call__(struct Partial *partial, PyObject *args, PyObject *kwds) {
    int new_missing = 0;  Py_ssize_t num_args = PyTuple_GET_SIZE(args);  PyObject * TBC = partial->Fn.TBCSentinel;
    if (kwds != 0 && PyDict_Size(kwds) > 0) return PyErr_Format(PyCoppertopSyntaxError, "%s.%s does not take keyword arguments", PyUnicode_DATA(partial->Fn.bmod), PyUnicode_DATA(partial->Fn.name));
    if (num_args != partial->num_tbc) return PyErr_Format(PyCoppertopSyntaxError, "Wrong number of args to partial fn %s.%s - %l expected, %l given", PyUnicode_DATA(partial->Fn.bmod), PyUnicode_DATA(partial->Fn.name), partial->num_tbc, num_args);
    if (partial->pipe1 != 0) return PyErr_Format(PyCoppertopSyntaxError, "Partial fn %s.%s is now piping - it is no longer callable in fortran style", PyUnicode_DATA(partial->Fn.bmod), PyUnicode_DATA(partial->Fn.name));

    for (Py_ssize_t o=0; o < num_args ; o++) new_missing += PyTuple_GET_ITEM(args, o) == TBC;
    Py_ssize_t full_size = Py_SIZE(partial);
    if (new_missing == 0) {
        // dispatch
        PyObject ** buffer = malloc(sizeof(PyObject*) * full_size);         // we could keep a stack of buffers
        int oNextArg = 0;
        for (Py_ssize_t o=0; o < full_size; o++) {
            PyObject * arg = partial->args[o];
            if (arg == TBC) {
                buffer[o] = PyTuple_GET_ITEM(args, oNextArg);
                oNextArg ++;
            }
            else
                buffer[o] = arg;
        }
        PyObject * result = PyObject_Vectorcall(partial->Fn.d, buffer, full_size, 0);
        free(buffer);
        return result;
    }
    else {
        // create another partial
        struct Partial *new_partial = (struct Partial *) Py_TYPE(partial)->tp_alloc(Py_TYPE(partial), full_size);
        Partial_initFromFn(
            new_partial, partial->Fn.name, partial->Fn.bmod, partial->Fn.d, TBC,
            new_missing, 0, partial->args
        );
        // replace the TBCs with the new args
        int oNextArg = 0;
        for (Py_ssize_t o=0; o < full_size; o++) {
            PyObject * arg = partial->args[o];
            if (arg == TBC) {
                Py_DECREF(arg);
                PyObject * new_arg = PyTuple_GET_ITEM(args, oNextArg);
                new_partial->args[o] = Py_NewRef(new_arg);
                oNextArg ++;
            }
        }
        return (PyObject *) new_partial;
    }
}



// ---------------------------------------------------------------------------------------------------------------------
// Partial misc
// ---------------------------------------------------------------------------------------------------------------------

pvt PyObject * Partial_o_tbc(struct Partial *partial, void* closure) {
    Py_ssize_t full_size = Py_SIZE(partial);  PyObject **args = partial->args;  PyObject * TBC = partial->Fn.TBCSentinel;
    if (partial->pipe1 != 0 || partial->pipe2 != 0) return 0;
    int num_tbc = 0;
    for (Py_ssize_t o=0; o < full_size; o++) num_tbc += (args[o] == TBC);
    PyObject *answer = PyTuple_New(num_tbc);
    if (answer == 0) return 0;
    int o_next = 0;
    for (int o=0; o < full_size; o++) {
        if (args[o] == TBC) {
            PyTuple_SET_ITEM(answer, o_next, PyLong_FromLong(o));
            o_next++;
        }
    }
    return answer;
}

pvt PyObject * Partial_args(struct Partial *partial, void* closure) {
    Py_ssize_t full_size = Py_SIZE(partial);  PyObject **args = partial->args;  PyObject * TBC = partial->Fn.TBCSentinel;
    if (partial->pipe1 != 0 || partial->pipe2 != 0) return 0;
    PyObject *answer = PyTuple_New(full_size);
    if (answer == 0) return 0;
    for (Py_ssize_t o=0; o < full_size; o++) {
        if (args[o] == TBC)
            PyTuple_SET_ITEM(answer, o, Py_NewRef(Py_None));
        else
            PyTuple_SET_ITEM(answer, o, Py_NewRef(args[o]));
    }
    return answer;
}



// ---------------------------------------------------------------------------------------------------------------------
// __array_ufunc__ to work nicely with numpy
// ---------------------------------------------------------------------------------------------------------------------

// https://numpy.org/doc/stable/reference/c-api/ufunc.html#c.PyUFunc_RegisterLoopForType
// https://numpy.org/devdocs/reference/c-api/ufunc.html#c.PyUFunc_ReplaceLoopBySignature
// https://numpy.org/doc/stable/user/c-info.beyond-basics.html
// multi methods in numpy https://technicaldiscovery.blogspot.com/2013/07/thoughts-after-scipy-2013-and-specific.html
// https://numpy.org/doc/1.14/neps/ufunc-overrides.html
// https://numpy.org/doc/1.14/reference/generated/numpy.right_shift.html#numpy.right_shift
// https://stackoverflow.com/questions/55386602/how-to-overide-numpy-ufunc-with-array-ufunc
//
// the last one provides a way to co-exist with numpy - there may be more performant ones (and we can revisit later)
// and this issue may arise with other ndarray implementations (or other libraries that want to use >>), but we'll
// cross that bridge when we come to it
//
//    def __array_ufunc__(self, *args, **kwargs):
//        f"{self.name}.__array_ufunc__   args: {args}\n" >> PP
//        if kwargs: print(kwargs)
//        return 5
//
// np.array(5) >> fn
// fred.__array_ufunc__   args: (<ufunc 'right_shift'>, '__call__', array(5), fred)

pvt PyObject * _Common__array_ufunc__(struct Fn *fn, PyObject *args, PyObject *kwds) {
    Py_ssize_t num_args = PyTuple_GET_SIZE(args);
    if (kwds != 0 && PyDict_Size(kwds) > 0) return PyErr_Format(PyCoppertopSyntaxError, "fn %s.%s.__array_ufunc__ does not take keyword arguments", PyUnicode_DATA(fn->bmod), PyUnicode_DATA(fn->name));
    if (num_args != 4) return PyErr_Format(PyCoppertopSyntaxError, "Wrong number of args to fn %s.%s.__array_ufunc__ - %l expected, %l given", PyUnicode_DATA(fn->bmod), PyUnicode_DATA(fn->name), 4, num_args);
    // MORE ERROR DETECTION?
    PyObject *lhs = PyTuple_GET_ITEM(args, 2);
    PyObject *rhs = PyTuple_GET_ITEM(args, 3);
    PyTypeObject *cls = Py_TYPE(rhs);
    if (cls == &PyUnaryCls) return _unary_nb_rshift(lhs, rhs);
    if (cls == &PyBinaryCls) return _binary_nb_rshift(lhs, rhs);
    if (cls == &PyTernaryCls) return _ternary_nb_rshift(lhs, rhs);
    if (cls == &PyPUnaryCls) return _punary_nb_rshift(lhs, rhs);
    if (cls == &PyPBinaryCls) return _pbinary_nb_rshift(lhs, rhs);
    if (cls == &PyPTernaryCls) return _pternary_nb_rshift(lhs, rhs);
    Py_RETURN_NOTIMPLEMENTED;
}



// ---------------------------------------------------------------------------------------------------------------------
// Fn lifecycle
// ---------------------------------------------------------------------------------------------------------------------

pvt void Fn_dealloc(struct Fn *self) {
    Py_XDECREF(self->name);
    Py_XDECREF(self->bmod);
    Py_XDECREF(self->d);
    Py_XDECREF(self->TBCSentinel);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

pvt PyObject * Fn_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    if (PyTuple_GET_SIZE(args) != 4) return PyErr_Format(ProgrammerError, "Must be created as Fn(name, bmod, d, TBCSentinel)");
    return type->tp_alloc(type, 0);
}

pvt int Fn_init(struct Fn *self, PyObject *args, PyObject *kwds) {
    PyObject *name, *bmod, *d, *TBCSentinel;
    if (!PyArg_ParseTuple(args, "UUOO:", &name, &bmod, &d, &TBCSentinel)) return -1;
    // OPEN: check type of other args
    self->name = Py_NewRef(name);
    self->bmod = Py_NewRef(bmod);
    if (!PyCallable_Check(d)) {PyErr_Format(PyExc_TypeError, "d is not a callable"); return -1;}
    self->d = Py_NewRef(d);
    self->TBCSentinel = Py_NewRef(TBCSentinel);
    return 0;
}



// ---------------------------------------------------------------------------------------------------------------------
// Partial lifecycle
// ---------------------------------------------------------------------------------------------------------------------

pvt void Partial_dealloc(struct Partial *self) {
    Py_DECREF(self->Fn.name);
    Py_DECREF(self->Fn.bmod);
    Py_DECREF(self->Fn.d);
    Py_DECREF(self->Fn.TBCSentinel);
    Py_XDECREF(self->pipe1);
    Py_XDECREF(self->pipe2);
    // decref each arg
    Py_ssize_t num_args = Py_SIZE(self);
    if (num_args > 0) {
        for (Py_ssize_t o=0; o < num_args ; o++) {
            Py_XDECREF(self->args[o]);
        }
    }
    Py_TYPE(self)->tp_free((PyObject *) self);
}

pvt int Partial_initFromFn(
        struct Partial *self, PyObject *name, PyObject *bmod, PyObject *d, PyObject *TBCSentinel, unsigned char num_tbc,
        PyObject *pipe1, PyObject *args[]) {
    Py_ssize_t num_args;  Py_ssize_t i;
    self->Fn.name = Py_NewRef(name);
    self->Fn.bmod = Py_NewRef(bmod);
    self->Fn.d = Py_NewRef(d);
    self->Fn.TBCSentinel = Py_NewRef(TBCSentinel);
    self->num_tbc = num_tbc;
    self->pipe1 = Py_XNewRef(pipe1);
    self->pipe2 = 0;
    num_args = Py_SIZE(self);
    if (args != 0) {
        for (i = 0; i < num_args ; i++) {
            self->args[i] = Py_XNewRef(args[i]);
        }
    }
    return 0;
}



// ---------------------------------------------------------------------------------------------------------------------
// members, get/setter, methods
// ---------------------------------------------------------------------------------------------------------------------

pvt PyObject * Fn_get_doc(struct Fn *self, void *closure) {
    return PyUnicode_FromString("Fn...");
}

pvt PyObject * Fn_get_d(struct Fn *self, void *closure) {
    return Py_NewRef(self->d);
}

pvt int Fn_set_d(struct Fn *self, PyObject *d, void* closure) {
    if (!PyCallable_Check(d)) {
        PyErr_Format(PyExc_TypeError, "d is not a callable");
        return -1;
    }
    Py_XDECREF(self->d);
    self->d = Py_NewRef(d);
    return 0;
}

pvt PyGetSetDef Fn_getsetters[] = {
    {"d", (getter) Fn_get_d, (setter) Fn_set_d, "dispatcher", 0},
    {"__doc__", (getter) Fn_get_doc, 0, 0, 0},
    {0}
};

pvt PyMemberDef Fn_members[] = {
    {"name", Py_T_OBJECT_EX, offsetof(struct Fn, name), Py_READONLY, "function name"},
    {"bmod", Py_T_OBJECT_EX, offsetof(struct Fn, bmod), Py_READONLY, "bones module name"},
    {0}
};

pvt PyMethodDef Fn_methods[] = {
    {"__array_ufunc__", (PyCFunction) _Common__array_ufunc__, METH_VARARGS | METH_KEYWORDS, "__array_ufunc__"},
    {0}
};

pvt PyGetSetDef Partial_getsetters[] = {
    {"o_tbc", (getter) Partial_o_tbc, 0, "offsets of missing arguments", 0},
    {"args", (getter) Partial_args, 0, "arguments thus far", 0},
    {0}
};

pvt PyMemberDef Partial_members[] = {
    {"name", Py_T_OBJECT_EX, offsetof(struct Partial, Fn.name), Py_READONLY, "function name"},
    {"bmod", Py_T_OBJECT_EX, offsetof(struct Partial, Fn.bmod), Py_READONLY, "bones module name"},
    {"d", Py_T_OBJECT_EX, offsetof(struct Partial, Fn.d), Py_READONLY, "dispatcher"},
    {"num_tbc", Py_T_UBYTE, offsetof(struct Partial, num_tbc), Py_READONLY, "number of argument to be confirmed"},
    {"num_args", Py_T_UBYTE, offsetof(PyVarObject, ob_size), Py_READONLY, "total number of arguments"},
    {"pipe1", Py_T_OBJECT_EX, offsetof(struct Partial, pipe1), Py_READONLY, "first piped arg"},
    {"pipe2", Py_T_OBJECT_EX, offsetof(struct Partial, pipe2), Py_READONLY, "second piped arg"},
    {0}
};

pvt PyMethodDef Partial_methods[] = {
    {"__array_ufunc__", (PyCFunction) _Common__array_ufunc__, METH_VARARGS | METH_KEYWORDS, "__array_ufunc__"},
    {0}
};


pvt PyNumberMethods _nullary_tp_as_number = {.nb_rshift = (binaryfunc) _nullary_nb_rshift,};
pvt PyNumberMethods _pnullary_tp_as_number = {.nb_rshift = (binaryfunc) _pnullary_nb_rshift,};

pvt PyNumberMethods _unary_tp_as_number = {.nb_rshift = (binaryfunc) _unary_nb_rshift,};
pvt PyNumberMethods _punary_tp_as_number = {.nb_rshift = (binaryfunc) _punary_nb_rshift,};

pvt PyNumberMethods _binary_tp_as_number = {.nb_rshift = (binaryfunc) _binary_nb_rshift,};
pvt PyNumberMethods _pbinary_tp_as_number = {.nb_rshift = (binaryfunc) _pbinary_nb_rshift,};

pvt PyNumberMethods _ternary_tp_as_number = {.nb_rshift = (binaryfunc) _ternary_nb_rshift,};
pvt PyNumberMethods _pternary_tp_as_number = {.nb_rshift = (binaryfunc) _pternary_nb_rshift,};



// ---------------------------------------------------------------------------------------------------------------------
// Python classes
// ---------------------------------------------------------------------------------------------------------------------

pvt PyTypeObject FnCls = {
    PyVarObject_HEAD_INIT(0, 0)
    .tp_name = "jones._fn",
    .tp_basicsize = sizeof(struct Base),
    .tp_itemsize = 0,
    .tp_doc = PyDoc_STR("_fn"),
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
};


pvt PyTypeObject PFnCls = {
    PyVarObject_HEAD_INIT(0, 0)
    .tp_name = "jones._pfn",
    .tp_basicsize = sizeof(struct Base),
    .tp_itemsize = 0,
    .tp_doc = PyDoc_STR("_pfn"),
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
};


pvt PyTypeObject PyNullaryCls = {
    PyVarObject_HEAD_INIT(0, 0)
    .tp_base = &FnCls,
    .tp_name = "jones._nullary",
    .tp_basicsize = sizeof(struct Fn),
    .tp_itemsize = 0,
    .tp_doc = PyDoc_STR("_nullary() - todo delegate to d"),
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = Fn_new,
    .tp_init = (initproc) Fn_init,
    .tp_dealloc = (destructor) Fn_dealloc,
    .tp_members = Fn_members,
    .tp_methods = Fn_methods,
    .tp_getset = Fn_getsetters,
    .tp_call = (ternaryfunc) _Fn__call__,
    .tp_as_number = (PyNumberMethods*) &_nullary_tp_as_number,
};

pvt PyTypeObject PyPNullaryCls = {
    PyVarObject_HEAD_INIT(0, 0)
    .tp_base = &PFnCls,
    .tp_name = "jones._pnullary",
    .tp_basicsize = sizeof(struct Partial),
    .tp_itemsize = sizeof(PyObject *),
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_dealloc = (destructor) Partial_dealloc,
    .tp_members = Partial_members,
    .tp_methods = Partial_methods,
    .tp_getset = Partial_getsetters,
    .tp_call = (ternaryfunc) _Partial__call__,
    .tp_as_number = (PyNumberMethods*) &_pnullary_tp_as_number,
};


pvt PyTypeObject PyUnaryCls = {
    PyVarObject_HEAD_INIT(0, 0)
    .tp_base = &FnCls,
    .tp_name = "jones._unary",
    .tp_basicsize = sizeof(struct Fn),
    .tp_itemsize = 0,
    .tp_doc = PyDoc_STR("_unary() - todo delegate to d"),
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = Fn_new,
    .tp_init = (initproc) Fn_init,
    .tp_dealloc = (destructor) Fn_dealloc,
    .tp_members = Fn_members,
    .tp_methods = Fn_methods,
    .tp_getset = Fn_getsetters,
    .tp_call = (ternaryfunc) _Fn__call__,
    .tp_as_number = (PyNumberMethods*) &_unary_tp_as_number,
};

pvt PyTypeObject PyPUnaryCls = {
    PyVarObject_HEAD_INIT(0, 0)
    .tp_base = &PFnCls,
    .tp_name = "jones._punary",
    .tp_basicsize = sizeof(struct Partial),
    .tp_itemsize = sizeof(PyObject *),
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_dealloc = (destructor) Partial_dealloc,
    .tp_members = Partial_members,
    .tp_methods = Partial_methods,
    .tp_getset = Partial_getsetters,
    .tp_call = (ternaryfunc) _Partial__call__,
    .tp_as_number = (PyNumberMethods*) &_punary_tp_as_number,
};


pvt PyTypeObject PyBinaryCls = {
    PyVarObject_HEAD_INIT(0, 0)
    .tp_base = &FnCls,
    .tp_name = "jones._binary",
    .tp_basicsize = sizeof(struct Fn),
    .tp_itemsize = 0,
    .tp_doc = PyDoc_STR("_binary() - todo delegate to d"),
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = Fn_new,
    .tp_init = (initproc) Fn_init,
    .tp_dealloc = (destructor) Fn_dealloc,
    .tp_members = Fn_members,
    .tp_methods = Fn_methods,
    .tp_getset = Fn_getsetters,
    .tp_call = (ternaryfunc) _Fn__call__,
    .tp_as_number = (PyNumberMethods*) &_binary_tp_as_number,
};

pvt PyTypeObject PyPBinaryCls = {
    PyVarObject_HEAD_INIT(0, 0)
    .tp_base = &PFnCls,
    .tp_name = "jones._pbinary",
    .tp_basicsize = sizeof(struct Partial),
    .tp_itemsize = sizeof(PyObject *),
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_dealloc = (destructor) Partial_dealloc,
    .tp_members = Partial_members,
    .tp_methods = Partial_methods,
    .tp_getset = Partial_getsetters,
    .tp_call = (ternaryfunc) _Partial__call__,
    .tp_as_number = (PyNumberMethods*) &_pbinary_tp_as_number,
};


pvt PyTypeObject PyTernaryCls = {
    PyVarObject_HEAD_INIT(0, 0)
    .tp_base = &FnCls,
    .tp_name = "jones._ternary",
    .tp_basicsize = sizeof(struct Fn),
    .tp_itemsize = 0,
    .tp_doc = PyDoc_STR("_ternary() - todo delegate to d"),
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = Fn_new,
    .tp_init = (initproc) Fn_init,
    .tp_dealloc = (destructor) Fn_dealloc,
    .tp_members = Fn_members,
    .tp_methods = Fn_methods,
    .tp_getset = Fn_getsetters,
    .tp_call = (ternaryfunc) _Fn__call__,
    .tp_as_number = (PyNumberMethods*) &_ternary_tp_as_number,
};

pvt PyTypeObject PyPTernaryCls = {
    PyVarObject_HEAD_INIT(0, 0)
    .tp_base = &PFnCls,
    .tp_name = "jones._pternary",
    .tp_basicsize = sizeof(struct Partial),
    .tp_itemsize = sizeof(PyObject *),
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_dealloc = (destructor) Partial_dealloc,
    .tp_members = Partial_members,
    .tp_methods = Partial_methods,
    .tp_getset = Partial_getsetters,
    .tp_call = (ternaryfunc) _Partial__call__,
    .tp_as_number = (PyNumberMethods*) &_pternary_tp_as_number,
};


#endif  // SRC_JONES_PYFNS_C