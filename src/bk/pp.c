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
};
int g_logging_level = info;     // OPEN: add filter as well as level?


// define die_ in client
pvt void die_(char *preamble, char *msg, va_list args);
//{
//    fprintf(stderr, "%s", preamble);
//    vfprintf(stderr, msg, args);
//    exit(1);
//}

pvt void PP(int level, char *msg, ...) {
    if (level & g_logging_level) {
        va_list args;
        va_start(args, msg);
        vfprintf(stderr, msg, args);
        fprintf(stderr, "\n");
        va_end(args);
    }
}

pvt void onOomDie(void *p, char *msg, ...) {
    if (p == 0) {
        va_list args;
        va_start(args, msg);
        die_("", msg, args);
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

#endif  // __BK_PP_C