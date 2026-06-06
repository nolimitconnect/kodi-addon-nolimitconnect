
cmake_minimum_required (VERSION 3.22.1)

project(postproc, LANGUAGES C CXX)

set(postproc_SOURCES)

list(APPEND postproc_SOURCES 
  postprocess.c

)

add_library(postproc STATIC ${postproc_SOURCES})

target_include_directories(postproc
  PRIVATE
    ../../libcurl/include
    ../../libcurl/lib
    ../../libplayer-nlc
)

nlc_target_add_supported_warning_options(postproc C
    -Wno-deprecated-declarations
    -Wno-pointer-sign
)

