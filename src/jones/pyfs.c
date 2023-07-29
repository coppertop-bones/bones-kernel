// ---------------------------------------------------------------------------------------------------------------------
// Copyright 2025 David Briant, https://github.com/coppertop-bones. Licensed under the Apache License, Version 2.0
//
// PYFS - PYTHON INTERFACE TO FUNCTION SELECTION
// ---------------------------------------------------------------------------------------------------------------------

#ifndef SRC_JONES_PYFS_C
#define SRC_JONES_PYFS_C "jones/pyfs.c"


#include "jones.h"
#include "../bk/fs.c"
#include "lib/pyutils.h"
#include "pybtype.c"


static PyObject * Partial_o_tbc(struct Partial *, void *);


// ---------------------------------------------------------------------------------------------------------------------
// function selection api
// ---------------------------------------------------------------------------------------------------------------------

pvt PyObject * _fs_get_result(PyObject *mod, PyObject **args, Py_ssize_t nargs) {
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 2, nargs);
    if (!PyLong_Check(args[0])) return 0;

    FS *fs = PyLong_AsVoidPtr(args[0]);
    Py_ssize_t num_args = PyTuple_Size(args[1]);
    PY_ASSERT_INT_WITHIN_CLOSED(num_args, "numArgs", 1, 16);

    unsigned short *query = P_QUERY(fs);
    unsigned short *array = P_SIG_ARRAY(fs);

    // answer the result
    return PyLong_FromLong(fast_probe_sigs(query, array, fs->slot_width, fs->num_slots));
}


pvt PyObject * _fs_fill_query_slot_with_btypes_of(PyObject *mod, PyObject **pyargs, Py_ssize_t npyargs) {

    //    # get the types of the arguments
    //    hasValue = False  # used to figure if it's just a dispatch query
    //    types = []
    //    for arg in args:
    //        if hasattr(arg, '_t'):
    //            hasValue = True
    //            tArg = arg._t
    //        elif isinstance(arg, type):
    //            tArg = _aliases.get(arg, py)
    //        elif isinstance(arg, BType):
    //            tArg = arg
    //        elif isinstance(arg, jones._fn):
    //            hasValue = True
    //            if arg.__class__ in (jones._nullary, jones._unary, jones._binary, jones._ternary):
    //                tArg = arg.d._t
    //            else:
    //                tArg = arg.d._tPartial(arg.num_args, arg.o_tbc)
    //        else:
    //            hasValue = True
    //            t = type(arg)
    //            if t is _CoWProxy:
    //                t = type(arg._target)  # return the type of thing being proxied
    //            tArg = _aliases.get(t, py)
    //        types.append(tArg)
    //    tArgs = builtins.tuple(types)

    PyObject *maybe;  PyBType *t;
    // (pSc : SC&ptr, args : N**py, BTypeByType : pydict)
    if (npyargs != 5) return jErrWrongNumberOfArgs(FN_NAME, 4, npyargs);

    // get pSC
    if (!PyLong_Check(pyargs[0])) return PyErr_Format(PyExc_TypeError, "pSC, argument 1, is not a ptr (long)");
    FS *fs = PyLong_AsVoidPtr(pyargs[0]);

    // get args
    PyObject *args = pyargs[1];
    if (!PyTuple_Check(args)) return PyErr_Format(PyExc_TypeError, "t, argument 2, is not a tuple");
    Py_ssize_t num_args = PyTuple_Size(args);
    PY_ASSERT_INT_WITHIN_CLOSED(num_args, "numArgs", 1, 16);

    // get BTypeByType
    PyObject *PyBTypeByType = pyargs[2];
    if (!PyDict_Check(PyBTypeByType)) return PyErr_Format(PyExc_TypeError, "BTypeByType, argument 3, is not a dictionary");

    // get py - the BType representing any non indentified python type
    PyObject *py = pyargs[3];
    if (!PyObject_IsInstance(py, (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "py, argument 4, is not a BType");

    // _CoWProxy
    PyObject *_CoWProxy = pyargs[4];
    if (!PyType_Check(_CoWProxy)) return PyErr_Format(PyExc_TypeError, "_CoWProxy, argument 5, is not a python class");

    unsigned short *query, lower, upper;
    query = P_QUERY(fs);

    // if all the arguments are types then hasValue will be false, else if any argument is a value it hasValue will
    // be true, thus for inspection purposes we can return the function itself rather than calling it - enabling the
    // user to check that they are dispatching to the anticipated function
    bool hasValue = false;
    Py_ssize_t o_slot = 1;

    for (uint_fast8_t o = 0; o < num_args; o++) {
        PyObject *arg = PyTuple_GET_ITEM(args, o);
        PyTypeObject *argCls = Py_TYPE(arg);

        // is it a python type? if so look it up in BTypeByType defaulting to py if it's not there
        if (PyType_Check(arg)) {
            maybe = PyDict_GetItem(PyBTypeByType, arg);
            if (maybe != 0) {
                if (!PyObject_IsInstance(maybe, (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "The mapping of args[%l] is not a BType", o);
                t = (PyBType *) maybe;
            }
            else
                t = (PyBType *) py;
            lower = t->btypeid & LOWER_TYPE_MASK;
            upper = t->btypeid & UPPER_TYPE_MASK;
            query[o_slot] = lower;  o_slot++;
            if (upper) {query[o_slot - 1] |= HAS_UPPER_TYPE_FLAG; query[o_slot] = upper >> UPPER_TYPE_SHIFT;  o_slot++;}
        }

        // otherwise, is it a BType?
        else if (PyObject_IsInstance(arg, (PyObject *) &PyBTypeCls)) {
            t = (PyBType *) arg;
            lower = t->btypeid & LOWER_TYPE_MASK;
            upper = (t->btypeid & UPPER_TYPE_MASK);
            query[o_slot] = lower;  o_slot++;
            if (upper) {query[o_slot - 1] |= HAS_UPPER_TYPE_FLAG; query[o_slot] = upper >> UPPER_TYPE_SHIFT;  o_slot++;}
        }

        // otherwise, is it a jones Fn? if so get the type of the whole space
        else if (argCls == &PyNullaryCls || argCls == &PyUnaryCls || argCls == &PyBinaryCls || argCls == &PyTernaryCls) {
            // call the fn.d.get_t(arg)
            struct Fn *f = (struct Fn *) arg;
            PyObject *d = f->d;
            if (!PyCallable_Check(d)) return PyErr_Format(PyExc_TypeError, "args[%l].d is not a callable", 0); // this should be defended in the Fn setter of d
            maybe = PyObject_GetAttrString(d, "_t");
            if (!PyObject_IsInstance(maybe, (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "args[%l].d._t didn't answer a BType", o);
            t = (PyBType *) maybe;
            lower = t->btypeid & LOWER_TYPE_MASK;
            upper = (t->btypeid & UPPER_TYPE_MASK);
            query[o_slot] = lower;  o_slot++;
            if (upper) {query[o_slot - 1] |= HAS_UPPER_TYPE_FLAG; query[o_slot] = upper >> UPPER_TYPE_SHIFT;  o_slot++;}
            hasValue = true;
        }

        // otherwise, is it a jones Partial Fn? if so get the partial type of the overload, i.e. args[%l].d._tPartial(num_args, o_tbc)
        else if (argCls == &PyPNullaryCls || argCls == &PyPUnaryCls || argCls == &PyPBinaryCls || argCls == &PyPTernaryCls ) {
            // call the fn.d.get_t(arg)
            struct Partial *p = (struct Partial *) arg;
            PyObject *d = p->Fn.d;
            if (!PyCallable_Check(d)) return PyErr_Format(PyExc_TypeError, "args[%l].d is not a callable", 0); // this should be defended in the Fn setter of d
            PyObject *_tPartial = PyObject_GetAttrString(d, "_tPartial");
            if (_tPartial == 0) return PyErr_Format(PyExc_TypeError, "args[%l].d._tPartial does not exist", 0);
            if (!PyCallable_Check(_tPartial)) PyErr_Format(PyExc_TypeError, "args[%l].d._tPartial isn't callable", o);
            PyObject * result = PyObject_CallFunctionObjArgs(
                _tPartial,                      // _tPartial method
                PyLong_FromLong(Py_SIZE(p)),    // num_args
                Partial_o_tbc(p, 0),         // o_tbc
                0
            );
            if (result == 0) return 0;    // the call attempt will have set an exception
            if (!PyObject_IsInstance(result, (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "args[%l].d._tPartial didn't answer a BType", o);
            t = (PyBType *) result;
            lower = t->btypeid & LOWER_TYPE_MASK;
            upper = (t->btypeid & UPPER_TYPE_MASK);
            query[o_slot] = lower;  o_slot++;
            if (upper) {query[o_slot - 1] |= HAS_UPPER_TYPE_FLAG; query[o_slot] = upper >> UPPER_TYPE_SHIFT;  o_slot++;}
            hasValue = true;
        }
        else {
            // does it have a _t? i.e. a bones object
            maybe = PyObject_GetAttrString(arg, "_t");
            if (maybe != 0) {
                if (!PyObject_IsInstance(maybe, (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "The _t attribute of args[%l] is not a BType", o);
                t = (PyBType *) maybe;
                lower = t->btypeid & LOWER_TYPE_MASK;
                upper = (t->btypeid & UPPER_TYPE_MASK);
                query[o_slot] = lower;  o_slot++;
                if (upper) {query[o_slot - 1] |= HAS_UPPER_TYPE_FLAG; query[o_slot] = upper >> UPPER_TYPE_SHIFT;  o_slot++;}
                hasValue = true;
                continue;
            }
            else
                PyErr_Clear();

            // given none of the above it must be a python object
            if (argCls == (PyTypeObject *) _CoWProxy) {
                maybe = PyObject_GetAttrString(arg, "_target");
                if (maybe == 0) return PyErr_Format(PyExc_TypeError, "args[%l] is a _CoWProxy but has no attribute _t", o);
                argCls = Py_TYPE(maybe);
            }
            maybe = PyDict_GetItem(PyBTypeByType, (PyObject *) argCls);
            if (maybe != 0) {
                if (!PyObject_IsInstance(maybe, (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "BTypeByType[args[%l]] is not a BType", o);
                t = (PyBType *) maybe;
            }
            else
                t = (PyBType *) py;
            lower = t->btypeid & LOWER_TYPE_MASK;
            upper = (t->btypeid & UPPER_TYPE_MASK);
            query[o_slot] = lower;  o_slot++;
            if (upper) {query[o_slot - 1] |= HAS_UPPER_TYPE_FLAG; query[o_slot] = upper >> UPPER_TYPE_SHIFT;  o_slot++;}
            hasValue = true;
        }
    }
    query[0] = 0x001F & num_args;
    return PyBool_FromLong(hasValue);
}


pvt PyObject * _fs_tArgs_from_query(PyObject *mod, PyObject **params, Py_ssize_t nparams) {
    // (pSc : Selector&ptr, BTypeById : N**BType&pylist) -> N**BType&pytuple

    if (nparams != 2) return jErrWrongNumberOfArgs(FN_NAME, 2, nparams);

    if (!PyLong_Check(params[0])) return 0;
    FS *fs = PyLong_AsVoidPtr(params[0]);

    PyObject *PyBTypeById = params[1];
    if (!PyList_Check(PyBTypeById)) return 0;

    uint_fast8_t num_args = NUM_ARGS_FROM_SLOT_WIDTH(fs->slot_width);
    PyObject *answer = PyTuple_New(num_args);
    if (answer == 0) return 0;

    unsigned short *query = P_QUERY(fs);
    Py_ssize_t o_next = 1;
    for (Py_ssize_t o = 0; o < num_args; o++) {
        btypeid_t btypeid = query[o_next];
        if (btypeid & HAS_UPPER_TYPE_FLAG) {
            o_next++;
            btypeid &= LOWER_TYPE_MASK;                                            // remove the hasUpper flag
            btypeid |= ((query[o_next] & MAX_UPPER_TYPE) << UPPER_TYPE_SHIFT);     // add the upper part
        }
        PyObject *t = PyList_GET_ITEM(PyBTypeById, (Py_ssize_t) btypeid);
        PyTuple_SET_ITEM(answer, o, Py_NewRef(t));
        o_next++;
    }
    return answer;
}


//    Py_ssize_t full_size = Py_SIZE(partial);  PyObject **args = partial->args;  PyObject * TBC = partial->Fn.TBCSentinel;
//    if (partial->pipe1 != 0 || partial->pipe2 != 0) return 0;
//    int num_tbc = 0;
//    for (Py_ssize_t o=0; o < full_size; o++) num_tbc += (args[o] == TBC);

pvt PyObject * _fs_next_free_array_index(PyObject *mod, PyObject **args, Py_ssize_t nargs) {
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    // TODO raise a type error
    if (!PyLong_Check(args[0])) return 0;

    FS *fs = PyLong_AsVoidPtr(args[0]);
    return PyLong_FromLong(FS_next_free_array_index(fs));
}


pvt PyObject * _fs_atArrayPut(PyObject *mod, PyObject **args, Py_ssize_t nargs) {
    // pSC : *fs, index : unsigned char, pSig : unsigned short[], fnId : u16
    if (nargs != 4) return jErrWrongNumberOfArgs(FN_NAME, 4, nargs);
    // TODO raise a type error
    if (!PyLong_Check(args[0])) return 0;
    if (!PyLong_Check(args[1])) return 0;
    if (!PyLong_Check(args[2])) return 0;
    if (!PyLong_Check(args[3])) return 0;

    FS *fs = PyLong_AsVoidPtr(args[0]);
    uint_fast8_t index = (uint_fast8_t) PyLong_AsLong(args[1]);
    PY_ASSERT_INT_WITHIN_CLOSED(index, "index", 1, fs->num_slots);
    unsigned short *sig = PyLong_AsVoidPtr(args[2]);
    unsigned long v = PyLong_AsLong(args[3]);
    PY_ASSERT_INT_WITHIN_CLOSED(v, "v", 0, _64K);

    FS_at_array_put(fs, index, sig, (unsigned short) v);
    return PyLong_FromVoidPtr(fs);
}



// ---------------------------------------------------------------------------------------------------------------------
// lifecycle and accessing
// ---------------------------------------------------------------------------------------------------------------------

pvt PyObject * _fs_create(PyObject *mod, PyObject **args, Py_ssize_t nargs) {
    if (nargs != 2) return jErrWrongNumberOfArgs(FN_NAME, 2, nargs);
    // TODO raise a descriptive type error
    if (!PyLong_Check(args[0])) return 0;
    if (!PyLong_Check(args[1])) return 0;

    unsigned char num_args = (unsigned char) PyLong_AsLong(args[0]);
    unsigned char array_n_slots = (unsigned char) PyLong_AsLong(args[1]);

    size_t size = 0;
    TRAP_PY( FS_required_size(num_args, array_n_slots, &size) );
    FS *fs = malloc(size);
//    FS *fs = malloc(FS_required_size(num_args, array_n_slots));
    if (fs == 0) return 0;
    TRAP_PY( FS_create(fs, num_args, array_n_slots) );
    return PyLong_FromVoidPtr(fs);
}


pvt PyObject * _fs_trash(PyObject *mod, PyObject **args, Py_ssize_t nargs) {
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    // TODO raise a type error
    if (!PyLong_Check(args[0])) return 0;

    void *fs = PyLong_AsVoidPtr(args[0]);
    FS_trash(fs);
    free(fs);
    Py_RETURN_NONE;
}


pvt PyObject * _fs_pQuery(PyObject *mod, PyObject **args, Py_ssize_t nargs) {
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    // TODO raise a type error
    if (!PyLong_Check(args[0])) return 0;

    FS *fs = PyLong_AsVoidPtr(args[0]);
    return PyLong_FromVoidPtr(P_QUERY(fs));
}



// ---------------------------------------------------------------------------------------------------------------------
// testing interface
// ---------------------------------------------------------------------------------------------------------------------

pvt PyObject * _fs_test_slot_width(PyObject *mod, PyObject **params, Py_ssize_t nparams) {
    if (nparams != 1) return jErrWrongNumberOfArgs(FN_NAME, 2, nparams);
    FS *fs = PyLong_AsVoidPtr(params[0]);
    return PyLong_FromLong(fs->slot_width);
}


pvt PyObject * _fs_test_num_slots(PyObject *mod, PyObject **params, Py_ssize_t nparams) {
    if (nparams != 1) return jErrWrongNumberOfArgs(FN_NAME, 2, nparams);
    FS *fs = PyLong_AsVoidPtr(params[0]);
    return PyLong_FromLong(fs->num_slots);
}


pvt PyObject * _fs_test_pArray(PyObject *mod, PyObject **args, Py_ssize_t nargs) {
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    // TODO raise a type error
    if (!PyLong_Check(args[0])) return 0;

    FS *fs = PyLong_AsVoidPtr(args[0]);
    return PyLong_FromVoidPtr(P_SIG_ARRAY(fs));
}


pvt PyObject * _fs_test_fill_query_slot_and_get_result(PyObject *mod, PyObject **args, Py_ssize_t nargs) {
    if (nargs != 2) return jErrWrongNumberOfArgs(FN_NAME, 2, nargs);
    if (!PyLong_Check(args[0])) return 0;
    PyObject *tArgs = args[1];
    if (!PyTuple_Check(tArgs)) return 0;

    FS *fs = PyLong_AsVoidPtr(args[0]);
    Py_ssize_t num_args = PyTuple_Size(args[1]);
    PY_ASSERT_INT_WITHIN_CLOSED(num_args, "numArgs", 1, 16);

    unsigned short *query, *array, lower, upper, upperFlag;
    query = P_QUERY(fs);
    array = P_SIG_ARRAY(fs);

    for (uint_fast8_t o = 0; o < num_args; o++) {
        // get the id from each tArg
        PyBType *tArg = (PyBType *) PyTuple_GetItem(tArgs, o);
        if (!PyObject_IsInstance((PyObject *) tArg, (PyObject *) &PyBTypeCls)) PyErr_Format(PyBTypeError, "Arg is not a BType");
        lower = tArg->btypeid & LOWER_TYPE_MASK;
        upper = (tArg->btypeid & UPPER_TYPE_MASK) >> UPPER_TYPE_SHIFT;
        upperFlag = upper ? HAS_UPPER_TYPE_FLAG : 0;
        PY_ASSERT_INT_WITHIN_CLOSED(upper, "btypeid", 0, MAX_UPPER_TYPE);
        // put btypeid into the query scratchpad
        query[o + 1] = lower | upperFlag;
    }
    // add the size
    query[0] = 0x001F & num_args;

    // answer the result
    return PyLong_FromLong(fast_probe_sigs(query, array, fs->slot_width, fs->num_slots));
}


//pvt PyObject * _fs_test_get_result_for_query(PyObject *mod, PyObject **args, Py_ssize_t nargs) {
//    // pQuery:unsigned short*, pSigs:unsigned short*, slot_width:unsigned char, num_slots:unsigned char
//    if (nargs != 4) return jErrWrongNumberOfArgs(FN_NAME, 4, nargs);
//    // TODO raise a type error
//    if (!PyLong_Check(args[0])) return 0;
//    if (!PyLong_Check(args[1])) return 0;
//    if (!PyLong_Check(args[2])) return 0;
//    if (!PyLong_Check(args[3])) return 0;
//
//    unsigned short *query = PyLong_AsVoidPtr(args[0]);
//    unsigned short *sigs = PyLong_AsVoidPtr(args[1]);
//    unsigned long slot_width = PyLong_AsLong(args[2]);
//    unsigned long num_slots = PyLong_AsLong(args[3]);
//    unsigned long x = 0;
//    for (unsigned long i = 0; i < 1; i++) {
//        x = fast_probe_sigs(query, sigs, slot_width, num_slots);
//    }
////    PP_INT("x: ", x);
//    return PyLong_FromLong(x);          // need to return x else the loop gets optimised away :(
//}



#endif  // SRC_JONES_PYFS_C