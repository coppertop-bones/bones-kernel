// ---------------------------------------------------------------------------------------------------------------------
// Copyright 2025 David Briant, https://github.com/coppertop-bones. Licensed under the Apache License, Version 2.0
// ---------------------------------------------------------------------------------------------------------------------

#ifndef SRC_JONES_LIB_PYUTILS_C
#define SRC_JONES_LIB_PYUTILS_C "jones/_utils.c"


#include "../jones.h"


// PyExc_ValueError, PyExc_TypeError

#define PY_ASSERT_INT_WITHIN_CLOSED(variable, accessorDesc, lb, ub) {                                                   \
    if (!((lb) <= (variable) && (variable) <= (ub))) {                                                                  \
        char *s1, *s2, *s3;                                                                                             \
        asprintf (&s1, "%li", (long)(lb));                                                                              \
        asprintf (&s2, "%li", (long)(ub));                                                                              \
        asprintf (&s3, "%li", (long)(variable));                                                                        \
        char *msg = join_txts(12, FN_NAME, ": ", accessorDesc, " = ", s3, " but {", s1, " <= ", accessorDesc, " <= ", s2, "}"); \
        PyObject *answer =  PyErr_Format(PyJonesError, msg);                                                            \
        free(s1);                                                                                                       \
        free(s2);                                                                                                       \
        free(s3);                                                                                                       \
        free(msg);                                                                                                      \
        return answer;                                                                                                  \
    }                                                                                                                   \
}


#define TRAP_PY(src) {                                                                                                  \
        char *retval = (src);                                                                                           \
        if (retval != 0) {                                                                                              \
            PyObject *answer = PyErr_Format(PyJonesError, (char *) retval);                                             \
            free(retval);                                                                                               \
            return answer;                                                                                              \
        }                                                                                                               \
    }


pvt PyObject * jErrWrongNumberOfArgs(char * fName, int numExpected, Py_ssize_t numGiven) {
    // https://pythonextensionpatterns.readthedocs.io/en/latest/exceptions.html
    // https://docs.python.org/3/library/stdtypes.html#old-string-formatting
    // https://docs.python.org/3/c-api/exceptions.html#c.PyErr_Format
    if (numExpected == 1)
        return PyErr_Format(PyExc_TypeError, "%s takes 1 positional argument but %i were given", fName, numGiven);
    else {
        if (numGiven == 1)
            return PyErr_Format(PyExc_TypeError, "%s takes %i positional arguments but 1 was given", fName, numExpected);
        else
            return PyErr_Format(PyExc_TypeError, "%s takes %i positional arguments but %i were given", fName, numExpected, numGiven);
    }
}


#define __TO_DOUBLE_OR_ERR(VARNAME, ARG, ...)                                                                           \
if (PyFloat_Check(ARG)) {VARNAME = PyFloat_AsDouble(ARG);}                                                              \
else if (PyLong_Check(ARG)) {VARNAME = (double) PyLong_AS_LONG(ARG);}                                                   \
else return PyErr_Format(PyExc_TypeError, __VA_ARGS__);

#define __TO_INT_OR_ERR(VARNAME, ARG, ...)                                                                              \
if (PyLong_Check(ARG)) {VARNAME = (int) PyLong_AS_LONG(ARG);}                                                           \
else return PyErr_Format(PyExc_TypeError, __VA_ARGS__);

#define __CHECK(EXPR, CLASS, ...)                                                                                       \
if (!(EXPR)) return PyErr_Format(CLASS, __VA_ARGS__);

#define __CHECK_NOT(EXPR, CLASS, ...)                                                                                   \
if (EXPR) return PyErr_Format(CLASS, __VA_ARGS__);

#define __GET_KWARG(KWARGNAME, ARG, SETTER) if (strcmp(KWARGNAME, PyUnicode_AsUTF8(ARG)) == 0) { SETTER; }
#define __OR_GET_KWARG(KWARGNAME, ARG, SETTER) else if (strcmp(KWARGNAME, PyUnicode_AsUTF8(ARG)) == 0) { SETTER; }
#define __ELSE_RAISE(CLASS, ...) else {return PyErr_Format(CLASS, __VA_ARGS__);}


#define checkPyLong(py, msg) {if (!PyLong_Check(py)) return PyErr_Format(PyExc_TypeError, msg);}
#define checkPyFloat(py, msg) {if (!PyFloat_Check(py)) return PyErr_Format(PyExc_TypeError, msg);}
#define checkPyUtf8(py, msg) {if (!PyUnicode_Check(py) || (PyUnicode_KIND(py) != PyUnicode_1BYTE_KIND)) return PyErr_Format(PyExc_TypeError, msg);}


#define PP_INT(s, i) { printf("%s%#02x\n", s, (int) i) }
#define PP_PTR(s, i) { printf("%s%zu\n", s, (size_t) i) }
//    printf("%s%#02zu\n", s, (size_t) i)



#endif  // SRC_JONES_LIB_PYUTILS_C