#pragma once

#if !defined(TARGET_OS_ANDROID) && !defined(TARGET_CPU_ARM64)
# include <NlcDependLibrariesConfig.h>
#endif // !defined(TARGET_OS_ANDROID)

#include "x264_config.h"

#if defined(TARGET_OS_ANDROID) 
# include "config_x264_android.h"
#elif defined(TARGET_OS_WINDOWS)
# include "config_x264_w64.h"
#else
# if defined(TARGET_CPU_ARM64)
#  include "config_x264_aarch64.h"
# else
#  include "config_x264_linux.h"
# endif // 
#endif // defined(TARGET_OS_ANDROID)
