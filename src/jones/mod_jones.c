// ---------------------------------------------------------------------------------------------------------------------
// Copyright 2025 David Briant, https://github.com/coppertop-bones. Licensed under the Apache License, Version 2.0
// ---------------------------------------------------------------------------------------------------------------------

#ifndef SRC_JONES_MOD_JONES_C
#define SRC_JONES_MOD_JONES_C "jones/mod_jones.c"


#include "jones.h"
#include "../bk/lib/os.c"
#include "../bk/k.c"
#include "pyfns.c"
#include "pybtype.c"
#include "pysm.c"
#include "pyem.c"
#include "pytm.c"
#include "pyfs.c"
#include "pykernel.c"
#include "pytvfunc.c"



// ---------------------------------------------------------------------------------------------------------------------
// jones module
// ---------------------------------------------------------------------------------------------------------------------

pvt PyMethodDef jones_fns[] = {
    {"sc_new", (PyCFunction)                        _fs_create, METH_FASTCALL, "sc_new(numArgs, arrayLen) -> pSC"},
    {"sc_drop", (PyCFunction)                       _fs_trash, METH_FASTCALL, "sc_drop(pSC) -> None"},
    {"sc_nextFreeArrayIndex", (PyCFunction)         _fs_next_free_array_index, METH_FASTCALL, ""},
    {"sc_atArrayPut", (PyCFunction)                 _fs_atArrayPut, METH_FASTCALL, "sc_atArrayPut(pSC, index, pSig, fnId) -> Void\n\nin pSig belonging to pSC, at index put fnId"},
    {"sc_queryPtr", (PyCFunction)                   _fs_pQuery, METH_FASTCALL, "scQueryPtr(pSC)\n\nanswer a pointer to the query buffer"},
    {"sc_getFnId", (PyCFunction)                    _fs_get_result, METH_FASTCALL, ""},
    {"sc_tArgsFromQuery", (PyCFunction)             _fs_tArgs_from_query, METH_FASTCALL, "sc_tArgsFromQuery(pSC : ptr, allTypes : pylist)\n\nanswers a tuple of tArgs from the slot"},
    {"sc_fillQuerySlotWithBTypesOf", (PyCFunction)  _fs_fill_query_slot_with_btypes_of, METH_FASTCALL, "sc_fillQuerySlotWithBTypesOf(pSC : ptr, args : tuple)\n\nanswers a tuple of tArgs from the slot"},

    {"sc_test_arrayPtr", (PyCFunction)              _fs_test_pArray, METH_FASTCALL, "sc_test_arrayPtr(pSC)\n\nanswer a pointer to the array of sigs"},
    {"sc_test_slotWidth", (PyCFunction)             _fs_test_slot_width, METH_FASTCALL, "sc_test_slotWidth(pSC) -> count"},
    {"sc_test_numSlots", (PyCFunction)              _fs_test_num_slots, METH_FASTCALL, "sc_test_numSlots(pSC) -> count"},
    {"sc_test_fillQuerySlotAndGetFnId", (PyCFunction) _fs_test_fill_query_slot_and_get_result, METH_FASTCALL, "sc_test_fillQuerySlotAndGetFnId(pSC, tArgs : pytuple) -> fnId\n\nanswer the resultId for the signature tArgs"},

    //  tvfunc, overload, family functions
    {"disableReturnCheck", (PyCFunction)            setDisableReturnCheck,          METH_FASTCALL, "disableReturnCheck(bool) -> None"},
    {"set_typeOf", (PyCFunction)                    set_typeOf_pyfn,                METH_FASTCALL, "set_typeOf(callable) -> None"},
    {"set_distancesEtAl", (PyCFunction)             set_distancesEtAl_pyfn,         METH_FASTCALL, "set_distancesEtAl(callable) -> None"},
    {"set_fitsWithin",  (PyCFunction)               set_fitsWithin_pyfn,            METH_FASTCALL, "set_fitsWithin(callable) -> None"},
    {"set_tvfuncErrorCallback1", (PyCFunction)      set_tvfuncErrorCallback1_pyfn,  METH_FASTCALL, "set_tvfuncErrorCallback1(callable) -> None"},
    {"set_tvfuncErrorCallback2", (PyCFunction)      set_tvfuncErrorCallback2_pyfn,  METH_FASTCALL, "set_tvfuncErrorCallback2(callable) -> None"},
    {"set_updateSchemaVarsWith", (PyCFunction)      set_updateSchemaVarsWith_pyfn,  METH_FASTCALL, "set_updateSchemaVarsWith(callable) -> None"},
    {"set_BType_py", (PyCFunction)                  set_BType_py,                   METH_FASTCALL, "set_BType_py(BType) -> None"},
    {"_distancesEtAl", (PyCFunction)                _distancesEtAl,                 METH_FASTCALL, "distancesEtAl(callerSig, fnSig) -> fnId\n\nanswers match, fallback, schemaVars and argDistances for the given callerSig and fnSig"},

    {0}
};

pvt PyModuleDef jones_module = {
    PyModuleDef_HEAD_INIT,
    .m_name = "jones",
    .m_doc = "as in archibald",
    .m_size = -1,
    jones_fns
};


pub PyMODINIT_FUNC PyInit_jones(void) {
    // https://docs.python.org/3/c-api/module.html#c.PyModule_AddObject
    PyObject *m;

    m = PyModule_Create(&jones_module);
    if (m == 0) {return PyErr_Format(PyExc_ImportError, "PySMCls is not ready, %s, %i", __FILE__, __LINE__);}

    g_logging_level = info;

    // import threading and sys modules
    threadingModule = PyImport_ImportModule("threading");
    if (!threadingModule) return PyErr_Format(PyExc_ImportError, "Can't import threading!");
    sysModule = PyImport_ImportModule("sys");
    if (!sysModule) return PyErr_Format(PyExc_ImportError, "Can't import sys!");

    _disableReturnCheck = false;


    // PyJonesError
    PyJonesError = PyErr_NewException("jones.JonesError", 0, 0);
    if (PyModule_AddObject(m, "JonesError", PyJonesError) < 0) {
        Py_XDECREF(PyJonesError);
        Py_CLEAR(PyJonesError);
        Py_DECREF(m);
        return PyErr_Format(PyExc_ImportError, "Could not add PyJonesError, %s, %i", __FILE__, __LINE__);
    }

    // PyCoppertopSyntaxError
    PyCoppertopSyntaxError = PyErr_NewException("jones.CoppertopSyntaxError", PyExc_SyntaxError, 0);
    if (PyModule_AddObject(m, "CoppertopSyntaxError", PyCoppertopSyntaxError) < 0) {
        Py_XDECREF(PyCoppertopSyntaxError);
        Py_CLEAR(PyCoppertopSyntaxError);
        Py_DECREF(m);
        return PyErr_Format(PyExc_ImportError, "Could not add PyCoppertopSyntaxError, %s, %i", __FILE__, __LINE__);
    }

    // PyBTypeError
    PyBTypeError = PyErr_NewException("jones.BTypeError", PyExc_TypeError, 0);
    if (PyModule_AddObject(m, "BTypeError", PyBTypeError) < 0) {
        Py_XDECREF(PyBTypeError);
        Py_CLEAR(PyBTypeError);
        Py_DECREF(m);
        return PyErr_Format(PyExc_ImportError, "Could not add PyBTypeError, %s, %i", __FILE__, __LINE__);
    }

    // PyBTypeCls
    if (PyType_Ready(&PyBTypeCls) < 0) {return PyErr_Format(PyExc_ImportError, "PyBTypeCls is not ready, %s, %i", __FILE__, __LINE__);}
    if (PyModule_AddObject(m, "BType", (PyObject *) &PyBTypeCls) < 0) {
        Py_DECREF(&PyBTypeCls);
        Py_DECREF(m);
        return 0;
    }

    // PyKernelCls
    if (PyType_Ready(&PyKernelCls) < 0) {return PyErr_Format(PyExc_ImportError, "PyKernelCls is not ready, %s, %i", __FILE__, __LINE__);}
    if (PyModule_AddObject(m, "Kernel", (PyObject *) &PyKernelCls) < 0) {
        Py_DECREF(&PyKernelCls);
        Py_DECREF(m);
        return 0;
    }

    // PySMCls
    if (PyType_Ready(&PySMCls) < 0) {return PyErr_Format(PyExc_ImportError, "PySMCls is not ready, %s, %i", __FILE__, __LINE__);}
    if (PyModule_AddObject(m, "SM", (PyObject *) &PySMCls) < 0) {
        Py_DECREF(&PySMCls);
        Py_DECREF(m);
        return 0;
    }

    // PyEMCls
    if (PyType_Ready(&PyEMCls) < 0) {return PyErr_Format(PyExc_ImportError, "PyEMCls is not ready, %s, %i", __FILE__, __LINE__);}
    if (PyModule_AddObject(m, "EM", (PyObject *) &PyEMCls) < 0) {
        Py_DECREF(&PyEMCls);
        Py_DECREF(m);
        return 0;
    }

    // PyTMCls
    if (PyType_Ready(&PyTMCls) < 0) {return PyErr_Format(PyExc_ImportError, "PyTMCls is not ready, %s, %i", __FILE__, __LINE__);}
    if (PyModule_AddObject(m, "TM", (PyObject *) &PyTMCls) < 0) {
        Py_DECREF(&PyTMCls);
        Py_DECREF(m);
        return 0;
    }


    // add pipeable function and partial classes
    if (PyType_Ready(&FnCls) < 0) {return PyErr_Format(PyExc_ImportError, "FnCls is not ready, %s, %i", __FILE__, __LINE__);}
    if (PyModule_AddObject(m, "_fn", (PyObject *) &FnCls) < 0) {
        Py_DECREF(&FnCls);
        Py_DECREF(m);
        return 0;
    }

    if (PyType_Ready(&PFnCls) < 0) {return PyErr_Format(PyExc_ImportError, "PFnCls is not ready, %s, %i", __FILE__, __LINE__);}
    if (PyModule_AddObject(m, "_pfn", (PyObject *) &PFnCls) < 0) {
        Py_DECREF(&PFnCls);
        Py_DECREF(m);
        return 0;
    }

    if (PyType_Ready(&PyNullaryCls) < 0) {return PyErr_Format(PyExc_ImportError, "PyNullaryCls is not ready, %s, %i", __FILE__, __LINE__);}
    if (PyModule_AddObject(m, "_nullary", (PyObject *) &PyNullaryCls) < 0) {
        Py_DECREF(&PyNullaryCls);
        Py_DECREF(m);
        return 0;
    }

    if (PyType_Ready(&PyUnaryCls) < 0) {return PyErr_Format(PyExc_ImportError, "PyUnaryCls is not ready, %s, %i", __FILE__, __LINE__);}
    if (PyModule_AddObject(m, "_unary", (PyObject *) &PyUnaryCls) < 0) {
        Py_DECREF(&PyUnaryCls);
        Py_DECREF(m);
        return 0;
    }

    if (PyType_Ready(&PyBinaryCls) < 0) {return PyErr_Format(PyExc_ImportError, "PyBinaryCls is not ready, %s, %i", __FILE__, __LINE__);}
    if (PyModule_AddObject(m, "_binary", (PyObject *) &PyBinaryCls) < 0) {
        Py_DECREF(&PyBinaryCls);
        Py_DECREF(m);
        return 0;
    }

    if (PyType_Ready(&PyTernaryCls) < 0) {return PyErr_Format(PyExc_ImportError, "PyTernaryCls is not ready, %s, %i", __FILE__, __LINE__);}
    if (PyModule_AddObject(m, "_ternary", (PyObject *) &PyTernaryCls) < 0) {
        Py_DECREF(&PyTernaryCls);
        Py_DECREF(m);
        return 0;
    }

    if (PyType_Ready(&PyPNullaryCls) < 0) {return PyErr_Format(PyExc_ImportError, "PyPNullaryCls is not ready, %s, %i", __FILE__, __LINE__);}
    if (PyModule_AddObject(m, "_pnullary", (PyObject *) &PyPNullaryCls) < 0) {
        Py_DECREF(&PyPNullaryCls);
        Py_DECREF(m);
        return 0;
    }

    if (PyType_Ready(&PyPUnaryCls) < 0) {return PyErr_Format(PyExc_ImportError, "PyPUnaryCls is not ready, %s, %i", __FILE__, __LINE__);}
    if (PyModule_AddObject(m, "_punary", (PyObject *) &PyPUnaryCls) < 0) {
        Py_DECREF(&PyPUnaryCls);
        Py_DECREF(m);
        return 0;
    }

    if (PyType_Ready(&PyPBinaryCls) < 0) {return PyErr_Format(PyExc_ImportError, "PyPBinaryCls is not ready, %s, %i", __FILE__, __LINE__);}
    if (PyModule_AddObject(m, "_pbinary", (PyObject *) &PyPBinaryCls) < 0) {
        Py_DECREF(&PyPBinaryCls);
        Py_DECREF(m);
        return 0;
    }

    if (PyType_Ready(&PyPTernaryCls) < 0) {return PyErr_Format(PyExc_ImportError, "PyPTernaryCls is not ready, %s, %i", __FILE__, __LINE__);}
    if (PyModule_AddObject(m, "_pternary", (PyObject *) &PyPTernaryCls) < 0) {
        Py_DECREF(&PyPTernaryCls);
        Py_DECREF(m);
        return 0;
    }

    // JSelectionResult
    if (PyType_Ready(&PyJSelectionResultCls) < 0) {return PyErr_Format(PyExc_ImportError, "PyJSelectionResultCls is not ready, %s, %i", __FILE__, __LINE__);}
    if (PyModule_AddObject(m, "JSelectionResult", (PyObject *) &PyJSelectionResultCls) < 0) {
        Py_DECREF(&PyJSelectionResultCls);
        Py_DECREF(m);
        return 0;
    }

    // Fits
    if (PyType_Ready(&PyFitsCls) < 0) {return PyErr_Format(PyExc_ImportError, "PyFitsCls is not ready, %s, %i", __FILE__, __LINE__);}
    if (PyModule_AddObject(m, "Fits", (PyObject *) &PyFitsCls) < 0) {
        Py_DECREF(&PyFitsCls);
        Py_DECREF(m);
        return 0;
    }

    // JFunc, JOverload, JFamily
    if (PyType_Ready(&PyJFuncCls) < 0) {return PyErr_Format(PyExc_ImportError, "PyJFuncCls is not ready, %s, %i", __FILE__, __LINE__);}
    if (PyModule_AddObject(m, "JFunc", (PyObject *) &PyJFuncCls) < 0) {
        Py_DECREF(&PyJFuncCls);
        Py_DECREF(m);
        return 0;
    }

    if (PyType_Ready(&PyJOverloadCls) < 0) {return PyErr_Format(PyExc_ImportError, "PyJOverloadCls is not ready, %s, %i", __FILE__, __LINE__);}
    if (PyModule_AddObject(m, "JOverload", (PyObject *) &PyJOverloadCls) < 0) {
        Py_DECREF(&PyJOverloadCls);
        Py_DECREF(m);
        return 0;
    }

    if (PyType_Ready(&PyJFamilyCls) < 0) {return PyErr_Format(PyExc_ImportError, "PyJFamilyCls is not ready, %s, %i", __FILE__, __LINE__);}
    if (PyModule_AddObject(m, "JFamily", (PyObject *) &PyJFamilyCls) < 0) {
        Py_DECREF(&PyJFamilyCls);
        Py_DECREF(m);
        return 0;
    }

    //  SchemaError
    PySchemaError = PyErr_NewException("jones.SchemaError", PyBTypeError, NULL);
    if (!PySchemaError) return PyErr_Format(PyExc_ImportError, "Can't create SchemaError");
    if (PyModule_AddObject(m, "SchemaError", PySchemaError) < 0) {
        Py_DECREF(PySchemaError);
        Py_DECREF(m);
        return 0;
    }

    return m;
}

pvt void die_(char const *preamble, char const *msg, va_list args) {
    fprintf(stderr, "%s", preamble);
    vfprintf(stderr, msg, args);
    fprintf(stderr, "\n");
    exit(1);
}

#endif  // SRC_JONES_MOD_JONES_C