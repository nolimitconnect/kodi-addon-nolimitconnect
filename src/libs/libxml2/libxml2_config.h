#pragma once

#include <NlcDependLibrariesConfig.h>

/* Version number of package */
#define VERSION "2.7.6"
/* Name of package */
#define PACKAGE "libxml2"

#if defined(TARGET_OS_WINDOWS)
# include "libxml2_config_windows.h"
#elif defined(TARGET_OS_ANDROID)
# include "libxml2_config_linux.h"
#elif defined(TARGET_OS_LINUX)
# include "libxml2_config_linux.h"
#else
#error libxml2 no os defined
#endif // TARGET_OS_WINDOWS

