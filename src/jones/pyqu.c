// ---------------------------------------------------------------------------------------------------------------------
// quant utils
// ---------------------------------------------------------------------------------------------------------------------
#ifndef SRC_JONES_PYQU_C
#define SRC_JONES_PYQU_C "jones/pyqu.c"


#include "jones.h"
#include "../qu/black.c"
#include "../qu/dists.c"
#include "../qu/mt.c"
#include "lib/pyutils.h"
#include BK_NUMPY_ARRAYOBJECT_H



// ---------------------------------------------------------------------------------------------------------------------
// b76_call: p:PyFloat -> PyFloat + PyException
// ---------------------------------------------------------------------------------------------------------------------
pvt PyObject * Py_qu_b76_call(PyTM *self, PyObject **args, Py_ssize_t nargs) {
    double t, r, df;
    // OPEN: assert tenor, strike, vol, r >= 0 and forward > 0
    if (nargs != 5) return jErrWrongNumberOfArgs(FN_NAME, 5, nargs);
    // OPEN: convert ints to floats
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
    // OPEN: convert ints to floats
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
    // OPEN: convert ints to floats
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
// b76_put_greeks
//      (p:PyFloat) -> PyFloat + PyException
// ---------------------------------------------------------------------------------------------------------------------
pvt PyObject * Py_qu_b76_put_greeks(PyTM *self, PyObject **args, Py_ssize_t nargs) {
    double t, r, df;  black_greeks greeks;  PyObject *answer;
    // OPEN: assert tenor, strike, vol, r >= 0 and forward > 0
    if (nargs != 5) return jErrWrongNumberOfArgs(FN_NAME, 5, nargs);
    // OPEN: convert ints to floats
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
// invcn_Acklam
//      (p:PyFloat) -> PyFloat + PyException
// ---------------------------------------------------------------------------------------------------------------------
pvt PyObject * Py_qu_invcn_Acklam(PyTM *self, PyObject **args, Py_ssize_t nargs) {
    double p;
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    // OPEN: convert ints to floats
    if (!PyFloat_Check(args[0])) return PyErr_Format(PyExc_TypeError, "p is not a float");
    p = PyFloat_AsDouble(args[0]);
    return PyFloat_FromDouble(qu_invcn_Acklam(p));
}

// ---------------------------------------------------------------------------------------------------------------------
// cn_Hart
//      (x:PyFloat) -> PyFloat + PyException
// ---------------------------------------------------------------------------------------------------------------------
pvt PyObject * Py_qu_cn_Hart(PyTM *self, PyObject **args, Py_ssize_t nargs) {
    double x;
    if (nargs != 1) return jErrWrongNumberOfArgs(FN_NAME, 1, nargs);
    // OPEN: convert ints to floats
    if (!PyFloat_Check(args[0])) return PyErr_Format(PyExc_TypeError, "x is not a float");
    x = PyFloat_AsDouble(args[0]);
    return PyFloat_FromDouble(qu_cn_Hart(x));
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

#define __INIT_ARRAY(DATA, TYPE, BLOCK)                                                                                 \
{                                                                                                                       \
    TYPE __attribute__((unused)) *_data;                                                                                \
    _data = DATA;                                                                                                       \
    BLOCK                                                                                                               \
}

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

#define checkPyLong(x, msg) {if (!PyLong_Check(x)) return PyErr_Format(PyExc_TypeError, msg);}
#define checkPyFloat(x, msg) {if (!PyFloat_Check(x)) return PyErr_Format(PyExc_TypeError, msg);}


// ---------------------------------------------------------------------------------------------------------------------
// new_mersennes_f64:
//      (n:PyLong) -> ndarray + PyException
//      (n:PyLong, m:PyLong) -> ndarray + PyException
// ---------------------------------------------------------------------------------------------------------------------
pvt PyObject * Py_qu_new_mersennes_f64(PyObject *self, PyObject **args, Py_ssize_t nargs) {
    int n, m;  PyObject *mat;
    if (nargs == 1) {
        if (!PyLong_Check(args[0])) return PyErr_Format(PyExc_TypeError, "n must be an int");
        n = (int) PyLong_AsLong(args[0]);
        npy_intp dims[1] = {n};
        if ((mat=PyArray_SimpleNew(1, dims, NPY_FLOAT64)) == 0) PyErr_Format(PyExc_TypeError, "could not create np.ndarray");
        __FILL_ARRAY(PyArray_DATA(mat), double, i, j, 0, n-1, 0, 0, n, p, {
            *p = qu_mt_f64_oo();
        })
        return mat;
    } else if (nargs == 2) {
        if (!PyLong_Check(args[0])) return PyErr_Format(PyExc_TypeError, "n must be an int");
        if (!PyLong_Check(args[1])) return PyErr_Format(PyExc_TypeError, "m must be an int");
        n = (int) PyLong_AsLong(args[0]);
        m = (int) PyLong_AsLong(args[1]);
        npy_intp dims[2] = {n, m};
        if ((mat=PyArray_New(&PyArray_Type, 2, dims, NPY_FLOAT64, NULL, NULL, 0, NPY_ARRAY_F_CONTIGUOUS, NULL)) == 0) PyErr_Format(PyExc_TypeError, "could not create np.ndarray");
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
        if (!PyLong_Check(args[0])) return PyErr_Format(PyExc_TypeError, "n must be an int");
        n = (int) PyLong_AsLong(args[0]);
        npy_intp dims[1] = {n};
        if ((mat=(PyArrayObject *)PyArray_SimpleNew(1, dims, NPY_FLOAT64)) == 0) PyErr_Format(PyExc_TypeError, "could not create np.ndarray");

        __FILL_ARRAY(PyArray_DATA((PyArrayObject *) mat), double, i, j, 0, n-1, 0, 0, n, p, {
            *p = qu_invcn_Acklam(qu_mt_f64_oo());
        })
        return (PyObject *) mat;
    } else if (nargs == 2) {
        if (!PyLong_Check(args[0])) return PyErr_Format(PyExc_TypeError, "n must be an int");
        if (!PyLong_Check(args[1])) return PyErr_Format(PyExc_TypeError, "m must be an int");
        n = (int) PyLong_AsLong(args[0]);
        m = (int) PyLong_AsLong(args[1]);
        npy_intp dims[2] = {n, m};
        if ((mat=(PyArrayObject *)PyArray_New(&PyArray_Type, 2, dims, NPY_FLOAT64, NULL, NULL, 0, NPY_ARRAY_F_CONTIGUOUS, NULL)) == 0) PyErr_Format(PyExc_TypeError, "could not create np.ndarray");

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
    if (!PyArray_Check(args[0])) return PyErr_Format(PyExc_TypeError, "matrix must be a numpy array");
    if (!PyLong_Check(args[1]) || !PyLong_Check(args[2]) || !PyLong_Check(args[3]) || !PyLong_Check(args[4])) return PyErr_Format(PyExc_TypeError, "i1, i2, j1 & j2 must be an int");
    mat = (PyArrayObject *) args[0];
    if ((PyArray_TYPE(mat) != NPY_FLOAT64)) return PyErr_Format(PyExc_TypeError, "matrix must have dtype np.float64");
    if (!(PyArray_FLAGS(mat) & NPY_ARRAY_F_CONTIGUOUS)) return PyErr_Format(PyExc_TypeError, "matrix must be in fortran (col major) form");
    i1 = (int) PyLong_AsLong(args[1]);
    i2 = (int) PyLong_AsLong(args[2]);
    j1 = (int) PyLong_AsLong(args[3]);
    j2 = (int) PyLong_AsLong(args[4]);
    ndim = PyArray_NDIM(mat);
    if (ndim < 1 || 2 < ndim) return PyErr_Format(PyExc_TypeError, "matrix must have 1 or 2 dimensions but has %i", ndim);
    shape = PyArray_DIMS(mat);
    n = shape[0];
    m = ndim == 2 ? shape[1] : 0;
    if (i1 < 0 || i1 >= n || i2 < 0 || i2 >= n || i1 > i2) return PyErr_Format(PyExc_TypeError, "i1 and i2 must be in range 0 <= i1 <= i2 < n");
    if (j1 < 0 || j1 >= m || j2 < 0 || j2 >= m || j1 > j2) return PyErr_Format(PyExc_TypeError, "j1 and j2 must be in range 0 <= j1 <= j2 < m");

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
#define ARG_MAT 0
#define ARG_OP 1
pvt PyObject * Py_qu_fill_matrix(PyObject *self, PyObject *const *args, Py_ssize_t nargs, PyObject *argnames) {
    // OPEN: "SABR", {"wa":0, "wf":1, "a":0.10, "nu":0.40, ...}) , "sobol", "mersenne_norm", "SLN", "N", "SLNSAR", STEPS:, DT:, etc
    PyArrayObject *mat;  int N, M, jj, jj1=0, jj2=0, ndim;  npy_intp *shape;  double *pmat;  char *op;  PyObject *pyJ=0, *pyJ1=0, *pyJ2=0;
    if (nargs != 2) return PyErr_Format(PyExc_TypeError, "fill_matrix(matrix, op, **kwargs) only takes 2 args but %i were given", nargs);

    // matrix
    if (!PyArray_Check(args[ARG_MAT])) return PyErr_Format(PyExc_TypeError, "matrix must be a numpy array");
    mat = (PyArrayObject *) args[ARG_MAT];
    if ((PyArray_TYPE(mat) != NPY_FLOAT64)) return PyErr_Format(PyExc_TypeError, "matrix must have dtype np.float64");
    if (!(PyArray_FLAGS(mat) & NPY_ARRAY_F_CONTIGUOUS)) return PyErr_Format(PyExc_TypeError, "matrix must be in fortran (col major) form");
    ndim = PyArray_NDIM(mat);
    if (ndim != 2) return PyErr_Format(PyExc_TypeError, "matrix must have 2 dimensions but has %i", ndim);
    shape = PyArray_DIMS(mat);
    N = shape[0];
    M = shape[1];
    pmat = PyArray_DATA((PyArrayObject *) mat);

    // op
    if (!PyUnicode_Check(args[ARG_OP]) || (PyUnicode_KIND(args[ARG_OP]) != PyUnicode_1BYTE_KIND)) return PyErr_Format(PyExc_TypeError, "op must be utf8");
    op = (char *) PyUnicode_AsUTF8(args[ARG_OP]);

    // j, j1, j2
    for (int i = 0; i < PyTuple_Size(argnames); i++) {
        if (strcmp("j", PyUnicode_AsUTF8(PyTuple_GET_ITEM(argnames, i))) == 0) pyJ = args[i + nargs];
        else if (strcmp("j1", PyUnicode_AsUTF8(PyTuple_GET_ITEM(argnames, i))) == 0) pyJ1 = args[i + nargs];
        else if (strcmp("j2", PyUnicode_AsUTF8(PyTuple_GET_ITEM(argnames, i))) == 0) pyJ2 = args[i + nargs];
    }
    if (pyJ && (pyJ1 || pyJ2)) return PyErr_Format(PyExc_SyntaxError, "Either specify j or both j1 and j2");
    if ((pyJ1 && !pyJ2) || (!pyJ1 && pyJ2)) return PyErr_Format(PyExc_SyntaxError, "Must specify both j1 and j2");
    if (pyJ) {
        checkPyLong(pyJ, "j must be an int");  jj = (int) PyLong_AsLong(pyJ);
        if (jj < 0 || jj >= M) return PyErr_Format(PyExc_ValueError, "j must be in range 0 <= j < M");
    }
    if (pyJ1) {
        checkPyLong(pyJ1, "j1 must be an int");  jj1 = (int) PyLong_AsLong(pyJ1);
        if (jj1 < 0 || jj1 >= M) return PyErr_Format(PyExc_ValueError, "j1 must be in range 0 <= j1 < M");
    }
    if (pyJ2) {
        checkPyLong(pyJ2, "j2 must be an int");  jj2 = (int) PyLong_AsLong(pyJ2);
        if (jj2 < 0 || jj2 >= M) return PyErr_Format(PyExc_ValueError, "j2 must be in range 0 <= j2 < M");
    }
    if (jj1 > jj2) return PyErr_Format(PyExc_ValueError, "j1 must be in range j1 <= j2");

    // op == "norm"
    if (strcmp(op, "norm") == 0) {
        // options: jW - the col containing the N(0,1), dt, sigma, mu
        PyObject *pyJW=0, *pyDt=0, *pySigma=0, *pyMu=0;  int jW=jj;  double dt=1, sigma=1, mu=0, sigmaRootDt, muDt;
        for (int i = 0; i < PyTuple_Size(argnames); i++) {
            if (strcmp("jW", PyUnicode_AsUTF8(PyTuple_GET_ITEM(argnames, i))) == 0) pyJW = args[i + nargs];
            else if (strcmp("dt", PyUnicode_AsUTF8(PyTuple_GET_ITEM(argnames, i))) == 0) pyDt = args[i + nargs];
            else if (strcmp("sigma", PyUnicode_AsUTF8(PyTuple_GET_ITEM(argnames, i))) == 0) pySigma = args[i + nargs];
            else if (strcmp("mu", PyUnicode_AsUTF8(PyTuple_GET_ITEM(argnames, i))) == 0) pyMu = args[i + nargs];
        }
        if (pyJW) {
            if (pyJ1) return PyErr_Format(PyExc_SyntaxError, "Cannot specify jW if j1 and j2 are specified");
            checkPyLong(pyJW, "jW must be an int");  jW = (int) PyLong_AsLong(pyJW);
            if (jW < 0 || jW >= M) return PyErr_Format(PyExc_ValueError, "jW must be in range 0 <= jW < M");
        }
        if (pyDt) {
            checkPyFloat(pyDt, "dt must be an double");  dt = PyFloat_AsDouble(pyDt);
            if (dt < 0) return PyErr_Format(PyExc_ValueError, "dt must be in range 0 < dt < +inf");
        }
        if (pySigma) {
            checkPyFloat(pySigma, "sigma must be an double");  sigma = PyFloat_AsDouble(pySigma);
            if (sigma <= 0 || sigma > 2) return PyErr_Format(PyExc_ValueError, "sigma must be in range 0% <= sigma <= 200%");
        }
        if (pyMu) {
            checkPyFloat(pyMu, "mu must be an double");  mu = PyFloat_AsDouble(pyMu);
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
        return Py_NewRef(mat);
    }

    // op == "log"
    else if (strcmp(op, "log") == 0) {
        // options: jW - the col containing the N(0,1), dt, sigma, mu
        PyObject *pyJW=0, *pyDt=0, *pySigma=0, *pyMu=0;  int jW=jj;  double dt=1, sigma=1, mu=0, sigmaRootDt, muDt, itoDrift;
        for (int i = 0; i < PyTuple_Size(argnames); i++) {
            if (strcmp("jW", PyUnicode_AsUTF8(PyTuple_GET_ITEM(argnames, i))) == 0) pyJW = args[i + nargs];
            else if (strcmp("dt", PyUnicode_AsUTF8(PyTuple_GET_ITEM(argnames, i))) == 0) pyDt = args[i + nargs];
            else if (strcmp("sigma", PyUnicode_AsUTF8(PyTuple_GET_ITEM(argnames, i))) == 0) pySigma = args[i + nargs];
            else if (strcmp("mu", PyUnicode_AsUTF8(PyTuple_GET_ITEM(argnames, i))) == 0) pyMu = args[i + nargs];
        }
        if (pyJW) {
            checkPyLong(pyJW, "jW must be an int");  jW = (int) PyLong_AsLong(pyJW);
            if (jW < 0 || jW >= M) return PyErr_Format(PyExc_ValueError, "jW must be in range 0 <= jW < M");
        }
        if (pyDt) {
            checkPyFloat(pyDt, "dt must be an double");  dt = PyFloat_AsDouble(pyDt);
            if (dt < 0) return PyErr_Format(PyExc_ValueError, "dt must be in range 0 < dt < +inf");
        }
        if (pySigma) {
            checkPyFloat(pySigma, "sigma must be an double");  sigma = PyFloat_AsDouble(pySigma);
            if (sigma <= 0 || sigma > 2) return PyErr_Format(PyExc_ValueError, "sigma must be in range 0% <= sigma <= 200%");
        }
        if (pyMu) {
            checkPyFloat(pyMu, "mu must be an double");  mu = PyFloat_AsDouble(pyMu);
        }
        sigmaRootDt = sigma * sqrt(dt);
        muDt = mu * dt;
        itoDrift = - 0.5 * sigma * sigma * dt;

        if (pyJ) {jj1 = jj; jj2 = jj;}
        __FILL_ARRAY(pmat, double, i, j, 0, 0, jj1, jj2, N, p, {
            double first;
            first = *__P(i, j, N);
            if (first <= 0) return PyErr_Format(PyExc_ValueError, "matrix[0,%i] must be in range 0 < x < +inf", j);
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
        return Py_NewRef(mat);
    }

    else {
        return PyErr_Format(PyExc_ValueError, "Unknown op \"%s\"", op);
    }

}
#undef ARG_MAT
#undef ARG_OP

#pragma pop_macro("__P")
#endif      // SRC_JONES_PYQU_C
