
cmake_minimum_required (VERSION 3.22.1)

project(crossguid LANGUAGES C CXX)

set(crossguid_SOURCES)

list(APPEND crossguid_SOURCES 
    guid.cpp
)

add_library(crossguid STATIC ${crossguid_SOURCES})

