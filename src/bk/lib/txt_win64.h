#ifndef __BK_LIB_TXT_WIN64_H
#define __BK_LIB_TXT_WIN64_H "bk/lib/txt_win64.h"


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


#endif      // __BK_LIB_TXT_WIN64_H
