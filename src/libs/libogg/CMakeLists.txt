
cmake_minimum_required (VERSION 3.22.1)

project(ogg, LANGUAGES C CXX)

set(ogg_SOURCES)

list(APPEND ogg_SOURCES 
    src/bitwise_ogg.c
    src/framing_ogg.c
)

add_library(ogg STATIC ${ogg_SOURCES})


