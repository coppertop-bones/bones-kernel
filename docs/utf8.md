to start with we'll just use null terminated / size prefixed utf8 as our string format - we shouldn't need to worry 
very much about unicode as we can convert classes and mix types and classes with ease

https://docs.python.org/3/c-api/unicode.html#utf-8-codecs
PyObject *PyUnicode_DecodeUTF8Stateful(char *s, Py_ssize_t size, char *errors, Py_ssize_t *consumed)
char *PyUnicode_AsUTF8AndSize(PyObject *unicode, Py_ssize_t *size)

This decision is backed up by
https://utf8everywhere.org/
https://developer.twitter.com/en/docs/counting-characters


what about fprintf etc? and the formatting dsl?
https://www.cprogramming.com/tutorial/unicode.html
https://www.haible.de/bruno/packages-libutf8.html
https://icu.unicode.org/
https://home.unicode.org/
https://unicode-org.github.io/icu-docs/apidoc/released/icu4c/
https://www.linkedin.com/pulse/mastering-unicode-modern-c-comprehensive-guide-wide-characters-tariq/


