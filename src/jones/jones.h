// ---------------------------------------------------------------------------------------------------------------------
// Copyright 2025 David Briant, https://github.com/coppertop-bones. Licensed under the Apache License, Version 2.0
// ---------------------------------------------------------------------------------------------------------------------

#ifndef SRC_JONES_JONES_H
#define SRC_JONES_JONES_H "jones/jones.h"

// https://docs.python.org/3/extending/extending.html#ownership-rules
// "The object reference returned from a C function that is called from Python must be an owned reference"

#define PY_SSIZE_T_CLEAN

//#include <unistd.h>
//#include <sys/mman.h>

#include "../../include/bk/_cfg.h"
#include BK_PYTHON_H
#include BK_DESCROBJECT_H
#include BK_STRUCTMEMBER_H

// Python 3.12 prefixes stuff with Py_
#define Py_T_OBJECT_EX  /*6*/       T_OBJECT
#define Py_READWRITE     /*0*/       0
#define Py_READONLY     /*1*/       READONLY
#define Py_T_INT        /*1*/       T_INT
#define Py_T_UBYTE      /*9*/       T_UBYTE
#define Py_T_UINT       /*11*/      T_UINT


#include "../../include/bk/bk.h"
#include "../../include/bk/k.h"


struct Base {
    PyObject_VAR_HEAD
};

// OPEN: rename to CoppertopFn
struct Fn {
    struct Base Base;
    PyObject *name;
    PyObject *bmod;
    PyObject *d;            // dispatcher
    PyObject *TBCSentinel;
};

// OPEN: rename to CoppertopPartial
struct Partial {
    struct Fn Fn;
    u8 num_tbc;             // the number of arguments missing in the args array
                            // pad48
    PyObject *pipe1;        // 1st piped arg for binaries and ternaries
    PyObject *pipe2;        // 2nd piped arg for ternaries
    PyObject *args[];
};

typedef struct {
    PyObject_HEAD
    btypeid_t btypeid;
} PyBType;

typedef struct {
    PyObject_HEAD
    BK_SM *sm;
} PySM;

typedef struct {
    PyObject_HEAD
    BK_EM *em;
} PyEM;

typedef struct {
    PyObject_HEAD
    BK_TM *tm;
} PyTM;

typedef struct {
    PyObject_HEAD
    BK_K *kernel;
    PyObject *pySM;
    PyObject *pyEM;
    PyObject *pyTM;
} PyKernel;


pvt PyObject *PyJonesError;

pvt PyObject *PyCoppertopSyntaxError;

pvt PyObject *PyBTypeError;
pvt PyObject *PySchemaError;

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

pvt PyTypeObject PyJSelectionResultCls;
pvt PyTypeObject PyFitsCls;
pvt PyTypeObject PyJFuncCls;
pvt PyTypeObject PyJOverloadCls;
pvt PyTypeObject PyJFamilyCls;

pvt PyObject *PyBType_py;       // like an any

#define PTR_MASK 0x0000FFFFFFFFFFFF


#endif  // SRC_JONES_JONES_H