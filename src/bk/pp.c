// ---------------------------------------------------------------------------------------------------------------------
// PP - PRETTY PRINTING
// ---------------------------------------------------------------------------------------------------------------------

#ifndef __BK_PP_C
#define __BK_PP_C "bk/pp.c"


#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "../../include/bk/bk.h"


// logging
enum {
    lex = 1,
    parse = 2,
    emit = 4,
    info = 8,
    error = 16,
    pt = 32,        // parse tree
    debug = 64,
};
int g_logging_level = info;     // OPEN: add filter as well as level?


// define die_ in client
pvt void die_(char *preamble, char *msg, va_list args);
//{
//    fprintf(stderr, "%s", preamble);
//    vfprintf(stderr, msg, args);
//    exit(1);
//}


// OPEN: could distinguish between void * to an S* and char * by making the first byte of an S8 > 128?
pvt void PP(i32 level, char *msg, ...) {
    if (level & g_logging_level) {
        va_list args;
        va_start(args, msg);
        FILE *f = level == error ? stderr : stdout;
        vfprintf(f, msg, args);
        fprintf(f, "\n");
        va_end(args);
    }
}

pvt void onOomDie(void *p, S8 msg, ...) {
    if (p == 0) {
        va_list args;
        va_start(args, msg);
        die_("", (char*) msg.cs, args);
        va_end(args);
    }
}

pvt void die(char *msg, ...) {
    va_list args;
    va_start(args, msg);
    die_("", msg, args);
    va_end(args);
}

pvt void nyi(char *msg, ...) {
    va_list args;
    va_start(args, msg);
    die_("nyi: ", msg, args);
    va_end(args);
}

_Pragma("GCC diagnostic push")
_Pragma("GCC diagnostic ignored \"-Wunused-function\"")
pvt void bug(char *msg, ...) {
    va_list args;
    va_start(args, msg);
    die_("bug: ", msg, args);
    va_end(args);
}

pvt void check(bool truth, char *msg, ...) {
    if (! truth) {
        va_list args;
        va_start(args, msg);
        die_("", msg, args);
        va_end(args);
    }
}
_Pragma("GCC diagnostic pop")

#endif  // __BK_PP_C