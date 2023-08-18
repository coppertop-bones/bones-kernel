#ifndef __JONES_PIPE_STRUCTS_C
#define __JONES_PIPE_STRUCTS_C "jones/pipe_structs.c"


#define PY_SSIZE_T_CLEAN

#include "Python.h"
#include "../bk/fn_select.c"


struct Base {
    PyObject_VAR_HEAD
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
    unsigned char num_tbc;            // the number of arguments missing in the args array
                            // pad48
    PyObject *pipe1;        // 1st piped arg for binaries and ternaries
    PyObject *pipe2;        // 2nd piped arg for ternaries
    PyObject *args[];
};


static PyObject *JonesError;
static PyObject *JonesSyntaxError;


#endif  // __JONES_PIPE_STRUCTS_C