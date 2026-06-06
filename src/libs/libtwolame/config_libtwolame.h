#pragma once
#include <NlcDependLibrariesConfig.h>
#include <CoreLib/AppVersion.h>

#if !defined(MIN)
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))
#endif // !defined(MIN)

#if !defined(MAX)
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))
#endif // !defined(MAX)

#ifdef _WIN32
# include <libtwolame/config_libtwolamewin.h>
#endif // _WIN32



