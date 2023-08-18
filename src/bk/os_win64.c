#ifndef __BK_OS_WIN64_C
#define __BK_OS_WIN64_C "bk/os_win64.c"


#include <errno.h>
#include <limits.h> /* for INT_MAX */
#include <stdarg.h>
#include <stdio.h>          /* for vsnprintf */
#include <stdlib.h>
#include <windows.h>
#include "../../include/bk/bk.h"

export int os_page_size() {
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    return si.dwPageSize;
}


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


export int os_cache_line_size() {
    size_t lineSize = 0;
    DWORD bufferSize = 0;
    DWORD i = 0;
    SYSTEM_LOGICAL_PROCESSOR_INFORMATION * buffer = 0;

    GetLogicalProcessorInformation(0, &bufferSize);
    buffer = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION *) malloc(bufferSize);
    GetLogicalProcessorInformation(&buffer[0], &bufferSize);

    for (i = 0; i != bufferSize / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION); ++i) {
        if (buffer[i].Relationship == RelationCache && buffer[i].Cache.Level == 1) {
            lineSize = buffer[i].Cache.LineSize;
            break;
        }
    }

    free(buffer);
    return lineSize;
}


#define INIT_SZ 128

pvt int vasprintf(char **str, const char *fmt, va_list ap) {
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

pvt int asprintf(char **str, const char *fmt, ...) {
    va_list ap; int ret;

    *str = 0;
    va_start(ap, fmt);
    ret = vasprintf(str, fmt, ap);
    va_end(ap);

    return ret;
}


#endif  // __BK_OS_WIN64_C