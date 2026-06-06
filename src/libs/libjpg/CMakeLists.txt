
cmake_minimum_required (VERSION 3.22.1)

project(jpg, LANGUAGES C CXX)

set(jpg_SOURCES)

list(APPEND jpg_SOURCES 
    VxJpg.cpp
    jcapimin.c
    jcapistd.c
    jccoefct.c
    jccolor.c
    jcdctmgr.c
    jchuff.c
    jcinit.c
    jcmainct.c
    jcmarker.c
    jcmaster.c
    jcomapi.c
    jcparam.c
    jcphuff.c
    jcprepct.c
    jcsample.c
    jctrans.c
    jdapimin.c
    jdapistd.c
    jdatadst.c
    jdatasrc.c
    jdcoefct.c
    jdcolor.c
    jddctmgr.c
    jdhuff.c
    jdinput.c
    jdmainct.c
    jdmarker.c
    jdmaster.c
    jdmerge.c
    jdphuff.c
    jdpostct.c
    jdsample.c
    jdtrans.c
    jerror.c
    jfdctflt.c
    jfdctfst.c
    jfdctint.c
    jidctflt.c
    jidctfst.c
    jidctint.c
    jidctred.c
    jmemansi.c
    jmemmgr.c
    jquant1.c
    jquant2.c
    jutils.c
    transupp.c
)

add_library(jpg STATIC ${jpg_SOURCES})

if(DEFINED TARGET_OS_WINDOWS)
    target_compile_definitions(jpg PRIVATE
        HAVE_CONFIG_H
        WIN32
        _LIB
        _CRT_SECURE_NO_WARNINGS
    )

else()
    # Override optimization flags for this library
    target_compile_options(jpg PRIVATE
        $<$<CONFIG:Debug>:-O2 -g>
        $<$<CONFIG:Release>:-O3>
    )

endif()


