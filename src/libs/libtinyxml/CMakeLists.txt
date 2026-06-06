
cmake_minimum_required (VERSION 3.22.1)

project(tinyxml, LANGUAGES CXX)

set(tinyxml_SOURCES)

list(APPEND tinyxml_SOURCES 
    tinystr.cpp
    tinyxml.cpp
    tinyxml2.cpp
    tinyxmlerror.cpp
    tinyxmlparser.cpp
)

add_library(tinyxml STATIC ${tinyxml_SOURCES})


