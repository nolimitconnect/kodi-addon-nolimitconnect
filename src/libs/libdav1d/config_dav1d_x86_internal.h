

#pragma once

#define ARCH_AARCH64 0
#define ARCH_ARM 0

#define ARCH_PPC64LE 0

#define ARCH_X86 1
#define ARCH_X86_32 0
#define ARCH_X86_64 1

#define CONFIG_16BPC 1
#define CONFIG_8BPC 1
#define CONFIG_LOG 1

#define ENDIANNESS_BIG 0

#define HAVE_ASM 1
#define HAVE_AS_FUNC 0
#define HAVE_C11_GENERIC 1
#define HAVE_CLOCK_GETTIME 1

#define HAVE_PTHREAD_GETAFFINITY_NP 1
#define HAVE_PTHREAD_SETAFFINITY_NP 1
#define TRIM_DSP_FUNCTIONS 0

#if defined(TARGET_OS_WINDOWS)
# define HAVE_UNISTD_H 0
//# define HAVE_MEMALIGN 0 // do not even define
//# define HAVE_POSIX_MEMALIGN 1 // do not even define
# define HAVE_ALIGNED_MALLOC
# define HAVE_DLSYM 1

#else
# define HAVE_UNISTD_H 1
# define HAVE_MEMALIGN 1
# define HAVE_POSIX_MEMALIGN 1 

#endif // defined(TARGET_OS_WINDOWS)

