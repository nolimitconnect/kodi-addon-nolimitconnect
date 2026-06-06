
cmake_minimum_required (VERSION 3.22.1)

project(lame, LANGUAGES C CXX)

set(lame_SOURCES)

list(APPEND lame_SOURCES 
    libmp3lame/VbrTag.c
    libmp3lame/gain_analysis.c
    libmp3lame/id3tag.c
    libmp3lame/lame.c
    libmp3lame/mpglib_interface.c
    libmp3lame/newmdct.c
    libmp3lame/presets.c
    libmp3lame/psymodel.c
    libmp3lame/quantize_pvt.c
    libmp3lame/reservoir.c
    libmp3lame/set_get.c
    libmp3lame/takehiro.c
    libmp3lame/vbrquantize.c
    libmp3lame/vector/xmm_quantize_sub.c
    mpglib/dct64_i386.c
    mpglib/decode_i386.c
    mpglib/interface.c
    mpglib/layer1.c
    mpglib/layer2.c
    mpglib/layer3.c
    mpglib/tabinit.c
    libmp3lame/bitstream_lame.c
    libmp3lame/encoder_lame.c
    libmp3lame/fft_lame.c
    libmp3lame/util_lame.c
    libmp3lame/version_lame.c
    libmp3lame/quantize_lame.c
    libmp3lame/tables_lame.c
    mpglib/common_lame.c
)

add_library(lame STATIC ${lame_SOURCES})

nlc_target_add_supported_warning_options(lame C
    -Wno-shift-negative-value
    -Wno-absolute-value
    -Wno-tautological-constant-out-of-range-compare
    -Wno-unused-value
    -Wno-tautological-pointer-compare
)

if(DEFINED TARGET_OS_WINDOWS)
    target_compile_definitions(lame
      PRIVATE
        _WINDOWS
        HAVE_MPGLIB
        HAVE_CONFIG_H
    )
endif()

if(DEFINED TARGET_OS_LINUX)
    target_compile_definitions(lame
      PRIVATE
        HAVE_MPGLIB
        HAVE_CONFIG_H
    )
endif()

if(DEFINED TARGET_OS_ANDROID)
    target_compile_definitions(lame
      PRIVATE
        HAVE_MPGLIB
        HAVE_CONFIG_H
    )
endif()

target_include_directories(lame
  PRIVATE
    mpglib
    include
    libmp3lame
)


