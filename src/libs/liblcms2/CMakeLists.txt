
cmake_minimum_required (VERSION 3.16.3)

project(CoreLib, LANGUAGES CXX)

set(CoreLib_SOURCES)

list(APPEND CoreLib_SOURCES 
    ACppFile.cpp

)


add_library(CoreLib STATIC ${CoreLib_SOURCES})
