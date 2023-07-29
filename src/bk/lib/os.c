// ---------------------------------------------------------------------------------------------------------------------
// Copyright 2025 David Briant, https://github.com/coppertop-bones. Licensed under the Apache License, Version 2.0
// ---------------------------------------------------------------------------------------------------------------------

#ifndef SRC_BK_LIB_OS_C
#define SRC_BK_LIB_OS_C "bk/os.c"

#if defined _WIN64 || defined _WIN32
#include "os_win64.c"
#elif defined _APPLE_ || defined __MACH__
#include "os_macos.c"
#elif defined __linux__
#include "os_linux.c"
#endif

#endif  // SRC_BK_LIB_OS_C



