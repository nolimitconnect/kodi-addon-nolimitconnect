#pragma once

#if defined(TARGET_CPU_ARM64) ||  defined(TARGET_CPU_ARM32)
# include "jconfig_arm.h"
#else
# include "jconfig_x64.h"
#endif
