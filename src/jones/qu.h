// ---------------------------------------------------------------------------------------------------------------------
// Copyright 2025 David Briant, https://github.com/coppertop-bones. Licensed under the Apache License, Version 2.0
// ---------------------------------------------------------------------------------------------------------------------

#ifndef SRC_JONES_QU_H
#define SRC_JONES_QU_H "jones/qu.h"

// https://docs.python.org/3/extending/extending.html#ownership-rules
// "The object reference returned from a C function that is called from Python must be an owned reference"

#define PY_SSIZE_T_CLEAN

#include "../../lib/config/common.h"
#include BK_PYTHON_H
#include BK_DESCROBJECT_H
#include BK_STRUCTMEMBER_H


#pragma clang diagnostic push
#ifdef BK_NUMPY_SUPPRESS_UNUSED_FUNCTION_WARNING
#pragma clang diagnostic ignored "-Wunused-function"
#endif
#ifdef BK_NUMPY_SUPPRESS_DEPRECATED_API_WARNING
#pragma clang diagnostic ignored "-W#warnings"
#endif

#include BK_NUMPY_ARRAYOBJECT_H

#pragma clang diagnostic pop


// Python 3.12 prefixes stuff with Py_
#define Py_T_OBJECT_EX  /*6*/       T_OBJECT
#define Py_READONLY     /*1*/       READONLY
#define Py_T_INT        /*1*/       T_INT
#define Py_T_UBYTE      /*9*/       T_UBYTE
#define Py_T_UINT       /*11*/      T_UINT


#include "../../include/bk/bk.h"
#include "../../include/bk/k.h"


pvt PyObject *PyJonesError;

pvt PyObject *PyCoppertopSyntaxError;

pvt PyObject *PyBTypeError;



#define PTR_MASK 0x0000FFFFFFFFFFFF


#endif  // SRC_JONES_QU_H