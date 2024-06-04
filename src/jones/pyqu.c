// ---------------------------------------------------------------------------------------------------------------------
// quant utils
// ---------------------------------------------------------------------------------------------------------------------
#ifndef SRC_JONES_PYQU_C
#define SRC_JONES_PYQU_C "jones/pyqu.c"


#include "../qu/black.c"
#include "../qu/dists.c"
#include "jones.h"
#include "lib/pyutils.h"



// ---------------------------------------------------------------------------------------------------------------------
// b76_call: p:PyFloat -> PyFloat + PyException
// ---------------------------------------------------------------------------------------------------------------------
pvt PyObject * Py_qu_b76_call(PyTM *self, PyObject **args, Py_ssize_t nargs) {
    double t, r, df;
    // OPEN: assert tenor, strike, vol, r >= 0 and forward > 0
    if (nargs != 5) return jErrWrongNumberOfArgs(FN_NAME, 5, nargs);
    if (!PyFloat_Check(args[0])) return PyErr_Format(PyExc_TypeError, "tenor is not a float");
    if (!PyFloat_Check(args[1])) return PyErr_Format(PyExc_TypeError, "strike is not a float");
    if (!PyFloat_Check(args[2])) return PyErr_Format(PyExc_TypeError, "forward is not a float");
    if (!PyFloat_Check(args[3])) return PyErr_Format(PyExc_TypeError, "vol is not a float");
    if (!PyFloat_Check(args[4])) return PyErr_Format(PyExc_TypeError, "r is not a float");
    t = PyFloat_AsDouble(args[0]);
    r = PyFloat_AsDouble(args[4]);
    df = exp(- r * t);
    return PyFloat_FromDouble(
        qu_b76_call(
            PyFloat_AsDouble(args[0]),
            PyFloat_AsDouble(args[1]),
            PyFloat_AsDouble(args[2]),
            PyFloat_AsDouble(args[3]),
            df,
            qu_cn_Hart
        )
    );
}

// ---------------------------------------------------------------------------------------------------------------------
// b76_call_greeks: p:PyFloat -> PyFloat + PyException
// ---------------------------------------------------------------------------------------------------------------------
pvt PyObject * Py_qu_b76_call_greeks(PyTM *self, PyObject **args, Py_ssize_t nargs) {
    double t, r, df;  black_greeks greeks;  PyObject *answer;
    // OPEN: assert tenor, strike, vol, r >= 0 and forward > 0
    if (nargs != 5) return jErrWrongNumberOfArgs(FN_NAME, 5, nargs);
    if (!PyFloat_Check(args[0])) return PyErr_Format(PyExc_TypeError, "tenor is not a float");
    if (!PyFloat_Check(args[1])) return PyErr_Format(PyExc_TypeError, "strike is not a float");
    if (!PyFloat_Check(args[2])) return PyErr_Format(PyExc_TypeError, "forward is not a float");
    if (!PyFloat_Check(args[3])) return PyErr_Format(PyExc_TypeError, "vol is not a float");
    if (!PyFloat_Check(args[4])) return PyErr_Format(PyExc_TypeError, "r is not a float");
    t = PyFloat_AsDouble(args[0]);
    r = PyFloat_AsDouble(args[4]);
    df = exp(- r * t);
    greeks = qu_b76_call_greeks(
        PyFloat_AsDouble(args[0]),
        PyFloat_AsDouble(args[1]),
        PyFloat_AsDouble(args[2]),
        PyFloat_AsDouble(args[3]),
        r,
        df,
        qu_cn_Hart
    );
    answer = PyTuple_New(6);
    if (answer == 0) return 0;
    PyTuple_SET_ITEM(answer, 0, PyFloat_FromDouble(greeks.price));
    PyTuple_SET_ITEM(answer, 1, PyFloat_FromDouble(greeks.delta));
    PyTuple_SET_ITEM(answer, 2, PyFloat_FromDouble(greeks.gamma));
    PyTuple_SET_ITEM(answer, 3, PyFloat_FromDouble(greeks.vega));
    PyTuple_SET_ITEM(answer, 4, PyFloat_FromDouble(greeks.theta));
    PyTuple_SET_ITEM(answer, 5, PyFloat_FromDouble(greeks.rho));
    return answer;
}

// ---------------------------------------------------------------------------------------------------------------------
// b76_put: p:PyFloat -> PyFloat + PyException
// ---------------------------------------------------------------------------------------------------------------------
pvt PyObject * Py_qu_b76_put(PyTM *self, PyObject **args, Py_ssize_t nargs) {
    double t, r, df;
    // OPEN: assert tenor, strike, vol, r >= 0 and forward > 0
    if (nargs != 5) return jErrWrongNumberOfArgs(FN_NAME, 5, nargs);
    if (!PyFloat_Check(args[0])) return PyErr_Format(PyExc_TypeError, "tenor is not a float");
    if (!PyFloat_Check(args[1])) return PyErr_Format(PyExc_TypeError, "strike is not a float");
    if (!PyFloat_Check(args[2])) return PyErr_Format(PyExc_TypeError, "forward is not a float");
    if (!PyFloat_Check(args[3])) return PyErr_Format(PyExc_TypeError, "vol is not a float");
    if (!PyFloat_Check(args[4])) return PyErr_Format(PyExc_TypeError, "r is not a float");
    t = PyFloat_AsDouble(args[0]);
    r = PyFloat_AsDouble(args[4]);
    df = exp(- r * t);
    return PyFloat_FromDouble(
        qu_b76_put(
            PyFloat_AsDouble(args[0]),
            PyFloat_AsDouble(args[1]),
            PyFloat_AsDouble(args[2]),
            PyFloat_AsDouble(args[3]),
            df,
            qu_cn_Hart
        )
    );
}

// ---------------------------------------------------------------------------------------------------------------------
// b76_put_greeks: p:PyFloat -> PyFloat + PyException
// ---------------------------------------------------------------------------------------------------------------------
pvt PyObject * Py_qu_b76_put_greeks(PyTM *self, PyObject **args, Py_ssize_t nargs) {
    double t, r, df;  black_greeks greeks;  PyObject *answer;
    // OPEN: assert tenor, strike, vol, r >= 0 and forward > 0
    if (nargs != 5) return jErrWrongNumberOfArgs(FN_NAME, 5, nargs);
    if (!PyFloat_Check(args[0])) return PyErr_Format(PyExc_TypeError, "tenor is not a float");
    if (!PyFloat_Check(args[1])) return PyErr_Format(PyExc_TypeError, "strike is not a float");
    if (!PyFloat_Check(args[2])) return PyErr_Format(PyExc_TypeError, "forward is not a float");
    if (!PyFloat_Check(args[3])) return PyErr_Format(PyExc_TypeError, "vol is not a float");
    if (!PyFloat_Check(args[4])) return PyErr_Format(PyExc_TypeError, "r is not a float");
    t = PyFloat_AsDouble(args[0]);
    r = PyFloat_AsDouble(args[4]);
    df = exp(- r * t);
    greeks = qu_b76_put_greeks(
        PyFloat_AsDouble(args[0]),
        PyFloat_AsDouble(args[1]),
        PyFloat_AsDouble(args[2]),
        PyFloat_AsDouble(args[3]),
        r,
        df,
        qu_cn_Hart
    );
    answer = PyTuple_New(6);
    if (answer == 0) return 0;
    PyTuple_SET_ITEM(answer, 0, PyFloat_FromDouble(greeks.price));
    PyTuple_SET_ITEM(answer, 1, PyFloat_FromDouble(greeks.delta));
    PyTuple_SET_ITEM(answer, 2, PyFloat_FromDouble(greeks.gamma));
    PyTuple_SET_ITEM(answer, 3, PyFloat_FromDouble(greeks.vega));
    PyTuple_SET_ITEM(answer, 4, PyFloat_FromDouble(greeks.theta));
    PyTuple_SET_ITEM(answer, 5, PyFloat_FromDouble(greeks.rho));
    return answer;
}

// ---------------------------------------------------------------------------------------------------------------------
// invcn_Acklam: p:PyFloat -> PyFloat + PyException
// ---------------------------------------------------------------------------------------------------------------------
pvt PyObject * Py_qu_invcn_Acklam(PyTM *self, PyObject **args, Py_ssize_t nargs) {
    double p;
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyFloat_Check(args[0])) return PyErr_Format(PyExc_TypeError, "p is not a float");
    p = PyFloat_AsDouble(args[0]);
    return PyFloat_FromDouble(qu_invcn_Acklam(p));
}

// ---------------------------------------------------------------------------------------------------------------------
// cn_Hart: x:PyFloat -> PyFloat + PyException
// ---------------------------------------------------------------------------------------------------------------------
pvt PyObject * Py_qu_cn_Hart(PyTM *self, PyObject **args, Py_ssize_t nargs) {
    double x;
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    if (!PyFloat_Check(args[0])) return PyErr_Format(PyExc_TypeError, "x is not a float");
    x = PyFloat_AsDouble(args[0]);
    return PyFloat_FromDouble(qu_cn_Hart(x));
}





#endif      // SRC_JONES_PYQU_C
