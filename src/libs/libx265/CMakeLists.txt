
cmake_minimum_required (VERSION 3.22.1)

project(x265, LANGUAGES C CXX ASM)

if(DEFINED TARGET_OS_ANDROID)
    #android/aarch64 uses clang as assembler
elseif(DEFINED TARGET_OS_WINDOWS)
    enable_language(ASM_NASM)
else()
    if(NOT DEFINED TARGET_CPU_ARM64)
        enable_language(ASM_NASM)
    endif()
endif()

set(x265_SOURCES)

if(DEFINED TARGET_CPU_X64)
    list(APPEND x265_SOURCES 
        common/x86/asm-primitives.cpp
        common/x86/blockcopy8.asm
        common/x86/const-a.asm
        common/x86/cpu-a.asm
        common/x86/dct8.asm
        #common/x86/h-ipfilter16.asm
        common/x86/h-ipfilter8.asm
        #common/x86/intrapred16.asm
        common/x86/intrapred8.asm
        common/x86/intrapred8_allangs.asm
        #common/x86/ipfilter16.asm
        common/x86/ipfilter8.asm
        common/x86/loopfilter.asm
        common/x86/mc-a.asm
        common/x86/mc-a2.asm
        #common/x86/pixel-32.asm
        common/x86/pixel-a.asm
        common/x86/pixel-util8.asm
        common/x86/pixeladd8.asm
        common/x86/sad-a.asm
        #common/x86/sad16-a.asm
        common/x86/seaintegral.asm
        common/x86/ssd-a.asm
        #common/x86/v4-ipfilter16.asm
        common/x86/v4-ipfilter8.asm
        # common/x86/x86util.asm // included by others
        # common/x86/x86inc.asm // included by others
    )
endif()

if(DEFINED TARGET_CPU_ARM64)
    list(APPEND x265_SOURCES
        common/aarch64/arm64-utils.cpp
        common/aarch64/asm-primitives.cpp
        common/aarch64/blockcopy8.S
        common/aarch64/dct-prim.cpp
        common/aarch64/filter-prim.cpp
        common/aarch64/intrapred-prim.cpp
        common/aarch64/ipfilter.S
        common/aarch64/loopfilter-prim.cpp
        common/aarch64/mc-a.S
        common/aarch64/p2s.S
        common/aarch64/pixel-prim.cpp
        common/aarch64/pixel-util.S
        common/aarch64/sad-a.S
        common/aarch64/ssd-a.S
        # common/aarch64/asm.S // included by others
    )
elseif(DEFINED TARGET_CPU_ARM32)
    list(APPEND x265_SOURCES
        common/arm/asm-primitives.cpp
        common/arm/blockcopy8.S
        common/arm/cpu-a.S
        common/arm/dct-a.S
        common/arm/ipfilter8.S
        common/arm/mc-a.S
        common/arm/pixel-util.S
        common/arm/sad-a.S
        common/arm/ssd-a.S
        # common/arm/asm.S // included by others
    )

endif()



list(APPEND x265_SOURCES 
    common/bitstream_x265.cpp
    common/common_x265.cpp
    common/constants_x265.cpp
    common/cpu_x265.cpp
    common/cudata_x265.cpp
    common/dct_x265.cpp
    common/deblock_x265.cpp
    common/framedata_x265.cpp
    common/frame_x265.cpp
    common/intrapred_x265.cpp
    common/ipfilter_x265.cpp
    common/loopfilter_x265.cpp
    common/lowpassdct_x265.cpp
    common/lowres_x265.cpp
    common/md5_x265.cpp
    common/param_x265.cpp
    common/piclist_x265.cpp
    common/picyuv_x265.cpp
    common/pixel_x265.cpp
    common/predict_x265.cpp
    common/primitives_x265.cpp
    common/quant_x265.cpp
    common/ringmem_x265.cpp
    common/scaler_x265.cpp
    common/scalinglist_x265.cpp
    common/shortyuv_x265.cpp
    common/slice_x265.cpp
    common/temporalfilter_x265.cpp
    common/threading_x265.cpp
    common/threadpool_x265.cpp
    common/version_x265.cpp
    common/wavefront_x265.cpp
    common/winxp_x265.cpp
    common/yuv_x265.cpp
    encoder/analysis_x265.cpp
    encoder/api_x265.cpp
    encoder/bitcost_x265.cpp
    encoder/dpb_x265.cpp
    encoder/encoder_x265.cpp
    encoder/entropy_x265.cpp
    encoder/frameencoder_x265.cpp
    encoder/framefilter_x265.cpp
    encoder/level_x265.cpp
    encoder/motion_x265.cpp
    encoder/nal_x265.cpp
    encoder/ratecontrol_x265.cpp
    encoder/reference_x265.cpp
    encoder/sao_x265.cpp
    encoder/search_x265.cpp
    encoder/sei_x265.cpp
    encoder/slicetype_x265.cpp
    encoder/weightPrediction_x265.cpp
    #   x265.cpp
)

if(NOT DEFINED TARGET_OS_ANDROID)
    foreach(x265_src IN LISTS x265_SOURCES)
        if(x265_src MATCHES "\\.asm$")
            set_source_files_properties(${x265_src} PROPERTIES LANGUAGE ASM_NASM)
        endif()
    endforeach()
endif()

add_library(x265 STATIC ${x265_SOURCES})

if(DEFINED TARGET_OS_LINUX AND DEFINED TARGET_CPU_ARM64)
    message(STATUS "Adding x265 flags -flax-vector-conversions")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flax-vector-conversions")
endif()

target_compile_definitions(x265 PUBLIC 
    __LITTLE_ENDIAN__=1
    HIGH_BIT_DEPTH=0
    BIT_DEPTH=8
    EXPORT_C_API=1 
    HAVE_INT_TYPES_H=1 
    X265_DEPTH=8 
    X265_NS=x265 
)

if(DEFINED TARGET_OS_WINDOWS)
    target_include_directories(x265 PRIVATE
        common
        common/x86
        encoder
    )

    target_compile_definitions(x265 PRIVATE
        ARCH_X86_64=1
        X86_64=1
        LIB_STATIC
        WIN64
        __STDC_CONSTANT_MACROS
        _CRT_SECURE_NO_WARNINGS
        _CRT_NONSTDC_NO_DEPRECATE
    )

else()
    if(DEFINED TARGET_CPU_ARM64)
        target_include_directories(x265 PRIVATE
            common
            common/aarch64
            encoder
        )

        target_compile_definitions(x265 PUBLIC  
            PIC
            X265_ARCH_ARM64=1 
        )

    elseif(DEFINED TARGET_CPU_ARM32)
        target_include_directories(x265 PRIVATE
            common
            common/arm
            encoder
        )

        target_compile_definitions(x265 PUBLIC  
            PIC
            X265_ARCH_ARM=1 
        )

    else()
        target_include_directories(x265 PRIVATE
            common
            common/x86
            encoder
        )

        target_compile_definitions(x265 PUBLIC 
            PIC 
            X265_ARCH_X86=1 
            X86_64=1
        )

    endif()
endif()


