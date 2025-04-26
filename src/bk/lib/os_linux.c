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


#ifndef SRC_BK_LIB_OS_LINUX_C
#define SRC_BK_LIB_OS_LINUX_C "bk/os_linux.c"

#include <stdio.h>
#include "../../../include/bk/bk.h"
#include "../../../include/bk/lib/os.h"
#include "../pp.c"

pub size_t os_cache_line_size() {
    FILE * p = 0;
    p = fopen("/sys/devices/system/cpu/cpu0/cache/index0/coherency_line_size", "r");
    unsigned int lineSize = 0;
    if (p) {
        fscanf(p, "%d", &lineSize);
        fclose(p);
    }
    return lineSize;
}

#endif  // SRC_BK_LIB_OS_LINUX_C
