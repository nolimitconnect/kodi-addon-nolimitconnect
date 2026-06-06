
cmake_minimum_required (VERSION 3.22.1)

project(glew, LANGUAGES C CXX)

set(glew_SOURCES)

list(APPEND glew_SOURCES 
    src/glew.c
)

add_library(glew STATIC ${glew_SOURCES})

target_compile_definitions(glew
  PRIVATE
    GLEW_STATIC
    DO_NOT_DEFINE_BOOL
)

target_include_directories(glew
  PRIVATE
    include
)


