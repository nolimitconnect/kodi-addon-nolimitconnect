
cmake_minimum_required (VERSION 3.22.1)

project(rnnoise, LANGUAGES C CXX)


if(DEFINED TARGET_OS_WINDOWS)
    message(STATUS "rnnoise TARGET_OS_WINDOWS is define")
endif()
if(DEFINED TARGET_OS_LINUX)
    message(STATUS "rnnoise TARGET_OS_LINUX is defined")
endif()
if(DEFINED TARGET_OS_ANDROID)
    message(STATUS "rnnoise TARGET_OS_ANDROID is defined")
endif()

if(DEFINED TARGET_CPU_X64)
    message(STATUS "rnnoise TARGET_CPU_X64 is defined")
endif()

if(DEFINED TARGET_CPU_ARM64)
    message(STATUS "rnnoise TARGET_CPU_ARM64 is defined")
endif()

set(rnnoise_SOURCES)

if(DEFINED TARGET_CPU_X64)
    list(APPEND rnnoise_SOURCES
        src/x86/x86_dnn_map.c 
        src/x86/x86cpu.c 
        src/x86/nnet_sse4_1.c 
        src/x86/nnet_avx2.c
    )

endif()

if(DEFINED TARGET_CPU_ARM64)
    # ARM64 placeholder:
    # Add ARM/NEON optimized sources here when ready (for Raspberry Pi 5 or
    # other ARM64 targets). Keep all ARM64-only files in this block.
    # Example future entries:
    #   src/arm/<file1>.c
    #   src/arm/<file2>.c
    list(APPEND rnnoise_SOURCES

    )
endif()

list(APPEND rnnoise_SOURCES
    src/denoise.c 
    src/rnn.c 
    src/pitch.c 
	src/kiss_fft.c 
    src/celt_lpc.c 
    src/nnet.c 
    src/nnet_default.c 
	src/parse_lpcnet_weights.c 
    src/rnnoise_data.c
	src/rnnoise_tables.c 
    RNNoise.cpp
)

set(rnnoise_HEADERS)

list(APPEND rnnoise_HEADERS
    config_rnnoise.h
    include/rnnoise.h
    RNNoise.h
    src/arch.h  
    src/celt_lpc.h  
    src/cpu_support.h 
    src/common.h  
    src/denoise.h 
    src/_kiss_fft_guts.h  
    src/kiss_fft.h  
    src/nnet.h 
    src/nnet_arch.h 
    src/opus_types.h  
    src/pitch.h  
    src/rnn.h  
    src/rnnoise_data.h 
    src/vec_neon.h 
    src/vec.h 
    src/vec_avx.h 
    src/x86/x86_arch_macros.h 
    src/x86/x86cpu.h 
    src/x86/dnn_x86.h

)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config_rnnoise.h
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
    COPYONLY
)

if(NOT DEFINED TARGET_OS_ANDROID)
    foreach(rnnoise_src IN LISTS rnnoise_SOURCES)
        if(rnnoise_src MATCHES "\\.asm$")
            set_source_files_properties(${rnnoise_src} PROPERTIES LANGUAGE ASM_NASM)
        endif()
    endforeach()
endif()

add_library(rnnoise STATIC ${rnnoise_SOURCES} ${rnnoise_HEADERS})

target_include_directories(rnnoise
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    src
    include
)

target_compile_definitions(rnnoise
    PRIVATE
    HAVE_CONFIG_H
)

if(MSVC)
    # Strip default /Od /RTC1 from debug configuration and apply /O2 /RTC-
    string(REPLACE "/Od" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
    string(REPLACE "/Od" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    string(REPLACE "/RTC1" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
    string(REPLACE "/RTC1" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    
    target_compile_options(rnnoise PRIVATE
        $<$<CONFIG:Debug>:/O2 /RTC->
        $<$<CONFIG:Release>:/O2>
    )
else()
    target_compile_options(rnnoise PRIVATE
        $<$<CONFIG:Debug>:-O2>
        $<$<CONFIG:Release>:-O2>
    )
endif()


if(DEFINED TARGET_CPU_X64)
    target_compile_definitions(rnnoise
        PRIVATE
        RNN_ENABLE_X86_RTCD
        CPU_INFO_BY_C
    )

    if(MSVC)
        set_source_files_properties(src/x86/nnet_sse4_1.c PROPERTIES COMPILE_OPTIONS "/arch:AVX2")
        set_source_files_properties(src/x86/nnet_avx2.c PROPERTIES COMPILE_OPTIONS "/arch:AVX2")
        set_source_files_properties(src/x86/nnet_sse4_1.c PROPERTIES COMPILE_DEFINITIONS "OPUS_X86_MAY_HAVE_SSE4_1")
    else()
        set_source_files_properties(src/x86/nnet_sse4_1.c PROPERTIES COMPILE_OPTIONS "-msse4.1")
        set_source_files_properties(src/x86/nnet_avx2.c PROPERTIES COMPILE_OPTIONS "-mavx2")
    endif()
endif()

if(DEFINED TARGET_OS_WINDOWS)

    target_compile_definitions(rnnoise
        PRIVATE
        TARGET_OS_WINDOWS
        LIB_STATIC
        _WIN64
        _LIB
        __STDC_CONSTANT_MACROS
        _CRT_SECURE_NO_WARNINGS
        _CRT_NONSTDC_NO_DEPRECATE
    )

    set_property(TARGET rnnoise PROPERTY C_STANDARD 90)
    set_property(TARGET rnnoise PROPERTY C_STANDARD_REQUIRED OFF)
    set_property(TARGET rnnoise PROPERTY CXX_STANDARD 17)
    set_property(TARGET rnnoise PROPERTY CXX_STANDARD_REQUIRED OFF)
endif()




