// ---------------------------------------------------------------------------------------------------------------------
//
//                             Copyright (c) 2019-2025 David Briant. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance
// with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software distributed under the License is distributed
// on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for
// the specific language governing permissions and limitations under the License.
//
// ---------------------------------------------------------------------------------------------------------------------


// ---------------------------------------------------------------------------------------------------------------------
// PYBTYPE - PYTHON BTYPE
// ---------------------------------------------------------------------------------------------------------------------

#ifndef SRC_JONES_PYBTYPE_C
#define SRC_JONES_PYBTYPE_C "jones/pybtype.c"

#include "../../include/jones/jones.h"



// ---------------------------------------------------------------------------------------------------------------------
// PyBTypeCls
// ---------------------------------------------------------------------------------------------------------------------

pvt PyObject * PyBType_create(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    PyBType *self = (PyBType *) type->tp_alloc(type, 0);
    return (PyObject *) self;
}

pvt void PyBType_trash(PyBType *self) {
    Py_TYPE(self)->tp_free((PyObject *) self);
}

pvt PyMemberDef PyBType_members[] = {
    {"id", Py_T_UINT, offsetof(PyBType, btypeid), 0, "bones type id"},
    {0}
};

pvt PyMethodDef PyBType_methods[] = {
    {0}
};

//char *buf = malloc(1000);

pvt PyObject * PyBType__str__(PyBType *self) {
//    return PyString_FromFormat("btype%i", self->btypeid);

    return PyUnicode_FromString("t");
//    return PyUnicodeUCS2_FromString("t");
}


pvt PyTypeObject PyBTypeCls = {
    PyVarObject_HEAD_INIT(0, 0)
    .tp_name = "jones.BType",
    .tp_doc = PyDoc_STR("A bones type"),
    .tp_basicsize = sizeof(PyBType),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = PyBType_create,
    .tp_dealloc = (destructor) PyBType_trash,
    .tp_members = PyBType_members,
    .tp_methods = PyBType_methods,
    .tp_str = (reprfunc) PyBType__str__,
};


#endif  // SRC_JONES_PYBTYPE_C