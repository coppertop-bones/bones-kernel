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


#ifndef SRC_JONES_MOD_QU_C
#define SRC_JONES_MOD_QU_C "jones/mod_qu.c"


#include "qu.h"
#include "pyqu.c"


// ---------------------------------------------------------------------------------------------------------------------
// qu module
// ---------------------------------------------------------------------------------------------------------------------

pvt PyMethodDef qu_fns[] = {
    {"cn_Hart", (PyCFunction)               Py_qu_cn_Hart, METH_FASTCALL, "cn_Hart(x)\n\n`Cumulative normal distribution - answers the probability for a given x (-inf to +inf)"},
    {"invcn_Acklam", (PyCFunction)          Py_qu_invcn_Acklam, METH_FASTCALL, "invcn_Acklam(p)\n\nInverse cumulative normal distribution - answers x (-inf to +inf) given probability p"},
    {"cn_h", (PyCFunction)                  Py_qu_cn_h, METH_FASTCALL, "cn_h(x)\n\n`Cumulative normal distribution - answers the probability for a given x (-inf to +inf)"},
    {"invcn_h", (PyCFunction)               Py_qu_invcn_h, METH_FASTCALL, "invcn_h(p)\n\nInverse cumulative normal distribution - answers x (-inf to +inf) given probability p"},

    {"b76_call", (PyCFunction)              Py_qu_b76_call, METH_FASTCALL, "b76_call(tenor, strike, forward, vol, r) -> price"},
    {"b76_call_greeks", (PyCFunction)       Py_qu_b76_call_greeks, METH_FASTCALL, "b76_call_greeks(tenor, strike, forward, vol, r) -> greeks"},
    {"b76_put", (PyCFunction)               Py_qu_b76_put, METH_FASTCALL, "b76_put(tenor, strike, forward, vol, r) -> price"},
    {"b76_put_greeks", (PyCFunction)        Py_qu_b76_put_greeks, METH_FASTCALL, "b76_put_greeks(tenor, strike, forward, vol, r) -> greeks"},

    {"bachelier_call", (PyCFunction)        Py_qu_bachelier_call, METH_FASTCALL, "bachelier_call(tenor, strike, forward, vol, r) -> price"},
    {"bachelier_put", (PyCFunction)         Py_qu_bachelier_put, METH_FASTCALL, "bachelier_put(tenor, strike, forward, vol, r) -> price"},

    {"new_mersennes_f64", (PyCFunction)     Py_qu_new_mersennes_f64, METH_FASTCALL, "new_mersennes_f64(n [, m]) -> matrix"},
    {"new_mersennes_norm", (PyCFunction)    Py_qu_new_mersennes_norm, METH_FASTCALL, "new_mersennes_norm(n [, m]) -> matrix"},
    {"fill_mersennes_norm", (PyCFunction)   Py_qu_fill_mersennes_norm, METH_FASTCALL, "fill_mersennes_norm(matrix, i1, i2, j1, j2)"},
    {"fill_matrix", (PyCFunction)           Py_qu_fill_matrix, METH_FASTCALL | METH_KEYWORDS, "fill_matrix(matrix, j, op, **kwargs)"},

    {0}
};

pvt PyModuleDef qu_module = {
    PyModuleDef_HEAD_INIT,
    .m_name = "qu",
    .m_doc = "Quant Utils",
    .m_size = -1,
    qu_fns
};

pub PyMODINIT_FUNC PyInit_qu(void) {
    PyObject *m;
    import_array();  // Initialize NumPy C-API
    m = PyModule_Create(&qu_module);
    if (m == 0) return 0;
    return m;
}

#endif  // SRC_JONES_MOD_QU_C

