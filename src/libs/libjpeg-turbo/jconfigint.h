#pragma once

#if defined(TARGET_CPU_ARM64) ||  defined(TARGET_CPU_ARM32)
# include "jconfigint_arm.h"
#else
# include "jconfigint_x64.h"
#endif
