#ifndef SRC_JONES_JONES_C
#define SRC_JONES_JONES_C "jones/jones.c"


#include "jones.h"
#include "pyqu.c"


// ---------------------------------------------------------------------------------------------------------------------
// init module
// ---------------------------------------------------------------------------------------------------------------------

pvt PyMethodDef qu_fns[] = {
    {"cn_Hart", (PyCFunction)       Py_qu_cn_Hart, METH_FASTCALL, "cn_Hart(x)\n\n`Cumulative normal distribution - answers the probability for a given x (-inf to +inf)"},
    {"invcn_Acklam", (PyCFunction)  Py_qu_invcn_Acklam, METH_FASTCALL, "invcn_Acklam(p)\n\nInverse cumulative normal distribution - answers x (-inf to +inf) given probability p"},

    {"b76_call", (PyCFunction)  Py_qu_b76_call, METH_FASTCALL, "b76_call(tenor, strike, forward, vol, r) -> price"},
    {"b76_put", (PyCFunction)  Py_qu_b76_put, METH_FASTCALL, "b76_put(tenor, strike, forward, vol, r) -> price"},

    {0, 0, 0, 0}
};

pvt PyModuleDef qu_module = {
    PyModuleDef_HEAD_INIT,
    .m_name = "jones.qu",
    .m_doc = "Quant Utils",
    .m_size = -1,
    qu_fns
};


pub PyMODINIT_FUNC PyInit_qu(void) {
    PyObject *m;
    m = PyModule_Create(&qu_module);
    if (m == 0) return 0;
    return m;
}


#endif  // SRC_JONES_JONES_C

