#pragma once
//============================================================================
// Created by Brett R. Jones in 2017 and issued to public domain
//============================================================================

// Processor architecture detection.For more info on what's defined, see:
//   http://msdn.microsoft.com/en-us/library/b0084kay.aspx
//   http://www.agner.org/optimize/calling_conventions.pdf
//   or with gcc, run: "echo | gcc -E -dM -"

#include "NlcTargetOsConfig.h"

#define NLC_FLOATING_POINT        1 // use floats and don't support fixed point
//#define NLC_FIXED_POINT        1 // support fixed point

//============================================================================
// determine CPU type and number of bits and endianess

#define ARCH_ALPHA						0
#define ARCH_MIPS						0
#define ARCH_M68K						0
#define ARCH_PPC						0
#define ARCH_RISCV						0
#define ARCH_SPARC						0

#if defined(TARGET_OS_WINDOWS)
# define NLC_ARCH_LITTLE_ENDIAN			1

# define ARCH_ARM                       0
# define ARCH_AARCH64                   0
# define ARCH_X86						1

#if defined(WIN64) || defined(__x86_64__) || defined(_WIN64)
// have to check for windows 64 bit first.. WIN32 seems to get defined in unexpected places
# define ARCH_64_BITS                   1
# define ARCH_32_BITS                   0
# define ARCH_X86_64_IS_AVAILABLE		1
# define TARGET_CPU_X64					1  // general cpu type

// some libraries require different defines
# if !defined (__x86_64__)
#  define __x86_64__                    1
# endif // !defined (__x86_64__)
# if !defined (WIN64)
#  define WIN64    
# endif // !defined (WIN64)
# if !defined (_WIN64)
#  define _WIN64    
# endif // !defined (_WIN64)
#else
# define TARGET_CPU_X86					1  // general cpu type

# define ARCH_64_BITS                   0
# define ARCH_32_BITS                   1
# define ARCH_AARCH64                   0
# define ARCH_X86_64_IS_AVAILABLE		0
# if !defined (__i386__)
#  define __i386__                       1
# endif // !defined (__i386__)
#endif

#if ARCH_32_BITS
# define ARCH_X86_32 1
# define ARCH_X86_64 0
#else
# define ARCH_X86_32 0
# define ARCH_X86_64 1
# ifndef __x86_64__
#  define __x86_64__ // needed for libgnu
# endif // __x86_64__
#endif // ARCH_32_BITS
//#endif //defined(_M_X64) || defined(__x86_64__)
// end of TARGET_OS_WINDOWS arch defines


#elif defined(TARGET_OS_ANDROID)
# define NLC_ARCH_LITTLE_ENDIAN			1

# if defined(TARGET_CPU_ARM64) || defined(ARM64)
#  define TARGET_CPU_ARM64            1
#  define ARCH_X86                      0
#  define ARCH_AARCH64                  1
#  define ARCH_ARM                      0

#  define ARCH_32_BITS					0
#  define ARCH_64_BITS					1

#  define ARM_FEATURE_NEON              1
#  define HAVE_ARM_ARCH_V6				1

# elif defined(TARGET_CPU_ARM32) || defined(ARM32)
#  define TARGET_CPU_ARM32				1
#  define ARCH_X86                      0
#  define ARCH_AARCH64                  0
#  define ARCH_ARM                      1

#  define ARCH_32_BITS					1
#  define ARCH_64_BITS					0

#  define ARM_FEATURE_NEON              1
#  define HAVE_ARM_ARCH_V6				1				1

# else // X64 android
#  define TARGET_CPU_X64				1
#  define ARCH_X86                      1
#  define ARCH_ARM                      0

#  define ARCH_32_BITS					0
#  define ARCH_64_BITS					1

#  define ARM_FEATURE_NEON              0
#  define HAVE_ARM_ARCH_V6				0

# endif // defined(__ARMEL__)

# define ARCH_ALPHA		                0
# define ARCH_ALPHA                     0
# define ARCH_SPARC                     0
# define ARCH_MIPS                      0
# define ARCH_M68K                      0
# define ARCH_PPC                       0


#elif defined(TARGET_OS_LINUX)

# define NLC_ARCH_LITTLE_ENDIAN         1
# define ARCH_ALPHA                     0
# define ARCH_SPARC                     0
# define ARCH_MIPS                      0
# define ARCH_M68K                      0
# define ARCH_PPC                       0
# define ARCH_ALPHA		                0
# define ARCH_ARM                       0

// only 64bit linux is supported
# define ARCH_32_BITS					0
# define ARCH_64_BITS                   1

# if defined(TARGET_CPU_ARM64) || defined(__aarch64__) || defined(_M_ARM64)
#  define TARGET_CPU_ARM64            1
#  define ARCH_AARCH64                1
# else
#  define TARGET_CPU_X64			    1  // general cpu type
#  define ARCH_X86                      1
#  define ARCH_AARCH64                  0
# endif // defined(TARGET_CPU_ARM64)


#elif defined(TARGET_OS_APPLE)
echo error apple and ppc processors not supported
# define NLC_ARCH_LITTLE_ENDIAN			0
# define TARGET_CPU_PPC					1 // general cpu type

# define ARCH_ALPHA		                0
# define ARCH_ARM                       0
# define AARCH64                        0 // the other arm type android cpu
# define ARCH_X86						0
# define ARCH_ALPHA                     0
# define ARCH_SPARC                     0
# define ARCH_MIPS                      0
# define ARCH_M68K                      0
# define ARCH_PPC                       1

# define ARCH_32_BITS					0
# define ARCH_64_BITS                   1

#else // no os defined
//============================================================================
// not supported.. try to give reasonable error
#if defined(__MIPSEL__)
echo error mips processors not supported
# define NLC_ARCH_LITTLE_ENDIAN				1
# define TARGET_CPU_MIPS					1
# define ARCH_64_BITS						1
# define ARCH_MIPS                          0

#elif defined(TARGET_OS_APPLE) || ARCH_PPC
echo error ppc processors not supported
# define NLC_ARCH_LITTLE_ENDIAN				0

#elif defined(__aarch64__)
echo error aarch64 processors not supported
# define NLC_ARCH_LITTLE_ENDIAN				1
# define NLC_ARCH_BIG_ENDIAN				0
# define TARGET_CPU_ARM64			1
# define ARCH_AARCH64				1
# define ARCH_64_BITS						1
#else
echo unknown processor types are not supported
#endif // error type for unsupported processor
#endif // defined(TARGET_OS_APPLE)

#if !ARCH_X86 && !ARCH_ARM && !ARCH_AARCH64
# define NLC_ARCH_LITTLE_ENDIAN				1
echo Nlc CPU Arch Defines error no cpu target defined
//#define ARCH_AARCH		1
//#define ARCH_ALPHA		1
//#define ARCH_AVR32		1
//#define ARCH_AVR32_AP		1
//#define ARCH_AVR32_UC		1
//#define ARCH_BFIN			1
//#define ARCH_M68K			1
//#define ARCH_PARISC		1
//#define ARCH_S390			1
//#define ARCH_SH4			1
//#define ARCH_SPARC		1
//#define ARCH_SPARC64		1
//#define ARCH_TILEGX		1
//#define ARCH_TILEPRO		1
//#define ARCH_TOMI			1
/* Defined for Alpha platforms */
/* #undef HAVE_CPU_ARCH_ALPHA */
/* Defined for M68k platforms */
/* #undef HAVE_CPU_ARCH_M68K */
/* Defined for MIPS platforms */
/* #undef HAVE_CPU_ARCH_MIPS */
/* Defined for PPC platforms */
/* #undef HAVE_CPU_ARCH_PPC */
/* Defined for SPARC platforms */
/* #undef HAVE_CPU_ARCH_SPARC */

#endif // !ARCH_X86 && !ARCH_ARM

#if !defined(TARGET_CPU_ARM32) && !defined(TARGET_CPU_ARM64)
#if ARCH_32_BITS
# define ARCH_X86_32 1
# define ARCH_X86_64 0
#else
# define ARCH_X86_32 0
# define ARCH_X86_64 1
# ifndef __x86_64__
#  define __x86_64__ // needed for libgnu
# endif // __x86_64__
#endif // ARCH_32_BITS
#endif // !TARGET_CPU_ARM32

//============================================================================
//============================================================================
//=== cpu features ===//
#if defined(TARGET_CPU_ARM64)
# define HAVE_ARM_NEON				1 // both arm68-v8a and armeabi-v7a have neon extensions
# define HAVE_ARMV7					1

#  define ARCH_32_BITS				0
#  define ARCH_64_BITS				1

#  define HAVE_ARMV8				1
#  define HAVE_ARMV5TE				0
#  define HAVE_ARMV6				0
#  define HAVE_ARMV6T2				0

#elif defined(TARGET_CPU_X86) || defined(TARGET_CPU_X64)
# if defined(TARGET_CPU_X64)
// less work to define TARGET_CPU_X86 also and then check for TARGET_CPU_X64 if needed
#  define TARGET_CPU_X86 1
#  define ARCH_32_BITS 0
#  define ARCH_64_BITS 1
# elif !defined(ARCH_64_BITS) // if not previously defined for TARGET_OS_ANDROID
#  define ARCH_32_BITS 1
#  define ARCH_64_BITS 0
# endif // defined(TARGET_CPU_X86)

# if !defined(NLC_ARCH_LITTLE_ENDIAN)
  echo error NLC_ARCH_LITTLE_ENDIAN  is not defined
# else
#  if NLC_ARCH_LITTLE_ENDIAN // has been set to 1
// just leave HAVE_BIGENDIAN undefined
#  else
#  define HAVE_BIGENDIAN // is a big endian cpu
#  endif // NLC_ARCH_LITTLE_ENDIAN
# endif // !defined(NLC_ARCH_LITTLE_ENDIAN)

// ONLY INTEL HAS AVX ?
# define HAVE_AVX 1
# define HAVE_AVX2 1

# define HAVE_I686					1
# define HAVE_MMX					1
# define HAVE_MMXEXT				1
# define HAVE_SSE					1
# define HAVE_SSE2					1
//# define HAVE_AMD3DNOW			1
//# define HAVE_AMD3DNOWEXT			1
//# define HAVE_MMX					1
//# define HAVE_SSE3				1
//# define HAVE_SSE4				1
//# define HAVE_SSE42				1
//# define HAVE_SSSE3				1
//# define HAVE_XOP					1
//# define HAVE_CPUNOP				0

//==== PPC architectures ===//
#elif ARCH_PPC
// For PPC only
echo Nlc CPU Config error ppc processors not supported

//==== MIPS architectures ===//
#elif ARCH_MIPS
// For mips only
echo Nlc CPU Config error mips processors not supported
//#define HAVE_MIPSDSP				1
//#define HAVE_MIPSDSPR2			1
//#define HAVE_MIPSFPU				1
//#define HAVE_MIPS32R2				1
//#define HAVE_MIPS32R5				1
//#define HAVE_MIPS64R2				1
//#define HAVE_MIPS32R6				1
//#define HAVE_MIPS64R6				1

#elif defined(TARGET_OS_ANDROID)
//==== arm architectures ===//
# if TARGET_CPU_ARM32
#  define HAVE_SSE           0
#  define HAVE_SSE2          0
# else
#  define HAVE_SSE           1
#  define HAVE_SSE2          1
# endif // TARGET_CPU_ARM32

# define HAVE_ARM_NEON				1 // both arm68-v8a and both armeabi-v7a have neon extensions
# define HAVE_ARMV7					1

# if defined(TARGET_CPU_ARM64)
#  define ARCH_32_BITS				0
#  define ARCH_64_BITS				1

#  define HAVE_ARMV8				1
#  define HAVE_ARMV5TE				0
#  define HAVE_ARMV6				0
#  define HAVE_ARMV6T2				0

# else
#  define ARCH_32_BITS				1
#  define ARCH_64_BITS				0

#  define HAVE_ARMV8				0
#  define HAVE_ARMV5TE				1
#  define HAVE_ARMV6				1
#  define HAVE_ARMV6T2				1
# endif // defined(TARGET_CPU_ARM64)

#else
echo Nlc CPU Config error no cpu arc defined.. unknown processors not supported
// don't know what these are

//#define HAVE_VFP				1
//#define HAVE_VFPV3			1
//#define HAVE_SETEND			1
//#define HAVE_ALTIVEC			1
//#define HAVE_DCBZL			1
//#define HAVE_LDBRX			1
//#define HAVE_VSX				1
//#define HAVE_AESNI			1
//#define HAVE_FMA3				1
//#define HAVE_FMA4				1

//#define HAVE_MSA				1
//#define HAVE_LOONGSON2		1
//#define HAVE_LOONGSON3		1

#endif // CPU FEATURES

#if HAVE_SSE && !defined(__SSE__)
/* Enable SSE functions, if compiled with SSE/SSE2 (All intel/AMD x86 cpus since 2003) */
# define __SSE__ 1
#endif // HAVE_SSE

#if HAVE_SSE2 && !defined(__SSE2__)
/* Enable SSE functions, if compiled with SSE/SSE2 (All intel/AMD x86 cpus since 2003) */
# define __SSE2__ 1
#endif // HAVE_SSE

#if !ARCH_64_BITS && !ARCH_32_BITS
	@echo either ARCH_64_BITS or ARCH_32_BITS must be defined
#endif // !defined(ARCH_64_BITS) && !defined(ARCH_32_BITS)

	
#if ARCH_32_BITS
# define SIZEOF_UNSIGNED_LONG_INT 4
# define SIZEOF_UNSIGNED_INT 4
# define SIZEOF_INT 4
# define SIZEOF_LONG 4 
# define SIZEOF_VOID_P 4
# define SIZEOF_CHAR_P 4

#elif ARCH_64_BITS

# if defined(TARGET_OS_WINDOWS) && defined(_MSC_VER)
#  define SIZEOF_LONG 4 // microsoft long is 4 bytes even in 64 bit builds 
#  define SIZEOF_UNSIGNED_LONG_INT 4
# else
#  define SIZEOF_LONG 8 // everybody else uses long size the same as arch size
#  define SIZEOF_UNSIGNED_LONG_INT 8
# endif // defined(TARGET_OS_WINDOWS) && defined(_MSC_VER)

// oddly int is always 4 bytes even on 64 bit builds on windows and linux and android
# define SIZEOF_UNSIGNED_INT 4
# define SIZEOF_INT 4

# define SIZEOF_VOID_P 8
# define SIZEOF_CHAR_P 8

#else
 echo either ARCH_64_BITS or ARCH_32_BITS must be defined
#endif // ARCH_32_BITS

/* The size of `short', as computed by sizeof. */
#define SIZEOF_SHORT 2
/* The size of `char', as computed by sizeof. */
#define SIZEOF_CHAR 1
/* The size of `long long', as computed by sizeof. */
#define SIZEOF_LONG_LONG 8
/* The size of `__int64', as computed by sizeof. */
#define SIZEOF___INT64 8





