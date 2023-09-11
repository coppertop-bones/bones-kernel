#ifndef __JONES___JONES_C
#define __JONES___JONES_C "jones/__jones.c"


#include "../../include/all.cfg"
#include "pyfn.c"
#include "../bk/os.c"
#include "pybtype.c"
#include "pykernel.c"
#include "sc.c"
#include "fns.c"
#include "play.c"






// ---------------------------------------------------------------------------------------------------------------------
// init module
// ---------------------------------------------------------------------------------------------------------------------

pvt PyMethodDef free_fns[] = {
    {"toAddress", (PyCFunction)                 _toAddress, METH_FASTCALL, "toAddress(object)\n\nanswer the address of object and it's refcount"},
    {"toPtr", (PyCFunction)                     _toPtr, METH_FASTCALL, "toPtr(object)\n\nanswer the address of object"},
    {"toObj",                                   _toObj, METH_VARARGS, "toObj(address)\n\nreturn the ptr as an object"},
    {"ob_refcnt",                               _ob_refcnt, METH_VARARGS, "ob_refcnt(address)\n\nreturn the ref count for the object at the address"},
    {"atU16", (PyCFunction)                     _atU16, METH_FASTCALL, "atU16(pBuf, index"},
    {"atU16Put", (PyCFunction)                  _atU16Put, METH_FASTCALL, "atU16Put(pBuf, index, mask, value"},
    {"atU8", (PyCFunction)                      _atU8, METH_FASTCALL, "atU8(pBuf, index"},
    {"atU8Put", (PyCFunction)                   _atU8Put, METH_FASTCALL, "atU8Put(pBuf, index, mask, value"},
    {"malloc", (PyCFunction)                    _malloc, METH_FASTCALL, ""},
    {"getPageSize", (PyCFunction)               _pageSize, METH_FASTCALL, "system page size"},
    {"getCacheLineSize", (PyCFunction)          _getCacheLineSize, METH_FASTCALL, "system cache line size"},

    {"sc_new", (PyCFunction)                    _sc_new, METH_FASTCALL, "sc_new(numArgs, arrayLen) -> pSC"},
    {"sc_drop", (PyCFunction)                   _sc_drop, METH_FASTCALL, "sc_drop(pSC) -> None"},
    {"sc_nextFreeArrayIndex", (PyCFunction)     _sc_next_free_array_index, METH_FASTCALL, ""},
    {"sc_atArrayPut", (PyCFunction)             _sc_atArrayPut, METH_FASTCALL, "sc_atArrayPut(pSC, index, pSig, fnId) -> Void\n\nin pSig belonging to pSC, at index put fnId"},
    {"sc_queryPtr", (PyCFunction)               _sc_pQuery, METH_FASTCALL, "scQueryPtr(pSC)\n\nanswer a pointer to the query buffer"},
    {"sc_getFnId", (PyCFunction)                _sc_get_result, METH_FASTCALL, ""},
    {"sc_tArgsFromQuery", (PyCFunction)         _sc_tArgs_from_query, METH_FASTCALL, "sc_tArgsFromQuery(pSC : ptr, allTypes : pylist)\n\nanswers a tuple of tArgs from the slot"},
    {"sc_fillQuerySlotWithBTypesOf", (PyCFunction) _sc_fill_query_slot_with_btypes_of, METH_FASTCALL, "sc_fillQuerySlotWithBTypesOf(pSC : ptr, args : tuple)\n\nanswers a tuple of tArgs from the slot"},

    {"sc_test_arrayPtr", (PyCFunction)          _sc_test_pArray, METH_FASTCALL, "sc_test_arrayPtr(pSC)\n\nanswer a pointer to the array of sigs"},
    {"sc_test_slotWidth", (PyCFunction)         _sc_test_slot_width, METH_FASTCALL, "sc_test_slotWidth(pSC) -> count"},
    {"sc_test_numSlots", (PyCFunction)          _sc_test_num_slots, METH_FASTCALL, "sc_test_numSlots(pSC) -> count"},
    {"sc_test_fillQuerySlotAndGetFnId", (PyCFunction) _sc_test_fill_query_slot_and_get_result, METH_FASTCALL, "sc_test_fillQuerySlotAndGetFnId(pSC, tArgs : pytuple) -> fnId\n\nanswer the resultId for the signature tArgs"},

//    {"type_new",                                Shifter_type_new, METH_VARARGS | METH_KEYWORDS, ""},

    // play
    {"execShell",                               _execShell, METH_VARARGS, "Execute a shell command."},
    {"sizeofFredJoe", (PyCFunction)             _sizeofFredJoe, METH_FASTCALL, "tuple with sizeOf Fred and Joe in it"},
    {"reserve", (PyCFunction)                   _reserve, METH_FASTCALL, "reserve(pVA, numPages)"},
//    {"unreserve", (PyCFunction)                 _unreserve, METH_FASTCALL, ""},
    {"getVaPtr", (PyCFunction)                  _getVaPtr, METH_FASTCALL, "getVaPtr()"},

    {0, 0, 0, 0}
};


pvt PyModuleDef jones_module = {
    PyModuleDef_HEAD_INIT,
    .m_name = "jones",
    .m_doc = "as in archibald",
    .m_size = -1,
    free_fns
};


//    {"bmodule", T_OBJECT, offsetof(struct Fn, bmodule), READONLY, "bones module name"},

pyapi PyMODINIT_FUNC PyInit_jones(void) {
    PyObject *m;

// TODO make cross platform
//    unsigned int num_pages = _1GB / db_os_page_size();
//    VA *va = init_va(num_pages);
//    if (va == 0) return 0;
//    g_va = va;

    m = PyModule_Create(&jones_module);
    if (m == 0) return 0;

    // PyJonesError
    PyJonesError = PyErr_NewException("jones.JonesError", 0, 0);
    if (PyModule_AddObject(m, "error", PyJonesError) < 0) {
        Py_XDECREF(PyJonesError);
        Py_CLEAR(PyJonesError);
        Py_DECREF(m);
        return 0;
    }

    // PyJonesSyntaxError
    PyJonesSyntaxError = PyErr_NewException("jones.JonesSyntaxError", 0, 0);
    if (PyModule_AddObject(m, "error", PyJonesSyntaxError) < 0) {
        Py_XDECREF(PyJonesSyntaxError);
        Py_CLEAR(PyJonesSyntaxError);
        Py_DECREF(m);
        return 0;
    }

    // PyBTypeCls
    if (PyType_Ready(&PyBTypeCls) < 0) return 0;
    if (PyModule_AddObject(m, "BType", (PyObject *) &PyBTypeCls) < 0) {
        Py_DECREF(&PyBTypeCls);
        Py_DECREF(m);
        return 0;
    }

    // PyKernelCls
    if (PyType_Ready(&PyKernelCls) < 0) return 0;
    if (PyModule_AddObject(m, "Kernel", (PyObject *) &PyKernelCls) < 0) {
        Py_DECREF(&PyKernelCls);
        Py_DECREF(m);
        return 0;
    }


    // add function classes
    if (PyType_Ready(&FnCls) < 0) return 0;
    if (PyModule_AddObject(m, "_fn", (PyObject *) &FnCls) < 0) {
        Py_DECREF(&FnCls);
        Py_DECREF(m);
        return 0;
    }

    if (PyType_Ready(&PFnCls) < 0) return 0;
    if (PyModule_AddObject(m, "_pfn", (PyObject *) &PFnCls) < 0) {
        Py_DECREF(&PFnCls);
        Py_DECREF(m);
        return 0;
    }

    if (PyType_Ready(&PyNullaryCls) < 0) return 0;
    if (PyModule_AddObject(m, "_nullary", (PyObject *) &PyNullaryCls) < 0) {
        Py_DECREF(&PyNullaryCls);
        Py_DECREF(m);
        return 0;
    }

    if (PyType_Ready(&PyUnaryCls) < 0) return 0;
    if (PyModule_AddObject(m, "_unary", (PyObject *) &PyUnaryCls) < 0) {
        Py_DECREF(&PyUnaryCls);
        Py_DECREF(m);
        return 0;
    }

    if (PyType_Ready(&PyBinaryCls) < 0) return 0;
    if (PyModule_AddObject(m, "_binary", (PyObject *) &PyBinaryCls) < 0) {
        Py_DECREF(&PyBinaryCls);
        Py_DECREF(m);
        return 0;
    }

    if (PyType_Ready(&PyTernaryCls) < 0) return 0;
    if (PyModule_AddObject(m, "_ternary", (PyObject *) &PyTernaryCls) < 0) {
        Py_DECREF(&PyTernaryCls);
        Py_DECREF(m);
        return 0;
    }

    if (PyType_Ready(&PyPNullaryCls) < 0) return 0;
    if (PyModule_AddObject(m, "_pnullary", (PyObject *) &PyPNullaryCls) < 0) {
        Py_DECREF(&PyPNullaryCls);
        Py_DECREF(m);
        return 0;
    }

    if (PyType_Ready(&PyPUnaryCls) < 0) return 0;
    if (PyModule_AddObject(m, "_punary", (PyObject *) &PyPUnaryCls) < 0) {
        Py_DECREF(&PyPUnaryCls);
        Py_DECREF(m);
        return 0;
    }

    if (PyType_Ready(&PyPBinaryCls) < 0) return 0;
    if (PyModule_AddObject(m, "_pbinary", (PyObject *) &PyPBinaryCls) < 0) {
        Py_DECREF(&PyPBinaryCls);
        Py_DECREF(m);
        return 0;
    }

    if (PyType_Ready(&PyPTernaryCls) < 0) return 0;
    if (PyModule_AddObject(m, "_pternary", (PyObject *) &PyPTernaryCls) < 0) {
        Py_DECREF(&PyPTernaryCls);
        Py_DECREF(m);
        return 0;
    }


    // add PyToyCls
    if (PyType_Ready(&PyToyCls) < 0) return 0;
    if (PyModule_AddObject(m, "Toy", (PyObject *) &PyToyCls) < 0) {
        Py_DECREF(&PyToyCls);
        Py_DECREF(m);
        return 0;
    }

    return m;
}

pvt void die_(char *preamble, char *msg, va_list args) {
    fprintf(stderr, "%s", preamble);
    vfprintf(stderr, msg, args);
    exit(1);
}

#endif  // __JONES___JONES_C