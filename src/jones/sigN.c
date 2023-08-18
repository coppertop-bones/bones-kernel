#ifndef __JONES_SIGN_C
#define __JONES_SIGN_C "jones/sigN.c"

#include "pipe_structs.c"



// ---------------------------------------------------------------------------------------------------------------------
// SigN
// ---------------------------------------------------------------------------------------------------------------------

struct SigN {
    PyObject_HEAD
    SigHeader h;
    TypeNum types[];
};


pvt void SigN_dealloc(struct SigN *self) {
    Py_TYPE(self)->tp_free((PyObject *) self);
}

pvt PyObject * SigN_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    struct SigN *self;
    self = (struct SigN *) type->tp_alloc(type, 0);
    if (self != 0) {

    }
    return (PyObject *) self;
}


pvt int SigN_init(struct SigN *self, PyObject *args, PyObject *kwds) {
    return 0;
}


static PyMemberDef SigN_members[] = {
//    {"first", T_OBJECT_EX, offsetof(struct SigN, first), 0, "first name"},
//    {"last", T_OBJECT_EX, offsetof(struct SigN, last), 0, "last name"},
//    {"number", T_INT, offsetof(Toy, number), 0, "custom number"},
    {0}
};


static PyMethodDef SigN_methods[] = {
//    {"has", (PyCFunction) Toy_has, METH_FASTCALL, "has(key)\n\nanswer if has key"},
//    {"atPut", (PyCFunction) Toy_atPut, METH_FASTCALL, "atPut(key, value)\n\nat key put value, answer self"},
//    {"atIfNone", (PyCFunction) Toy_atIfNone, METH_FASTCALL, "atIfNone(key, value, alt)\n\nanswer the value at key or alt if the key is absent"},
//    {"drop", (PyCFunction) Toy_drop, METH_FASTCALL, "drop(key)\n\ndrop value at key, answer self"},
//    {"count", (PyCFunction) Toy_count, METH_FASTCALL, "count()\n\nanswer the number of elements"},
//    {"numBuckets", (PyCFunction) Toy_numBuckets, METH_FASTCALL, "numBuckets()\n\nanswer the number of buckets"},
//    {"name", (PyCFunction) Toy_name, METH_NOARGS, "Return the name, combining the first and last name"},
    {0}
};


static PyTypeObject SigNCls = {
    PyVarObject_HEAD_INIT(0, 0)
    .tp_name = "jones.SigN",
    .tp_doc = PyDoc_STR("Type signature of a 1 arg function"),
    .tp_basicsize = sizeof(struct SigN),
    .tp_itemsize = sizeof(TypeNum),
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = SigN_new,
    .tp_init = (initproc) SigN_init,
    .tp_dealloc = (destructor) SigN_dealloc,
    .tp_members = SigN_members,
    .tp_methods = SigN_methods,
};



#endif  // __JONES_SIGN_C