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
pvt void die_(char const *preamble, char const *msg, va_list args);
//{
//    fprintf(stderr, "%s", preamble);
//    vfprintf(stderr, msg, args);
//    exit(1);
//}


// OPEN: could distinguish between void * to an S* and char * by making the first byte of an S8 > 128?
pvt void PP(i32 level, char const *msg, ...) {
    bool log = false;
    if (g_logging_level == debug) log = true;
    else if (g_logging_level == info && (level == info || level == error)) log = true;
    else if (g_logging_level == error && (level == error)) log = true;
    if (log) {
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

pvt void die(char const *msg, ...) {
    va_list args;
    va_start(args, msg);
    die_("", msg, args);
    va_end(args);
}

pvt void nyi(char const *msg, ...) {
    va_list args;
    va_start(args, msg);
    die_("nyi: ", msg, args);
    va_end(args);
}

#if defined _WIN64 || defined _WIN32
#else
_Pragma("GCC diagnostic push")
_Pragma("GCC diagnostic ignored \"-Wunused-function\"")
#endif

pvt void bug(char const *msg, ...) {
    va_list args;
    va_start(args, msg);
    die_("bug: ", msg, args);
    va_end(args);
}

pvt void check(bool truth, char const *msg, ...) {
    if (! truth) {
        va_list args;
        va_start(args, msg);
        die_("", msg, args);
        va_end(args);
    }
}

#if defined _WIN64 || defined _WIN32
#else
_Pragma("GCC diagnostic pop")
#endif


#endif  // __BK_PP_C