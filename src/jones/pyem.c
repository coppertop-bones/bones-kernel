// ---------------------------------------------------------------------------------------------------------------------
// PYEM - PYTHON INTERFACE TO ENUM MANAGER
// ---------------------------------------------------------------------------------------------------------------------

#ifndef SRC_JONES_PYEM_C
#define SRC_JONES_PYEM_C "jones/pyem.c"


#include "../../include/jones/jones.h"
#include "../bk/em.c"


// ---------------------------------------------------------------------------------------------------------------------
// PyEM
// ---------------------------------------------------------------------------------------------------------------------

pvt PyMethodDef PyEM_methods[] = {
    {0}
};

pvt PyTypeObject PyEMCls = {
    PyVarObject_HEAD_INIT(0, 0)
    .tp_name = "jones.EM",
    .tp_doc = PyDoc_STR("TBC"),
    .tp_basicsize = sizeof(PyEM),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_methods = PyEM_methods,
};



#endif  // SRC_JONES_PYEM_C