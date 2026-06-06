cmake_minimum_required(VERSION 3.16)
project(webrtc_aec2_apm LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
    BEFORE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(AEC2_CORE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/WebRtcAec.h
    ${CMAKE_CURRENT_SOURCE_DIR}/WebRtcAec.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/typedefs.h
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/common_audio/ring_buffer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/common_audio/ring_buffer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/common_audio/wav_file.h
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/modules/audio_processing/aec/aec_common.h
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/modules/audio_processing/aec/aec_core.c
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/modules/audio_processing/aec/aec_core.h
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/modules/audio_processing/aec/aec_core_internal.h
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/modules/audio_processing/aec/aec_rdft.c
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/modules/audio_processing/aec/aec_rdft.h
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/modules/audio_processing/aec/aec_resampler.c
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/modules/audio_processing/aec/aec_resampler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/modules/audio_processing/aec/echo_cancellation.c
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/modules/audio_processing/aec/echo_cancellation_internal.h
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/modules/audio_processing/aec/include/echo_cancellation.h
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/modules/audio_processing/utility/delay_estimator.c
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/modules/audio_processing/utility/delay_estimator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/modules/audio_processing/utility/delay_estimator_internal.h
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/modules/audio_processing/utility/delay_estimator_wrapper.c
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/modules/audio_processing/utility/delay_estimator_wrapper.h
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/system_wrappers/interface/compile_assert_c.h
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/system_wrappers/interface/cpu_features_wrapper.h
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/system_wrappers/source/cpu_features.cc
)

if(DEFINED TARGET_CPU_X64)
    list(APPEND AEC2_CORE_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/modules/audio_processing/aec/aec_core_sse2.c
        ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/modules/audio_processing/aec/aec_rdft_sse2.c
    )
endif()

if(DEFINED TARGET_CPU_ARM64)
    list(APPEND AEC2_CORE_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/modules/audio_processing/aec/aec_core_neon.c
        ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/modules/audio_processing/aec/aec_rdft_neon.c
    )
endif()

file(GLOB AEC2_SIGNAL_PROCESSING_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/common_audio/signal_processing/*.c
)

# Keep only sources valid for this architecture.
list(FILTER AEC2_SIGNAL_PROCESSING_SOURCES EXCLUDE REGEX ".*_mips\\.c$")
if(NOT DEFINED TARGET_CPU_ARM64)
    list(FILTER AEC2_SIGNAL_PROCESSING_SOURCES EXCLUDE REGEX ".*_neon\\.c$")
endif()

add_library(webrtc-aec2 STATIC
    ${AEC2_CORE_SOURCES}
    ${AEC2_SIGNAL_PROCESSING_SOURCES}
)

if(MSVC)
    # Strip default /Od /RTC1 from debug configuration and apply /O2 /RTC-
    string(REPLACE "/Od" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
    string(REPLACE "/Od" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    string(REPLACE "/RTC1" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
    string(REPLACE "/RTC1" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    
    target_compile_options(webrtc-aec2 PRIVATE
        $<$<CONFIG:Debug>:/O2 /RTC->
        $<$<CONFIG:Release>:/O2>
    )
else()
    target_compile_options(webrtc-aec2 PRIVATE
        $<$<CONFIG:Debug>:-O2>
        $<$<CONFIG:Release>:-O2>
    )
endif()


if(DEFINED TARGET_OS_LINUX OR DEFINED TARGET_OS_ANDROID)
    target_compile_definitions(webrtc-aec2
        PRIVATE
        WEBRTC_LINUX
        WEBRTC_POSIX
        _GNU_SOURCE
    )
endif()

if(DEFINED TARGET_OS_WINDOWS)
    target_compile_definitions(webrtc-aec2
        PRIVATE
        WEBRTC_WIN
        NOMINMAX
    )
endif()

if(DEFINED TARGET_CPU_X64)
    if(NOT MSVC)
        target_compile_options(webrtc-aec2 PRIVATE -msse2)
    endif()
endif()

if(DEFINED TARGET_CPU_ARM64)
    target_compile_definitions(webrtc-aec2
        PRIVATE
        WEBRTC_ARCH_ARM64
    )
endif()

target_include_directories(webrtc-aec2 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set_target_properties(webrtc-aec2 PROPERTIES
    OUTPUT_NAME webrtc_aec2
)



