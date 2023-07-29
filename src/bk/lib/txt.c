// ---------------------------------------------------------------------------------------------------------------------
//
//                             Copyright (c) 2019-2025 David Briant. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance
// with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software distributed under the License is distributed
// on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for
// the specific language governing permissions and limitations under the License.
//
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
