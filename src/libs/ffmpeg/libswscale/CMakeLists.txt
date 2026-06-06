
cmake_minimum_required (VERSION 3.22.1)

project(swscale, LANGUAGES C CXX ASM)

if(DEFINED TARGET_OS_ANDROID)
    #android/aarch64 uses clang as assembler
elseif(DEFINED TARGET_OS_WINDOWS)
    enable_language(ASM_NASM)
    string(APPEND CMAKE_ASM_NASM_FLAGS "-Pconfig_ffmpeg_w64.asm ")
else()
    enable_language(ASM_NASM)
    if(DEFINED FLATPAKBUILD)
        set(CMAKE_ASM_NASM_FLAGS "-fPic")
    else()
        string(APPEND CMAKE_ASM_NASM_FLAGS "-Pconfig_ffmpeg_linux.asm ")
    endif() # if(DEFINED FLATPAKBUILD)
endif()

set(swscale_SOURCES)

if(DEFINED TARGET_CPU_X64)
    list(APPEND swscale_SOURCES 
       x86/hscale_fast_bilinear_simd.c
       x86/rgb2rgb.c
       x86/swscale.c
       x86/yuv2rgb.c
       x86/input.asm
       x86/output.asm
       x86/rgb_2_rgb.asm
       x86/scale.asm
       x86/scale_avx2.asm
       x86/yuv_2_rgb.asm
       x86/yuv2yuvX.asm
    )
endif()

if(DEFINED TARGET_CPU_ARM64)
    list(APPEND swscale_SOURCES
       aarch64/rgb2rgb.c
       aarch64/swscale.c
       aarch64/swscale_unscaled.c
       aarch64/hscale.S
       aarch64/output.S
       aarch64/rgb2rgb_neon.S
       aarch64/yuv2rgb_neon.S
    )
elseif(DEFINED TARGET_CPU_ARM32)
    list(APPEND swscale_SOURCES 
       arm/hscale.S
       arm/output.S
       arm/rgb2yuv_neon_16.S
       arm/rgb2yuv_neon_32.S
       arm/swscale.c
       arm/swscale_unscaled.c
       arm/yuv2rgb_neon.S
    )
endif()

list(APPEND swscale_SOURCES 
   alphablend.c
   bayer_template.c
   gamma.c
   hscale.c
   hscale_fast_bilinear.c
   input.c
   log2_tab.c
   options.c
   output.c
   rgb2rgb_swscale.c
   rgb2rgb_template_swscale.c
   slice.c
   swscale_swscale.c
   swscale_unscaled_swscale.c
   utils.c
   vscale.c
   yuv2rgb_swscale.c

)

if(NOT DEFINED TARGET_OS_ANDROID)
    foreach(ffmpeg_src IN LISTS swscale_SOURCES)
        if(ffmpeg_src MATCHES "\\.asm$")
            set_source_files_properties(${ffmpeg_src} PROPERTIES LANGUAGE ASM_NASM)
        endif()
    endforeach()
endif()

add_library(swscale STATIC ${swscale_SOURCES})

target_include_directories(swscale
  PRIVATE
    ../../libcurl/include
    ../../libcurl/lib
    ../../libplayer-nlc
)

target_compile_definitions(swscale
    PRIVATE
    ${ffmpeg_DEFINES}
)

nlc_target_add_supported_warning_options(swscale C
    -Wno-deprecated-declarations
    -Wno-pointer-sign
    -Wno-switch
    -Wno-shift-op-parentheses
    -Wno-dangling-else
)

