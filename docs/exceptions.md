Consider getting a pointer for a symbol name. If I pass an invalid id what should the behaviour be?

If I have to test the bounds of the id and return an error code from bk then every caller has to check the return code.
This becomes inefficient in general. Solutions:

- 2 apis the checked api and an unchecked api
- don't check - pushes knowledge onto the client and makes api less secure
- exceptions - Itanium exception ABI, setjump API, report and exit(1)
- have a checking api, e.g. `if (precheck_sm_name(id) == FAIL) {handleIt();} char *name = sm_name.(id);` which is slower
  but allows for debug mode

the more I think about it the more I think exceptions are the way to go - ideally zero cost but setjmp is next best


