
cmake_minimum_required (VERSION 3.22.1)

project(asprintf, LANGUAGES C CXX)

set(asprintf_SOURCES)

list(APPEND asprintf_SOURCES 
    config_libasprintf.h
    alloca.c
    asnprintf.c
    asprintf.c
    autosprintf.cc
    lib-asprintf.c
    printf-args.c
    printf-parse.c
    vasnprintf.c
    vasprintf.c
    xsize_asprintf.c
)

add_library(asprintf STATIC ${asprintf_SOURCES})

target_compile_definitions(asprintf PRIVATE IN_LIBASPRINTF)

