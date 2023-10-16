#ifndef __JONES__UTILS_C
#define __JONES__UTILS_C "jones/_utils.c"


#include "../jones/_jones.h"


// PyExc_ValueError, PyExc_TypeError

#define PY_ASSERT_INT_WITHIN_CLOSED(variable, accessorDesc, lb, ub) {                                                   \
    if (!((lb) <= (variable) && (variable) <= (ub))) {                                                                  \
        char *s1, *s2, *s3;                                                                                             \
        asprintf (&s1, "%li", (long)(lb));                                                                              \
        asprintf (&s2, "%li", (long)(ub));                                                                              \
        asprintf (&s3, "%li", (long)(variable));                                                                        \
        char *msg = join_txts(12, __FUNCTION__, ": ", accessorDesc, " = ", s3, " but {", s1, " <= ", accessorDesc, " <= ", s2, "}"); \
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
            PyObject *answer = PyErr_Format(PyJonesError, (const char *) retval);                                       \
            free(retval);                                                                                               \
            return answer;                                                                                              \
        }                                                                                                               \
    }


pvt PyObject * jErrWrongNumberOfArgs(const char * fName, int numExpected, Py_ssize_t numGiven) {
    // https://pythonextensionpatterns.readthedocs.io/en/latest/exceptions.html
    // https://docs.python.org/3/library/stdtypes.html#old-string-formatting
    // https://docs.python.org/3/c-api/exceptions.html#c.PyErr_Format
    if (numExpected == 1)
        return PyErr_Format( PyExc_TypeError, "%s takes 1 positional argument but %i were given", fName, numGiven );
    else {
        if (numGiven == 1)
            return PyErr_Format( PyExc_TypeError, "%s takes %i positional arguments but 1 was given", fName, numExpected );
        else
            return PyErr_Format( PyExc_TypeError, "%s takes %i positional arguments but %i were given", fName, numExpected, numGiven );
    }
}



#define PP_INT(s, i) { printf("%s%#02x\n", s, (int) i) }
#define PP_PTR(s, i) { printf("%s%zu\n", s, (size_t) i) }
//    printf("%s%#02zu\n", s, (size_t) i)



#endif  // __JONES__UTILS_C