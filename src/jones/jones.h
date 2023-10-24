// https://docs.python.org/3/extending/extending.html#ownership-rules
// "The object reference returned from a C function that is called from Python must be an owned reference"


#ifndef __JONES__JONES_H
#define __JONES__JONES_H "jones/_jones.h"


#define PY_SSIZE_T_CLEAN

#include "config.h"
#include BK_PYTHON_H
#include BK_DESCROBJECT_H
#include BK_STRUCTMEMBER_H

// Python 3.12 prefixes stuff with Py_
#define Py_T_OBJECT_EX  /*6*/       T_OBJECT
#define Py_READONLY     /*1*/       READONLY
#define Py_T_INT        /*1*/       T_INT
#define Py_T_UBYTE      /*9*/       T_UBYTE
#define Py_T_UINT       /*11*/      T_UINT


#include "../../include/bk/bk.h"
#include "../../include/bk/tm.h"


struct Base {
    PyObject_VAR_HEAD;
};

struct Fn {
    struct Base Base;
    PyObject *name;
    PyObject *bmod;
    PyObject *d;            // dispatcher
    PyObject *TBCSentinel;
};

struct Partial {
    struct Fn Fn;
    unsigned char num_tbc;  // the number of arguments missing in the args array
                            // pad48
    PyObject *pipe1;        // 1st piped arg for binaries and ternaries
    PyObject *pipe2;        // 2nd piped arg for ternaries
    PyObject *args[];
};

struct PyBType {
    PyObject_HEAD;
    btypeid btypeid;
};

struct PySM {
    PyObject_HEAD;
    struct SM *sm;
};

struct PyEM {
    PyObject_HEAD;
    struct EM *em;
};

struct PyTM {
    PyObject_HEAD;
    struct TM *tm;
};

struct PyKernel {
    PyObject_HEAD;
    struct K *kernel;
    PyObject *pySM;
    PyObject *pyEM;
    PyObject *pyTM;
};


pvt PyObject *PyJonesError;
pvt PyObject *PyJonesSyntaxError;

pvt PyTypeObject PyBTypeCls;
pvt PyTypeObject PyKernelCls;

pvt PyTypeObject PyNullaryCls;
pvt PyTypeObject PyUnaryCls;
pvt PyTypeObject PyBinaryCls;
pvt PyTypeObject PyTernaryCls;

pvt PyTypeObject PyPNullaryCls;
pvt PyTypeObject PyPUnaryCls;
pvt PyTypeObject PyPBinaryCls;
pvt PyTypeObject PyPTernaryCls;


#define PTR_MASK 0x0000FFFFFFFFFFFF


#endif  // __JONES__JONES_H