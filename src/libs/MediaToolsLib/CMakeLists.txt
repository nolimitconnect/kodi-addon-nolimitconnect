
cmake_minimum_required (VERSION 3.22.1)

project(mediatools, LANGUAGES C CXX)

set(mediatools_SOURCES)

list(APPEND mediatools_SOURCES 
    OpusTools/win32/unicode_support_mtools.c
    OpusTools/audio-in_mtools.c
    OpusTools/diag_range_mtools.c
    OpusTools/flac_mtools.c
    OpusTools/lpc_mtools.c
    OpusTools/opus_header_mtools.c
    OpusTools/opusdec_mtools.c
    OpusTools/opusenc_mtools.c
    OpusTools/picture_mtools.c
    OpusTools/wav_io_mtools.c
    OpusTools/wave_out_mtools.c
    AviDefs_mtools.cpp
    MediaTools_mtools.cpp
    MJPEGReader_mtools.cpp
    MJPEGWriter_mtools.cpp
    MyOpusHeader_mtools.cpp
    OggBuffer_mtools.cpp
    OggStream_mtools.cpp
    OpusFileDecoder_mtools.cpp
    OpusFileEncoder_mtools.cpp
    SndReader_mtools.cpp
    SndWriter_mtools.cpp
)

add_library(mediatools STATIC ${mediatools_SOURCES})

# MediaTools uses OpusCodec implemented in libs/ffmpeg/opus/OpusCodec.cpp.
# Keep this dependency explicit so static-link order does not drop symbols.
target_link_libraries(mediatools
    PRIVATE
    ffmpegopus
)

nlc_target_add_supported_warning_options(mediatools C
    -Wno-macro-redefined
)

target_compile_definitions(mediatools
    PRIVATE
    OUTSIDE_SPEEX
    __LITTLE_ENDIAN
)

if(DEFINED TARGET_OS_WINDOWS)
    target_include_directories(mediatools
        PRIVATE
        opus/win32
    )
endif()

target_include_directories(mediatools
  PRIVATE
    OpusTools/include
    include
    ../libptopengine
    ../ffmpeg
)


