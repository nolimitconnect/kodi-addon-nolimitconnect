
cmake_minimum_required (VERSION 3.22.1)

project(vxvideolib, LANGUAGES CXX)

set(vxvideolib_SOURCES)

list(APPEND vxvideolib_SOURCES 
    GdvBufferUtil.cpp
    VxRescaleRgb.cpp
    VxVidCap.cpp
    VxVidCapImp.cpp
    VxVideoConvert.cpp
)

add_library(vxvideolib STATIC ${vxvideolib_SOURCES})


