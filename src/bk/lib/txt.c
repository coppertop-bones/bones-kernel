#ifndef __BK_LIB_TXT_C
#define __BK_LIB_TXT_C "bk/lib/txt.c"

#include <string.h>
#include <stdio.h>
#include "../bk.h"


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




#endif  // __BK_LIB_TXT_C
