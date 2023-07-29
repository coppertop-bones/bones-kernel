// ---------------------------------------------------------------------------------------------------------------------
// Copyright 2025 David Briant, https://github.com/coppertop-bones. Licensed under the Apache License, Version 2.0
// ---------------------------------------------------------------------------------------------------------------------

#ifndef SRC_BK_LIB_TXT_C
#define SRC_BK_LIB_TXT_C "bk/lib/txt.c"

#include <string.h>
#include <stdio.h>
#include "../bk.c"


#if defined _WIN64 || defined _WIN32
#include "txt_win64.c"
#elif defined _APPLE_ || defined __MACH__
#elif defined __linux__
#endif

pvt char * join_txts(int num_args, ...) {
    size_t size = 0;
    va_list ap;
    va_start(ap, num_args);
    for (int i = 0; i < num_args; i++) size += strlen(va_arg(ap, char*));
    char *res = malloc((size)+1);
    size = 0;
    va_start(ap, num_args);
    for (int i = 0; i < num_args; i++) {
        char *s = va_arg(ap, char*);
        strcpy(res + size, s);
        size += strlen(s);
    }
    va_end(ap);
    res[size] = '\0';
    return res;
}

char * concatMsg(char *str1, char *str2){
    char* result;
    asprintf(&result, "%s%s", str1, str2);
    return result;
}


#endif  // SRC_BK_LIB_TXT_C
