#ifndef __JONES__STRUCTS_H
#define __JONES__STRUCTS_H "jones/_common.h"


#define PY_SSIZE_T_CLEAN

#include "Python.h"
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
    btype btype;
};

struct PyKernel {
    PyObject_HEAD;
    struct K *pKernel;
};

struct PySM {
    PyObject_HEAD;
    struct SM *pSm;
};

struct PyEM {
    PyObject_HEAD;
    struct EM *pEm;
};

struct PyTM {
    PyObject_HEAD;
    struct TM *pTm;
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


#endif  // __JONES__STRUCTS_H