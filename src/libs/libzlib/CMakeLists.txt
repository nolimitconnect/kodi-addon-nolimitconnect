
cmake_minimum_required (VERSION 3.22.1)

project(zlib, LANGUAGES C CXX)

set(zlib_SOURCES)

list(APPEND zlib_SOURCES 
    zlib.h
    zconf_libzlib.h
    compress_zlib.c
    adler32_zlib.c
    crc32_zlib.c
    deflate_zlib.c
    gzclose_zlib.c
    gzlib_zlib.c
    gzread_zlib.c
    gzwrite_zlib.c
    infback_zlib.c
    inffast_zlib.c
    inflate_zlib.c
    inftrees_zlib.c
#    minigzip_zlib.c
    trees_zlib.c
    uncompr_zlib.c
    zutil_zlib.c
)

add_library(zlib STATIC ${zlib_SOURCES})

if(NOT DEFINED TARGET_OS_WINDOWS)
    nlc_target_add_supported_warning_options(zlib C
        -Wno-deprecated-non-prototype
        -Wno-shift-negative-value
    )

endif()

