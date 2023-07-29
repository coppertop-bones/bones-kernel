// ---------------------------------------------------------------------------------------------------------------------
// Copyright 2025 David Briant, https://github.com/coppertop-bones. Licensed under the Apache License, Version 2.0
//
// Single point of compile time configuration for the jones and qu libraries.
// ---------------------------------------------------------------------------------------------------------------------

#ifndef LIB_CONFIG_COMMON_H
#define LIB_CONFIG_COMMON_H "config/common.h"

#define BK_EXPOSE_TDD

// #define C_LION
// #define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION

#ifdef C_LION
    // for CLion
    #define BK_PYTHON_H "/Users/david/miniforge3/envs/b311/include/python3.11/Python.h"
    #define BK_DESCROBJECT_H "/Users/david/miniforge3/envs/b311/include/python3.11/descrobject.h"
    #define BK_STRUCTMEMBER_H "/Users/david/miniforge3/envs/b311/include/python3.11/structmember.h"
    #define BK_NUMPY_ARRAYOBJECT_H "/Users/david/miniforge3/envs/b311/lib/python3.11/site-packages/numpy/core/include/numpy/arrayobject.h"
#else
     #define BK_PYTHON_H "Python.h"
     #define BK_DESCROBJECT_H "descrobject.h"
     #define BK_STRUCTMEMBER_H "structmember.h"
     #define BK_NUMPY_ARRAYOBJECT_H "numpy/arrayobject.h"
#endif

#define BK_NUMPY_SUPPRESS_UNUSED_FUNCTION_WARNING
#define BK_NUMPY_SUPPRESS_DEPRECATED_API_WARNING


//#define JONES_INCLUDE_PLAY


#endif  // LIB_CONFIG_COMMON_H