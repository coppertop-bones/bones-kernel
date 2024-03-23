# Abreviations

pb - print buckets - return void
pp - print pad - return a text pad node
s8 - print s8 - return an s8


# "Class" functions
capitalise the type, e.g.
```c
pub void TP_init(BK_TP *tp, size initSz, Buckets *buckets) {
```


# "Instance" functions
lower case the type
```c
pub TPN tp_pp_printf(BK_TP *tp, char const *format, ...) 
```


# Casing

PascalCase for agents - e.g. typedef struct TP {} TP
camelCase and lower_snake_case for names
value structs - 


Use struct wrappers for strong typing - https://floooh.github.io/2019/09/27/modern-c-for-cpp-peeps.html


# Reserved Names
On leading underscores and names reserved by the C and C++ languages by Raymond Chen - https://devblogs.microsoft.com/oldnewthing/20230109-00/?p=107685




https://nullprogram.com/blog/2023/10/08/


https://nelkinda.com/blog/suppress-warnings-in-gcc-and-clang/#d12e376





- always use signed ints so differences can be taken, unsigned are only ever needed for debugging


Quote Originally Posted by C99 Section 6.2.5 Paragraph 15 and note 35 \
The three types char, signed char, and unsigned char are collectively called the character types. The
implementation shall define char to have the same range, representation, and behavior as either signed
char or unsigned char.

CHAR_MIN, defined in <limits.h>, will have one of the values 0 or SCHAR_MIN, and this can be used to
distinguish the two options. Irrespective of the choice made, char is a separate type from the other
two and is not compatible with either.

consider:
strcmp - e.g. https://codebrowser.dev/glibc/glibc/string/strcmp.c.html


size_t - the type which is used to represent the size of objects in bytes, and returned from sizeof, e.g.
https://www.embedded.com/further-insights-into-size_t/


https://embeddedgurus.com/stack-overflow/2008/06/efficient-c-tips-1-choosing-the-correct-integer-size/


## MACROS
\# - single hash in macro puts the argument in quotes \
\## creates a new symbol \
https://gcc.gnu.org/onlinedocs/cpp/Concatenation.html#Concatenation

## X MACROS
https://www.digitalmars.com/articles/b51.html \
https://en.wikipedia.org/wiki/X_Macro




//#define IFNDEF \#ifndef
//#define DEFINE \#define
//#define INCLUDE \#include
//#define ENDIF \#endif
//
//
//#define J_INCLUDE(file_locator, guard_symbol) \
//    IFNDEF guard_symbol \
//    INCLUDE file_locator \
//    DEFINE guard_symbol \
//    ENDIF



