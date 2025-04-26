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