#ifndef JONES_SELECT_C
#define JONES_SELECT_FN_C "jones/select_fn.c"


#include "pipe_structs.c"
#include "../../include/bk/bk.h"
#include "utils.c"
#include "py_btype.c"


static PyObject * Partial_o_tbc(struct Partial *, void *);      // from pipe_ops.c

static PyTypeObject NullaryCls;
static PyTypeObject PNullaryCls;
static PyTypeObject UnaryCls;
static PyTypeObject PUnaryCls;
static PyTypeObject BinaryCls;
static PyTypeObject PBinaryCls;
static PyTypeObject TernaryCls;
static PyTypeObject PTernaryCls;


pvt PyObject * _SC_fill_query_slot_and_get_result(PyObject *mod, PyObject *const *args, Py_ssize_t nargs) {
    if (nargs != 2) return _raiseWrongNumberOfArgs(__FUNCTION__, 2, nargs);
    if (!PyLong_Check(args[0])) return 0;
    PyObject *tArgs = args[1];
    if (!PyTuple_Check(tArgs)) return 0;

    SelectorCache *sc = PyLong_AsVoidPtr(args[0]);
    Py_ssize_t num_args = PyTuple_Size(args[1]);
    PY_ASSERT_INT_WITHIN_CLOSED(num_args, "numArgs", 1, 16);

    TypeNum *query = P_QUERY(sc);
    TypeNum *array = P_SIG_ARRAY(sc);

    for (fu8 o = 0; o < num_args; o++) {
        // get the id from each tArg
        struct PyBType *tArg = (struct PyBType *) PyTuple_GetItem(tArgs, o);
        if (!PyObject_IsInstance((PyObject *) tArg, (PyObject *) &PyBTypeCls)) PyErr_Format(JonesError, "Arg is not a BType");
        TypeNum TN1 = tArg->id & 0xFFFF;
        TypeNum TN2 = (tArg->id & 0xFFFF0000) >> 16;
        PY_ASSERT_INT_WITHIN_CLOSED(TN1, "id", 1, MAX_NUM_T1_TYPES);
        // put typeNum into the query scratchpad
        query[o + 1] = TN1;                //TODO handle T2
    }
    // add the size
    query[0] = 0x001F & num_args;

    // answer the result
    return PyLong_FromLong(fast_probe_sigs(query, array, sc->slot_width, sc->num_slots));
}


pvt PyObject * _SC_get_result(PyObject *mod, PyObject *const *args, Py_ssize_t nargs) {
    if (nargs != 1) return _raiseWrongNumberOfArgs(__FUNCTION__, 2, nargs);
    if (!PyLong_Check(args[0])) return 0;

    SelectorCache *sc = PyLong_AsVoidPtr(args[0]);
    Py_ssize_t num_args = PyTuple_Size(args[1]);
    PY_ASSERT_INT_WITHIN_CLOSED(num_args, "numArgs", 1, 16);

    TypeNum *query = P_QUERY(sc);
    TypeNum *array = P_SIG_ARRAY(sc);

    // answer the result
    return PyLong_FromLong(fast_probe_sigs(query, array, sc->slot_width, sc->num_slots));
}


pvt PyObject * _SC_fill_query_slot_with_btypes_of(PyObject *mod, PyObject *const *pyargs, Py_ssize_t npyargs) {

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

    PyObject *maybe;  struct PyBType *t;
    // (pSc : SC&ptr, args : N**py, BTypeByType : pydict)
    if (npyargs != 5) return _raiseWrongNumberOfArgs(__FUNCTION__, 4, npyargs);

    // get pSC
    if (!PyLong_Check(pyargs[0])) return PyErr_Format(PyExc_TypeError, "pSC, argument 1, is not a ptr (long)");
    SelectorCache *sc = PyLong_AsVoidPtr(pyargs[0]);

    // get args
    PyObject *args = pyargs[1];
    if (!PyTuple_Check(args)) return PyErr_Format(PyExc_TypeError, "t, argument 2, is not a tuple");
    pyssize num_args = PyTuple_Size(args);
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

    TypeNum *query, TN1, TN2;
    query = P_QUERY(sc);

    // if all the arguments are types then hasValue will be false, else if any argument is a value it hasValue will
    // be true, thus for inspection purposes we can return the function itself rather than calling it - enabling the
    // user to check that they are dispatching to the anticipated function
    bool hasValue = false;
    pyssize o_slot = 1;

    for (fu8 o = 0; o < num_args; o++) {
        PyObject *arg = PyTuple_GET_ITEM(args, o);
        PyTypeObject *argCls = Py_TYPE(arg);

        // is it a python type? if so look it up in BTypeByType defaulting to py if it's not there
        if (PyType_Check(arg)) {
            maybe = PyDict_GetItem(PyBTypeByType, arg);
            if (maybe != 0) {
                if (!PyObject_IsInstance(maybe, (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "The mapping of args[%l] is not a BType", o);
                t = (struct PyBType *) maybe;
            }
            else
                t = (struct PyBType *) py;
            TN1 = t->id & 0xFFFF;
            TN2 = (t->id & 0xFFFF0000) >> 16;
            query[o_slot] = TN1;  o_slot++;
            if (TN1 & HAS_TN2_MASK) {query[o_slot] = TN2;  o_slot++;}
        }

        // otherwise, is it a BType?
        else if (PyObject_IsInstance(arg, (PyObject *) &PyBTypeCls)) {
            t = (struct PyBType *) arg;
            TN1 = t->id & 0xFFFF;
            TN2 = (t->id & 0xFFFF0000) >> 16;
            query[o_slot] = TN1;  o_slot++;
            if (TN1 & HAS_TN2_MASK) {query[o_slot] = TN2;  o_slot++;}
        }

        // otherwise, is it a jones Fn? if so get the type of the whole family
        else if (argCls == &NullaryCls || argCls == &UnaryCls || argCls == &BinaryCls || argCls == &TernaryCls) {
            // call the fn.d.get_t(arg)
            struct Fn *f = (struct Fn *) arg;
            PyObject *d = f->d;
            if (!PyCallable_Check(d)) return PyErr_Format(PyExc_TypeError, "args[%l].d is not a callable", 0); // this should be defended in the Fn setter of d
            maybe = PyObject_GetAttrString(d, "_t");
            if (!PyObject_IsInstance(maybe, (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "args[%l].d._t didn't answer a BType", o);
            t = (struct PyBType *) maybe;
            TN1 = t->id & 0xFFFF;
            TN2 = (t->id & 0xFFFF0000) >> 16;
            query[o_slot] = TN1;  o_slot++;
            if (TN1 & HAS_TN2_MASK) {query[o_slot] = TN2;  o_slot++;}
            hasValue = true;
        }

        // otherwise, is it a jones Partial Fn? if so get the partial type of the overload, i.e. args[%l].d._tPartial(num_args, o_tbc)
        else if (argCls == &PNullaryCls || argCls == &PUnaryCls || argCls == &PBinaryCls || argCls == &PTernaryCls ) {
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
            t = (struct PyBType *) result;
            TN1 = t->id & 0xFFFF;
            TN2 = (t->id & 0xFFFF0000) >> 16;
            query[o_slot] = TN1;  o_slot++;
            if (TN1 & HAS_TN2_MASK) {query[o_slot] = TN2;  o_slot++;}
            hasValue = true;
        }
        else {
            // does it have a _t? i.e. a bones object
            maybe = PyObject_GetAttrString(arg, "_t");
            if (maybe != 0) {
                if (!PyObject_IsInstance(maybe, (PyObject *) &PyBTypeCls)) return PyErr_Format(PyExc_TypeError, "The _t attribute of args[%l] is not a BType", o);
                t = (struct PyBType *) maybe;
                TN1 = t->id & 0xFFFF;
                TN2 = (t->id & 0xFFFF0000) >> 16;
                query[o_slot] = TN1;  o_slot++;
                if (TN1 & HAS_TN2_MASK) {query[o_slot] = TN2;  o_slot++;}
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
                t = (struct PyBType *) maybe;
            }
            else
                t = (struct PyBType *) py;
            TN1 = t->id & 0xFFFF;
            TN2 = (t->id & 0xFFFF0000) >> 16;
            query[o_slot] = TN1;  o_slot++;
            if (TN1 & HAS_TN2_MASK) {query[o_slot] = TN2;  o_slot++;}
            hasValue = true;
        }
    }
    query[0] = 0x001F & num_args;
    return PyBool_FromLong(hasValue);
}


pvt PyObject * _SC_tArgs_from_query(PyObject *mod, PyObject *const *params, pyssize nparams) {
    // (pSc : Selector&ptr, BTypeById : N**BType&pylist) -> N**BType&pytuple

    if (nparams != 2) return _raiseWrongNumberOfArgs(__FUNCTION__, 2, nparams);

    if (!PyLong_Check(params[0])) return 0;
    SelectorCache *sc = PyLong_AsVoidPtr(params[0]);

    PyObject *PyBTypeById = params[1];
    if (!PyList_Check(PyBTypeById)) return 0;

    fu8 num_args = NUM_ARGS_FROM_SLOT_WIDTH(sc->slot_width);
    PyObject *answer = PyTuple_New(num_args);
    if (answer == 0) return 0;

    TypeNum *query = P_QUERY(sc);
    pyssize o_next = 1;
    for (pyssize o = 0; o < num_args; o++) {
        unsigned int typenum = query[o_next];
        if (typenum & HAS_TN2_MASK) {
            o_next++;
            typenum = typenum | (query[o_next] << TN2_SHIFT);
        }
        PyObject *t = PyList_GET_ITEM(PyBTypeById, (pyssize) typenum);
        Py_INCREF(t);
        PyTuple_SET_ITEM(answer, o, t);
        o_next++;
    }
    return answer;
}


//    pyssize full_size = Py_SIZE(partial);  PyObject **args = partial->args;  PyObject * TBC = partial->Fn.TBCSentinel;
//    if (partial->pipe1 != 0 || partial->pipe2 != 0) return 0;
//    int num_tbc = 0;
//    for (pyssize o=0; o < full_size; o++) num_tbc += (args[o] == TBC);

pvt PyObject * _SC_next_free_array_index(PyObject *mod, PyObject *const *args, pyssize nargs) {
    if (nargs != 1) return _raiseWrongNumberOfArgs(__FUNCTION__, 1, nargs);
    // TODO raise a type error
    if (!PyLong_Check(args[0])) return 0;

    SelectorCache *sc = PyLong_AsVoidPtr(args[0]);
    return PyLong_FromLong(SC_next_free_array_index(sc));
}


pvt PyObject * _SC_atArrayPut(PyObject *mod, PyObject *const *args, pyssize nargs) {
    // pSC : *sc, index : unsigned char, pSig : TypeNum[], fnId : u16
    if (nargs != 4) return _raiseWrongNumberOfArgs(__FUNCTION__, 4, nargs);
    // TODO raise a type error
    if (!PyLong_Check(args[0])) return 0;
    if (!PyLong_Check(args[1])) return 0;
    if (!PyLong_Check(args[2])) return 0;
    if (!PyLong_Check(args[3])) return 0;

    SelectorCache *sc = PyLong_AsVoidPtr(args[0]);
    fu8 index = (fu8) PyLong_AsLong(args[1]);
    PY_ASSERT_INT_WITHIN_CLOSED(index, "index", 1, sc->num_slots);
    TypeNum *sig = PyLong_AsVoidPtr(args[2]);
    unsigned long v = PyLong_AsLong(args[3]);
    PY_ASSERT_INT_WITHIN_CLOSED(v, "v", 0, _64K);

    SC_at_array_put(sc, index, sig, (unsigned short) v);
    return PyLong_FromVoidPtr(sc);
}


pvt PyObject * _SC_get_result_for_query(PyObject *mod, PyObject *const *args, pyssize nargs) {
    // pQuery:TypeNum*, pSigs:TypeNum*, slot_width:unsigned char, num_slots:unsigned char
    if (nargs != 4) return _raiseWrongNumberOfArgs(__FUNCTION__, 4, nargs);
    // TODO raise a type error
    if (!PyLong_Check(args[0])) return 0;
    if (!PyLong_Check(args[1])) return 0;
    if (!PyLong_Check(args[2])) return 0;
    if (!PyLong_Check(args[3])) return 0;

    TypeNum *query = PyLong_AsVoidPtr(args[0]);
    TypeNum *sigs = PyLong_AsVoidPtr(args[1]);
    unsigned long slot_width = PyLong_AsLong(args[2]);
    unsigned long num_slots = PyLong_AsLong(args[3]);
    unsigned long x = 0;
    for (unsigned long i = 0; i < 1; i++) {
        x = fast_probe_sigs(query, sigs, (unsigned char)slot_width, (unsigned char)num_slots);
    }
//    PP_INT("x: ", x);
    return PyLong_FromLong(x);          // need to return x else the loop gets optimised away :(
}



// lifecycle and accessing

pvt PyObject * _SC_new(PyObject *mod, PyObject *const *args, Py_ssize_t nargs) {
    if (nargs != 2) return _raiseWrongNumberOfArgs(__FUNCTION__, 2, nargs);
    // TODO raise a descriptive type error
    if (!PyLong_Check(args[0])) return 0;
    if (!PyLong_Check(args[1])) return 0;

    unsigned char num_args = (unsigned char) PyLong_AsLong(args[0]);
    unsigned char array_n_slots = (unsigned char) PyLong_AsLong(args[1]);

    SelectorCache * sc = malloc(SC_new_size(num_args, array_n_slots));  // OPEN trap error from SC_new_size
    if (sc == 0) return 0;
    TRAP_PY( SC_init(sc, num_args, array_n_slots) );
    return PyLong_FromVoidPtr(sc);
}


pvt PyObject * _SC_drop(PyObject *mod, PyObject *const *args, Py_ssize_t nargs) {
    if (nargs != 1) return _raiseWrongNumberOfArgs(__FUNCTION__, 1, nargs);
    // TODO raise a type error
    if (!PyLong_Check(args[0])) return 0;

    void *sc = PyLong_AsVoidPtr(args[0]);
    SC_drop(sc);
    free(sc);
    Py_RETURN_NONE;
}


pvt PyObject * _SC_slot_width(PyObject *mod, PyObject *const *params, pyssize nparams) {
    if (nparams != 1) return _raiseWrongNumberOfArgs(__FUNCTION__, 2, nparams);
    SelectorCache *sc = PyLong_AsVoidPtr(params[0]);
    return PyLong_FromLong(sc->slot_width);
}


pvt PyObject * _SC_num_slots(PyObject *mod, PyObject *const *params, pyssize nparams) {
    if (nparams != 1) return _raiseWrongNumberOfArgs(__FUNCTION__, 2, nparams);
    SelectorCache *sc = PyLong_AsVoidPtr(params[0]);
    return PyLong_FromLong(sc->num_slots);
}


pvt PyObject * _SC_pQuery(PyObject *mod, PyObject *const *args, pyssize nargs) {
    // pSelectorCache:*Selector
    if (nargs != 1) return _raiseWrongNumberOfArgs(__FUNCTION__, 1, nargs);
    // TODO raise a type error
    if (!PyLong_Check(args[0])) return 0;

    SelectorCache *sc = PyLong_AsVoidPtr(args[0]);
    return PyLong_FromVoidPtr(P_QUERY(sc));
}


// for development purposes
pvt PyObject * _SC_pArray(PyObject *mod, PyObject *const *args, pyssize nargs) {
    // pSelectorCache:*Selector
    if (nargs != 1) return _raiseWrongNumberOfArgs(__FUNCTION__, 1, nargs);
    // TODO raise a type error
    if (!PyLong_Check(args[0])) return 0;

    SelectorCache *sc = PyLong_AsVoidPtr(args[0]);
    return PyLong_FromVoidPtr(P_SIG_ARRAY(sc));
}




#endif  // JONES_SELECT_FN_C