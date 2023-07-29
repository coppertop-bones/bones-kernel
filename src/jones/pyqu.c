// ---------------------------------------------------------------------------------------------------------------------
// Copyright 2025 David Briant, https://github.com/coppertop-bones. Licensed under the Apache License, Version 2.0
//
// PYQU - PYTHON INTERFACE TO QUANT UTILS
// ---------------------------------------------------------------------------------------------------------------------

#ifndef SRC_JONES_PYQU_C
#define SRC_JONES_PYQU_C "jones/pyqu.c"


#include "qu.h"
#include "lib/pyutils.h"
#include "../qu/black.c"
#include "../qu/dists.c"
#include "../qu/mt.c"



// ---------------------------------------------------------------------------------------------------------------------
// b76_call: p:PyFloat -> PyFloat + PyException
// ---------------------------------------------------------------------------------------------------------------------
pvt PyObject * Py_qu_b76_call(PyTM *self, PyObject **args, Py_ssize_t nargs) {
    double t, k, f, v, r, df;
    // OPEN: assert tenor, strike, vol, r >= 0 and forward > 0
    if (nargs != 5) return jErrWrongNumberOfArgs(FN_NAME, 5, nargs);
    __TO_DOUBLE_OR_ERR(t, args[0], "tenor must be a float or int");
    __TO_DOUBLE_OR_ERR(k, args[1], "strike must be a float or int");
    __TO_DOUBLE_OR_ERR(f, args[2], "forward must be a float or int");
    __TO_DOUBLE_OR_ERR(v, args[3], "vol must be a float or int");
    __TO_DOUBLE_OR_ERR(r, args[4], "r must be a float or int");
    df = exp(- r * t);
    return PyFloat_FromDouble(qu_b76_call(t, k, f, v, df, qu_cn_Hart));
}

// ---------------------------------------------------------------------------------------------------------------------
// b76_call_greeks: p:PyFloat -> PyFloat + PyException
// ---------------------------------------------------------------------------------------------------------------------
pvt PyObject * Py_qu_b76_call_greeks(PyTM *self, PyObject **args, Py_ssize_t nargs) {
    double t, k, f, v, r, df;  black_greeks greeks;  PyObject *answer;
    // OPEN: assert tenor, strike, vol, r >= 0 and forward > 0
    if (nargs != 5) return jErrWrongNumberOfArgs(FN_NAME, 5, nargs);
    __TO_DOUBLE_OR_ERR(t, args[0], "tenor must be a float or int");
    __TO_DOUBLE_OR_ERR(k, args[1], "strike must be a float or int");
    __TO_DOUBLE_OR_ERR(f, args[2], "forward must be a float or int");
    __TO_DOUBLE_OR_ERR(v, args[3], "vol must be a float or int");
    __TO_DOUBLE_OR_ERR(r, args[4], "r must be a float or int");
    df = exp(- r * t);
    greeks = qu_b76_call_greeks(t, k, f, v, r, df, qu_cn_Hart);
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
    double t, k, f, v, r, df;
    // OPEN: assert tenor, strike, vol, r >= 0 and forward > 0
    if (nargs != 5) return jErrWrongNumberOfArgs(FN_NAME, 5, nargs);
    __TO_DOUBLE_OR_ERR(t, args[0], "tenor must be a float or int");
    __TO_DOUBLE_OR_ERR(k, args[1], "strike must be a float or int");
    __TO_DOUBLE_OR_ERR(f, args[2], "forward must be a float or int");
    __TO_DOUBLE_OR_ERR(v, args[3], "vol must be a float or int");
    __TO_DOUBLE_OR_ERR(r, args[4], "r must be a float or int");
    df = exp(- r * t);
    return PyFloat_FromDouble(qu_b76_put(t, k, f, v, df, qu_cn_Hart));
}

// ---------------------------------------------------------------------------------------------------------------------
// b76_put_greeks
//      (p:PyFloat) -> PyFloat + PyException
// ---------------------------------------------------------------------------------------------------------------------
pvt PyObject * Py_qu_b76_put_greeks(PyTM *self, PyObject **args, Py_ssize_t nargs) {
    double t, k, f, v, r, df;  black_greeks greeks;  PyObject *answer;
    // OPEN: assert tenor, strike, vol, r >= 0 and forward > 0
    if (nargs != 5) return jErrWrongNumberOfArgs(FN_NAME, 5, nargs);
    __TO_DOUBLE_OR_ERR(t, args[0], "tenor must be a float or int");
    __TO_DOUBLE_OR_ERR(k, args[1], "strike must be a float or int");
    __TO_DOUBLE_OR_ERR(f, args[2], "forward must be a float or int");
    __TO_DOUBLE_OR_ERR(v, args[3], "vol must be a float or int");
    __TO_DOUBLE_OR_ERR(r, args[4], "r must be a float or int");
    r = PyFloat_AsDouble(args[4]);
    df = exp(- r * t);
    greeks = qu_b76_put_greeks(t, k, f, v, r, df, qu_cn_Hart);
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
// bachelier_call: p:PyFloat -> PyFloat + PyException
// ---------------------------------------------------------------------------------------------------------------------
pvt PyObject * Py_qu_bachelier_call(PyTM *self, PyObject **args, Py_ssize_t nargs) {
    double t, k, f, v, r, df;
    // OPEN: assert tenor, strike, vol, r >= 0 and forward > 0
    if (nargs != 5) return jErrWrongNumberOfArgs(FN_NAME, 5, nargs);
    __TO_DOUBLE_OR_ERR(t, args[0], "tenor must be a float or int");
    __TO_DOUBLE_OR_ERR(k, args[1], "strike must be a float or int");
    __TO_DOUBLE_OR_ERR(f, args[2], "forward must be a float or int");
    __TO_DOUBLE_OR_ERR(v, args[3], "vol must be a float or int");
    __TO_DOUBLE_OR_ERR(r, args[4], "r must be a float or int");
    df = exp(- r * t);
    return PyFloat_FromDouble(qu_bachelier_call(t, k, f, v, df, qu_cn_Hart, qu_norm_pdf));
}

// ---------------------------------------------------------------------------------------------------------------------
// bachelier_call: p:PyFloat -> PyFloat + PyException
// ---------------------------------------------------------------------------------------------------------------------
pvt PyObject * Py_qu_bachelier_put(PyTM *self, PyObject **args, Py_ssize_t nargs) {
    double t, k, f, v, r, df;
    // OPEN: assert tenor, strike, vol, r >= 0 and forward > 0
    if (nargs != 5) return jErrWrongNumberOfArgs(FN_NAME, 5, nargs);
    __TO_DOUBLE_OR_ERR(t, args[0], "tenor must be a float or int");
    __TO_DOUBLE_OR_ERR(k, args[1], "strike must be a float or int");
    __TO_DOUBLE_OR_ERR(f, args[2], "forward must be a float or int");
    __TO_DOUBLE_OR_ERR(v, args[3], "vol must be a float or int");
    __TO_DOUBLE_OR_ERR(r, args[4], "r must be a float or int");
    df = exp(- r * t);
    return PyFloat_FromDouble(qu_bachelier_put(t, k, f, v, df, qu_cn_Hart, qu_norm_pdf));
}

// ---------------------------------------------------------------------------------------------------------------------
// invcn_Acklam
//      (p:PyFloat) -> PyFloat + PyException
// ---------------------------------------------------------------------------------------------------------------------
pvt PyObject * Py_qu_invcn_Acklam(PyTM *self, PyObject **args, Py_ssize_t nargs) {
    double p;
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    __TO_DOUBLE_OR_ERR(p, args[0], "p must be a float or int");
    return PyFloat_FromDouble(qu_invcn_Acklam(p));
}

// ---------------------------------------------------------------------------------------------------------------------
// cn_Hart
//      (x:PyFloat) -> PyFloat + PyException
// ---------------------------------------------------------------------------------------------------------------------
pvt PyObject * Py_qu_cn_Hart(PyTM *self, PyObject **args, Py_ssize_t nargs) {
    double x;
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    __TO_DOUBLE_OR_ERR(x, args[0], "p must be a float or int");
    return PyFloat_FromDouble(qu_cn_Hart(x));
}

// ---------------------------------------------------------------------------------------------------------------------
// invcn_h
//      (p:PyFloat) -> PyFloat + PyException
// ---------------------------------------------------------------------------------------------------------------------
pvt PyObject * Py_qu_invcn_h(PyTM *self, PyObject **args, Py_ssize_t nargs) {
    double p;
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    __TO_DOUBLE_OR_ERR(p, args[0], "p must be a float or int");
    return PyFloat_FromDouble(qu_invcn_h(p));
}

// ---------------------------------------------------------------------------------------------------------------------
// cn_h
//      (x:PyFloat) -> PyFloat + PyException
// ---------------------------------------------------------------------------------------------------------------------
pvt PyObject * Py_qu_cn_h(PyTM *self, PyObject **args, Py_ssize_t nargs) {
    double x;
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    __TO_DOUBLE_OR_ERR(x, args[0], "x must be a float or int");
    return PyFloat_FromDouble(qu_cn_h(x));
}

// ---------------------------------------------------------------------------------------------------------------------
// ndarray filling utils
// ---------------------------------------------------------------------------------------------------------------------

// numpy snippets
// npy_intp strideN, strideM;
// strideN = PyArray_STRIDE(mat, 0);
// strideM = PyArray_STRIDE(mat, 1);
// npy_intp size = PyArray_SIZE(array);
// *(double *) (PyArray_GETPTR1(mat, i)) =

//-Wunused-but-set-variable

// https://stackoverflow.com/questions/47615345/passing-macro-arguments-to-macro-function
// Macro arguments are not expanded when the macro call is parsed. After the macro call is parsed, each use of a
// macro parameter in the macro definition text is replaced with the macro-expanded argument, except for macro
// parameters used with the # or ## operations (stringify and token paste), which are replaced with the unexpanded
// text of the macro argument. Then the # and ## operations are performed, and then the entire macro body is scanned
// one more time.

// ---------------------------------------------------------------------------------------------------------------------
// ndarray filling macros
//      DATA is a N*M matrix and must be laid out must be laid out in col major (fortran) style
// ---------------------------------------------------------------------------------------------------------------------

#define __FILL_ARRAY(DATA, TYPE, I, J, I1, I2, J1, J2, N, P, BLOCK)                                                     \
/* DATA is a N*M matrix and must be laid out must be laid out in col major (fortran) style */                           \
{                                                                                                                       \
    int (I), (J);  TYPE *(P), *_t, __attribute__((unused)) *_data;                                                      \
    _data = DATA;                                                                                                       \
    for ((J) = (J1); (J) <= (J2); (J)++) {                                                                              \
        _t = (TYPE *) (DATA) + (J) * (N);                                                                               \
        for ((I) = (I1); (I) <= (I2); (I)++) {                                                                          \
            (P) = _t + (I);                                                                                             \
            BLOCK                                                                                                       \
        }                                                                                                               \
    }                                                                                                                   \
}

#pragma push_macro("__P")
#define __P(I, J, N) (_data + (J) * (N) + (I))


// ---------------------------------------------------------------------------------------------------------------------
// new_mersennes_f64:
//      (n:PyLong) -> ndarray + PyException
//      (n:PyLong, m:PyLong) -> ndarray + PyException
// ---------------------------------------------------------------------------------------------------------------------
pvt PyObject * Py_qu_new_mersennes_f64(PyObject *self, PyObject **args, Py_ssize_t nargs) {
    int n, m;  PyObject *mat;
    if (nargs == 1) {
        __TO_INT_OR_ERR(n, args[0], "n must be an int");
        npy_intp dims[1] = {n};
        if ((mat=PyArray_SimpleNew(1, dims, NPY_FLOAT64)) == 0) PyErr_Format(PyExc_RuntimeError, "could not create np.ndarray");
        __FILL_ARRAY(PyArray_DATA(mat), double, i, j, 0, n-1, 0, 0, n, p, {
            *p = qu_mt_f64_oo();
        })
        return mat;
    } else if (nargs == 2) {
        __TO_INT_OR_ERR(n, args[0], "n must be an int");
        __TO_INT_OR_ERR(m, args[1], "m must be an int");
        npy_intp dims[2] = {n, m};
        if ((mat=PyArray_New(&PyArray_Type, 2, dims, NPY_FLOAT64, NULL, NULL, 0, NPY_ARRAY_F_CONTIGUOUS, NULL)) == 0) PyErr_Format(PyExc_RuntimeError, "could not create np.ndarray");
        __FILL_ARRAY(PyArray_DATA(mat), double, i, j, 0, n-1, 0, m-1, n, p, {
            *p = qu_mt_f64_oo();
        })
        return mat;
    } else {
        return PyErr_Format(PyExc_TypeError, "new_mersennes_f64 takes 1 or 2 arg but %i were given", nargs);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
// new_mersennes_norm:
//      (n:PyLong) -> ndarray + PyException
//      (n:PyLong, m:PyLong) -> ndarray + PyException
// ---------------------------------------------------------------------------------------------------------------------
pvt PyObject * Py_qu_new_mersennes_norm(PyObject *self, PyObject **args, Py_ssize_t nargs) {
    PyArrayObject *mat;  int n, m;
    if (nargs == 1) {
        __TO_INT_OR_ERR(n, args[0], "n must be an int");
        npy_intp dims[1] = {n};
        if ((mat=(PyArrayObject *)PyArray_SimpleNew(1, dims, NPY_FLOAT64)) == 0) PyErr_Format(PyExc_RuntimeError, "could not create np.ndarray");

        __FILL_ARRAY(PyArray_DATA((PyArrayObject *) mat), double, i, j, 0, n-1, 0, 0, n, p, {
            *p = qu_invcn_Acklam(qu_mt_f64_oo());
        })
        return (PyObject *) mat;
    } else if (nargs == 2) {
        __TO_INT_OR_ERR(n, args[0], "n must be an int");
        __TO_INT_OR_ERR(m, args[1], "m must be an int");
        npy_intp dims[2] = {n, m};
        if ((mat=(PyArrayObject *)PyArray_New(&PyArray_Type, 2, dims, NPY_FLOAT64, NULL, NULL, 0, NPY_ARRAY_F_CONTIGUOUS, NULL)) == 0) PyErr_Format(PyExc_RuntimeError, "could not create np.ndarray");
        __FILL_ARRAY(PyArray_DATA((PyArrayObject *) mat), double, i, j, 0, n-1, 0, m-1, n, p, {
            *p = qu_invcn_Acklam(qu_mt_f64_oo());
        })
        return (PyObject *) mat;
    } else {
        return PyErr_Format(PyExc_TypeError, "new_mersennes_norm takes 1 or 2 arg but %i were given", nargs);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
// fill_mersennes_norm:
//      (matrix: ndarray, i1:PyLong, i2:PyLong, j1:PyLong, j2:PyLong) -> None + PyException
// ---------------------------------------------------------------------------------------------------------------------
pvt PyObject * Py_qu_fill_mersennes_norm(PyObject *self, PyObject **args, Py_ssize_t nargs) {
    PyArrayObject *mat;  int n, m, i1, i2, j1, j2, ndim;  npy_intp *shape;  double *data;
    if (nargs != 5) return PyErr_Format(PyExc_TypeError, "fill_mersennes_norm(matrix, i1, i2, j1, j2) takes 5 args but %i were given", nargs);
    __CHECK(PyArray_Check(args[0]), PyExc_TypeError, "matrix must be a numpy array");
    if (!PyLong_Check(args[1]) || !PyLong_Check(args[2]) || !PyLong_Check(args[3]) || !PyLong_Check(args[4])) return PyErr_Format(PyExc_TypeError, "i1, i2, j1 & j2 must be an int");
    mat = (PyArrayObject *) args[0];
    __CHECK(PyArray_TYPE(mat) == NPY_FLOAT64, PyExc_TypeError, "matrix must have dtype np.float64");
    __CHECK(PyArray_FLAGS(mat) & NPY_ARRAY_F_CONTIGUOUS, PyExc_TypeError, "matrix must be in fortran (col major) form");
    i1 = (int) PyLong_AsLong(args[1]);
    i2 = (int) PyLong_AsLong(args[2]);
    j1 = (int) PyLong_AsLong(args[3]);
    j2 = (int) PyLong_AsLong(args[4]);
    ndim = PyArray_NDIM(mat);
    __CHECK(ndim == 1 || ndim == 2, PyExc_TypeError, "matrix must have 1 or 2 dimensions but has %i", ndim);
    shape = PyArray_DIMS(mat);
    n = shape[0];
    m = ndim == 2 ? shape[1] : 0;
    __CHECK(0 <= i1 && i1 <= i2 && i2 < n, PyExc_ValueError, "i1 and i2 must be in range 0 <= i1 <= i2 < n");
    __CHECK(0 <= j1 && j1 <= j2 && j2 < m, PyExc_ValueError, "i1 and i2 must be in range 0 <= j1 <= j2 < m");

    data = PyArray_DATA((PyArrayObject *) mat);
    __FILL_ARRAY(data, double, i, j, i1, i2, j1, j2, n, p, {
        *p = qu_invcn_Acklam(qu_mt_f64_oo());
    })
    return Py_NewRef(mat);
}

// ---------------------------------------------------------------------------------------------------------------------
// fill_matrix:
//      (matrix: ndarray, op:PyString, **kwargs) -> ndarray + PyException
//      matrix is N x M
//      kwargs:
//          j - col to update
//          jW - col with normal RVs, defaults to j
//          j1, j2 - cols to update, jW not allowed
//      op:
//          "normal" process - mu, sigma, dt - defaults to mu=0, sigma=1, dt=1
//          "log" normal process - mu, sigma, dt - defaults to mu=ito drift, sigma=1, dt=1
//      OPEN:
//          add SABR, SLNSAR, NSAR, discrete versions, logspace, moment matching, etc
// ---------------------------------------------------------------------------------------------------------------------
#define IARG_MAT 0
#define IARG_OP 1
pvt PyObject * Py_qu_fill_matrix(PyObject *self, PyObject *const *args, Py_ssize_t nargs, PyObject *argnames) {
    // OPEN: "SABR", {"wa":0, "wf":1, "a":0.10, "nu":0.40, ...}) , "sobol", "mersenne_norm", "SLN", "N", "SLNSAR", STEPS:, DT:, etc
    PyArrayObject *pyMat;  int N, M, jj, jj1=0, jj2=0, ndim;  npy_intp *shape;  double *pmat;  char *op;  PyObject *pyJ=0, *pyJ1=0, *pyJ2=0;
    __CHECK(nargs == 2, PyExc_TypeError, "fill_matrix(matrix, op, **kwargs) only takes 2 args but %i were given", nargs);

    // matrix
    __CHECK(PyArray_Check(args[IARG_MAT]), PyExc_TypeError, "matrix must be a numpy array");
    pyMat = (PyArrayObject *) args[IARG_MAT];
    __CHECK(PyArray_TYPE(pyMat) == NPY_FLOAT64, PyExc_TypeError, "matrix must have dtype np.float64");
    __CHECK(PyArray_FLAGS(pyMat) & NPY_ARRAY_F_CONTIGUOUS, PyExc_TypeError, "matrix must be in fortran (col major) form");
    ndim = PyArray_NDIM(pyMat);
    __CHECK(ndim == 2, PyExc_TypeError, "matrix must have 2 dimensions but has %i", ndim);
    shape = PyArray_DIMS(pyMat);
    N = shape[0];
    M = shape[1];
    pmat = PyArray_DATA((PyArrayObject *) pyMat);

    // op
    __CHECK(PyUnicode_Check(args[IARG_OP]) && PyUnicode_KIND(args[IARG_OP]) == PyUnicode_1BYTE_KIND, PyExc_TypeError, "op must be utf8");
    op = (char *) PyUnicode_AsUTF8(args[IARG_OP]);

    // j, j1, j2
    for (int i = 0; i < PyTuple_Size(argnames); i++) {
        __GET_KWARG("j", PyTuple_GET_ITEM(argnames, i), pyJ = args[i + nargs])
        __OR_GET_KWARG("j1", PyTuple_GET_ITEM(argnames, i), pyJ1 = args[i + nargs])
        __OR_GET_KWARG("j2", PyTuple_GET_ITEM(argnames, i), pyJ2 = args[i + nargs])
    }
    __CHECK_NOT(pyJ && (pyJ1 || pyJ2), PyExc_SyntaxError, "Either specify j or both j1 and j2");
    __CHECK((pyJ1 && pyJ2) || (!pyJ1 && !pyJ2), PyExc_SyntaxError, "Must specify both j1 and j2");
    if (pyJ) {
        __TO_INT_OR_ERR(jj, pyJ, "j must be an int");
        __CHECK(0 <= jj && jj < M, PyExc_ValueError, "j must be in range 0 <= j < M");
    }
    if (pyJ1) {
        __TO_INT_OR_ERR(jj1, pyJ1, "j1 must be an int");
        __CHECK(0 <= jj1 && jj1 < M, PyExc_ValueError, "j1 must be in range 0 <= j1 < M");
    }
    if (pyJ2) {
        __TO_INT_OR_ERR(jj2, pyJ2, "j2 must be an int");
        __CHECK(0 <= jj2 && jj2 < M, PyExc_ValueError, "j2 must be in range 0 <= j2 < M");
    }
    __CHECK(jj1 <= jj2, PyExc_ValueError, "j1 must be in range j1 <= j2");

    // op == "ran"
    if (strcmp(op, "ran") == 0) {
        // options: i1, i2, j1, j2, dist="norm", anti=1 => AA', 2 => AB, A'B, AB', A'B'

    }

    // op == "shuffleTake"
    if (strcmp(op, "shuffleTake") == 0) {
        // options: i1, i2, j1, j2, src

    }

    // op == "cho2"
    if (strcmp(op, "cho2") == 0) {
        // options: i1, i2, j1, j2, rho
        PyObject *pyI1=0, *pyI2=0, *pyRho=0;  double rho;  int i1, i2=0;
        if (!pyJ1 || !pyJ2) return PyErr_Format(PyExc_SyntaxError, "If cho2 is specified then j1 and j2 must also be specified");
        for (int i = 0; i < PyTuple_Size(argnames); i++) {
            __GET_KWARG("i1", PyTuple_GET_ITEM(argnames, i), pyI1 = args[i + nargs])
            __OR_GET_KWARG("i2", PyTuple_GET_ITEM(argnames, i), pyI2 = args[i + nargs])
            __OR_GET_KWARG("rho", PyTuple_GET_ITEM(argnames, i), pyRho = args[i + nargs])
        }
        if (pyI1) {
            __TO_INT_OR_ERR(i1, pyI1, "i1 must be an int");
            if (i1 < 0 || i1 >= N || i1 > i2) return PyErr_Format(PyExc_ValueError, "i1 must be in range 0 <= i1 <= i2 < N");
        }
        if (pyI2) {
            __TO_INT_OR_ERR(i2, pyI2, "i2 must be an int");
            if (i2 < 0 || i2 >= N || i1 > i2) return PyErr_Format(PyExc_ValueError, "i2 must be in range 0 <= i1 <= i2 < N");
        }
        __CHECK(pyRho, PyExc_SyntaxError, "If cho2 is specified then rho also be specified");
        __TO_DOUBLE_OR_ERR(rho, pyRho, "rho must be an double");
        __CHECK(-1.0 <= rho && rho <= 1.0, PyExc_ValueError, "rho must be in range -1 <= rho <= 1");

        // rhoBar = sqrt(1 - rho * rho);
        // OPEN: transform independent vars
        return PyErr_Format(PyExc_ValueError, "Not Yet Implemented");
//        return Py_NewRef(pyMat);
    }

    // op == "eigen"


    // op == "norm"
    if (strcmp(op, "norm") == 0) {
        // options: jW - the col containing the N(0,1), dt, sigma, mu
        PyObject *pyJW=0, *pyDt=0, *pySigma=0, *pyMu=0;  int jW=jj;  double dt=1, sigma=1, mu=0, sigmaRootDt, muDt;
        for (int i = 0; i < PyTuple_Size(argnames); i++) {
            __GET_KWARG("jW", PyTuple_GET_ITEM(argnames, i), pyJW = args[i + nargs])
            __OR_GET_KWARG("dt", PyTuple_GET_ITEM(argnames, i), pyDt = args[i + nargs])
            __OR_GET_KWARG("sigma", PyTuple_GET_ITEM(argnames, i), pySigma = args[i + nargs])
            __OR_GET_KWARG("mu", PyTuple_GET_ITEM(argnames, i), pyMu = args[i + nargs])
        }
        if (pyJW) {
            if (pyJ1) return PyErr_Format(PyExc_SyntaxError, "Cannot specify jW if j1 and j2 are specified");
            __TO_INT_OR_ERR(jW, pyJW, "jW must be an int");
            __CHECK(0 <= jW && jW < M, PyExc_ValueError, "jW must be in range 0 <= jW < M");
        }
        if (pyDt) {
            __TO_DOUBLE_OR_ERR(dt, pyDt, "dt must be an double");
            __CHECK(0 <= dt, PyExc_ValueError, "dt must be in range 0 < dt < +inf");
        }
        if (pySigma) {
            __TO_DOUBLE_OR_ERR(sigma, pySigma, "sigma must be an double");
            __CHECK(0.0 < sigma && sigma <= 2, PyExc_ValueError, "sigma must be in range 0% < sigma <= 200%");
        }
        if (pyMu) {
            __TO_DOUBLE_OR_ERR(mu, pyMu, "mu must be an double");
        }
        sigmaRootDt = sigma * sqrt(dt);
        muDt = mu * dt;

        if (pyJ) {jj1 = jj; jj2 = jj;}
        if (pyJW) {
            __FILL_ARRAY(pmat, double, i, j, 1, N - 1, jj1, jj2, N, p, {
                double w;
                w = *__P(i, jW, N);
                *p = *__P(i - 1, j, N) + muDt + sigmaRootDt * w;
            })
        } else {
            __FILL_ARRAY(pmat, double, i, j, 1, N-1, jj1, jj2, N, p, {
                double w;
                w = *__P(i, j, N);
                *p = *__P(i-1, j, N) + muDt + sigmaRootDt * w;
            })
        }
        return Py_NewRef(pyMat);
    }

    // op == "log"
    else if (strcmp(op, "log") == 0) {
        // options: jW - the col containing the N(0,1), dt, sigma, mu
        PyObject *pyJW=0, *pyDt=0, *pySigma=0, *pyMu=0;  int jW=jj;  double dt=1, sigma=1, mu=0, sigmaRootDt, muDt, itoDrift;
        for (int i = 0; i < PyTuple_Size(argnames); i++) {
            __GET_KWARG("jW", PyTuple_GET_ITEM(argnames, i), pyJW = args[i + nargs])
            __OR_GET_KWARG("dt", PyTuple_GET_ITEM(argnames, i), pyDt = args[i + nargs])
            __OR_GET_KWARG("sigma", PyTuple_GET_ITEM(argnames, i), pySigma = args[i + nargs])
            __OR_GET_KWARG("mu", PyTuple_GET_ITEM(argnames, i), pyMu = args[i + nargs])
        }
        if (pyJW) {
            __TO_INT_OR_ERR(jW, pyJW, "jW must be an int");
            if (jW < 0 || jW >= M) return PyErr_Format(PyExc_ValueError, "jW must be in range 0 <= jW < M");
        }
        if (pyDt) {
            __TO_DOUBLE_OR_ERR(dt, pyDt, "dt must be an double");
            if (dt < 0) return PyErr_Format(PyExc_ValueError, "dt must be in range 0 < dt < +inf");
        }
        if (pySigma) {
            __TO_DOUBLE_OR_ERR(sigma, pySigma, "sigma must be an double");
            if (sigma <= 0 || sigma > 2) return PyErr_Format(PyExc_ValueError, "sigma must be in range 0% <= sigma <= 200%");
        }
        if (pyMu) {
            __TO_DOUBLE_OR_ERR(mu, pyMu, "mu must be an double");
        }
        sigmaRootDt = sigma * sqrt(dt);
        muDt = mu * dt;
        itoDrift = - 0.5 * sigma * sigma * dt;

        if (pyJ) {jj1 = jj; jj2 = jj;}
        __FILL_ARRAY(pmat, double, i, j, 0, 0, jj1, jj2, N, p, {
            double first;
            first = *__P(i, j, N);
            __CHECK(0 < first, PyExc_ValueError, "matrix[0,%i] must be in range 0 < x < +inf", j);
            *__P(0, j, N) = log(first);
        })
        if (pyJW) {
            __FILL_ARRAY(pmat, double, i, j, 1, N - 1, jj1, jj2, N, p, {
                double w;
                w = *__P(i, jW, N);
                *p = *__P(i - 1, j, N) + muDt + itoDrift + sigmaRootDt * w;
            })
        } else {
            __FILL_ARRAY(pmat, double, i, j, 1, N-1, jj1, jj2, N, p, {
                double w;
                w = *__P(i, j, N);
                *p = *__P(i-1, j, N) + muDt + itoDrift + sigmaRootDt * w;
            })
        }
        __FILL_ARRAY(pmat, double, i, j, 0, N-1, jj1, jj2, N, p, {
            *p = exp(*p);
        })
        return Py_NewRef(pyMat);
    }

    else {
        return PyErr_Format(PyExc_ValueError, "Unknown op \"%s\"", op);
    }

}
#undef IARG_MAT
#undef IARG_OP

#pragma pop_macro("__P")
#endif      // SRC_JONES_PYQU_C
