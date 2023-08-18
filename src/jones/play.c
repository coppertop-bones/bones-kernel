#ifndef __JONES_PLAY_C
#define __JONES_PLAY_C "jones/play.c"


#include "pipe_structs.c"



#include "../other/khash.h"


pvt kh_inline khint_t __ac_X31_hash_fred(const char *s) {
    khint_t h = (khint_t)*s;
    if (h) for (++s ; *s; ++s) h = (h << 5) - h + (khint_t)*s;
    return h;
}

pvt int fredcmp (const char *p1, const char *p2) {
    const char *s1 = (const char *) p1;
    const char *s2 = (const char *) p2;
    char c1, c2;
    do {
        c1 = (char) *s1++;
        c2 = (char) *s2++;
        if (c1 == '\0') return c1 - c2;
    }
    while (c1 == c2);
    return c1 - c2;
}

#define kh_fred_hash_func(key) __ac_X31_hash_fred(key)
#define kh_fred_hash_equal(a, b) (fredcmp(a, b) == 0)



#define KHASH_MAP_INIT_INT32(name, khval_t)								\
	KHASH_INIT(name, khint32_t, khval_t, 1, kh_int_hash_func, kh_int_hash_equal)

#define KHASH_MAP_INIT_INT64(name, khval_t)								\
	KHASH_INIT(name, khint64_t, khval_t, 1, kh_int64_hash_func, kh_int64_hash_equal)

#define KHASH_MAP_INIT_TXT(name, khval_t)								\
	KHASH_INIT(name, kh_cstr_t, khval_t, 1, kh_str_hash_func, kh_str_hash_equal)

#define KHASH_MAP_INIT_U16_ARRAY(name, khval_t)								\
	KHASH_INIT(name, char, khval_t, 1, kh_fred_hash_func, kh_fred_hash_equal)


KHASH_MAP_INIT_INT32(hm_u32_u8, unsigned char)
KHASH_MAP_INIT_TXT(hm_txt_u32, unsigned int)
KHASH_MAP_INIT_TXT(hm_txt_typenum, unsigned short)



// ---------------------------------------------------------------------------------------------------------------------
// free fns
// ---------------------------------------------------------------------------------------------------------------------


typedef struct {
    PyObject_HEAD                   // ob_refcnt:Py_ssize_t, *ob_type:PyTypeObject
} Fred;


typedef struct {
    PyObject_VAR_HEAD                   // ob_refcnt:Py_ssize_t, *ob_type:PyTypeObject
} Joe;


pvt PyObject * _sizeofFredJoe(PyObject *mod, PyObject *const *args, Py_ssize_t nargs) {
    if (nargs != 0) return _raiseWrongNumberOfArgs(__FUNCTION__, 0, nargs);
    return PyTuple_Pack(2, PyLong_FromLong((long) sizeof(Fred)), PyLong_FromLong((long) sizeof(Joe)));
}


pvt PyObject * _execShell(PyObject *mod, PyObject *args) {
    const char *command;  int ret;
    if (!PyArg_ParseTuple(args, "s", &command)) return 0;
    ret = system(command);
    if (ret < 0) {
        PyErr_SetString(JonesError, "System command failed");
        return 0;
    }
    return PyLong_FromLong(ret);
}



#endif  // __JONES_PLAY_C