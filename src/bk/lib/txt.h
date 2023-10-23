#ifndef __BK_LIB_TXT_H
#define __BK_LIB_TXT_H "bk/lib/txt.h"


#if defined _WIN64 || defined _WIN32
#include "txt_win64.h"
#elif defined _APPLE_ || defined __MACH__
//#include "os_macos.c"
#elif defined __linux__
//#include "os_linux.c"
#endif

#endif      // __BK_LIB_TXT_H
