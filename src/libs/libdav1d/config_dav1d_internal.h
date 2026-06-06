

#pragma once

#if defined(TARGET_CPU_ARM64)
# if defined(FLATPAKBUILD)
#  include "config_dav1d_aarch64_internal.h"
# else
#  include "config_dav1d_arm_internal.h"
# endif
#elif defined(TARGET_CPU_ARM32)
#include "config_dav1d_arm_internal.h"
#else
#include "config_dav1d_x86_internal.h"
#endif // defined(TARGET_CPU_ARM32)



