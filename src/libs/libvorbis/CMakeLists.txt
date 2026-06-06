
cmake_minimum_required (VERSION 3.22.1)

project(vorbis2, LANGUAGES C CXX)

set(vorbis2_SOURCES)

list(APPEND vorbis2_SOURCES 
    lib/analysis_vorbis.c
    lib/bitrate_vorbis.c
    lib/block_vorbis.c
    lib/codebook_vorbis.c
    lib/envelope_vorbis.c
    lib/floor0_vorbis.c
    lib/floor1_vorbis.c
    lib/info_vorbis.c
    lib/lookup_vorbis.c
    lib/lpc_vorbis.c
    lib/lsp_vorbis.c
    lib/mapping0_vorbis.c
    lib/mdct_vorbis.c
    lib/psy_vorbis.c
    lib/registry_vorbis.c
    lib/res0_vorbis.c
    lib/sharedbook_vorbis.c
    lib/smallft_vorbis.c
    lib/synthesis_vorbis.c
    lib/vorbisenc_vorbis.c
    lib/window_vorbis.c
)

add_library(vorbis2 STATIC ${vorbis2_SOURCES})

target_include_directories(vorbis2
  PRIVATE
    include
    lib
    ../libogg/include
)


