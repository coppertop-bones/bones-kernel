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


#ifndef __BK_LIB_TXT_WIN64_C
#define __BK_LIB_TXT_WIN64_C "bk/lib/txt_win64.c"

#include "../bk.c"

#define INIT_SZ 128

#ifndef VA_COPY
    #ifdef HAVE_VA_COPY
        #define VA_COPY(dest, src) va_copy(dest, src)
    #else
        #ifdef HAVE___VA_COPY
            #define VA_COPY(dest, src) __va_copy(dest, src)
        #else
            #define VA_COPY(dest, src) (dest) = (src)
        #endif
    #endif
#endif


pvt int vasprintf(char **str, char *fmt, va_list ap) {
    int ret;  va_list ap2;  char *string, *newstr;  size_t len;

    if ((string = malloc(INIT_SZ)) == 0) goto fail;

    VA_COPY(ap2, ap);
    ret = vsnprintf(string, INIT_SZ, fmt, ap2);
    va_end(ap2);
    if (ret >= 0 && ret < INIT_SZ) { /* succeeded with initial alloc */
        *str = string;
    } else if (ret == INT_MAX || ret < 0) { /* Bad length */
        free(string);
        goto fail;
    } else {    /* bigger than initial, realloc allowing for nul */
        len = (size_t) ret + 1;
        if ((newstr = realloc(string, len)) == 0) {
            free(string);
            goto fail;
        }
        VA_COPY(ap2, ap);
        ret = vsnprintf(newstr, len, fmt, ap2);
        va_end(ap2);
        if (ret < 0 || (size_t) ret >= len) { /* failed with realloc'ed string */
            free(newstr);
            goto fail;
        }
        *str = newstr;
    }
    return (ret);

    fail:
    *str = 0;
    errno = ENOMEM;
    return (-1);
}

pvt int asprintf(char **str, char *fmt, ...) {
    va_list ap; int ret;

    *str = 0;
    va_start(ap, fmt);
    ret = vasprintf(str, fmt, ap);
    va_end(ap);

    return ret;
}


#endif      //__BK_LIB_TXT_WIN64_C
