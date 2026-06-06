
cmake_minimum_required (VERSION 3.22.1)

project(jpegturbo, LANGUAGES C CXX ASM)

if(DEFINED TARGET_CPU_X64)
    enable_language(ASM_NASM)
    set(CMAKE_ASM_NASM_FLAGS "-DELF -D__x86_64__ -DPIC -D__CET__")
    set(NASM_INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/simd
        ${CMAKE_CURRENT_SOURCE_DIR}/simd/nasm
        ${CMAKE_CURRENT_SOURCE_DIR}/simd/x86_64
    )

    foreach(nasmdir ${NASM_INCLUDE_DIRS})
        list(APPEND NASM_INCLUDE_FLAGS "-I${nasmdir}")
    endforeach()
    message(STATUS "NASM include flags ${NASM_INCLUDE_FLAGS}")

    foreach(nasmflag ${NASM_INCLUDE_FLAGS})
        set(CMAKE_ASM_NASM_FLAGS "${CMAKE_ASM_NASM_FLAGS} ${nasmflag}")
    endforeach()
    message(STATUS "Final NASM flags: ${CMAKE_ASM_NASM_FLAGS}")

    set(CMAKE_ASM_NASM_OBJECT_FORMAT elf64)
    set(CMAKE_ASM_NASM_SOURCE_FILE_EXTENSIONS asm)

    set(CMAKE_ASM_NASM_COMPILE_OBJECT "<CMAKE_ASM_NASM_COMPILER>  <FLAGS> -f ${CMAKE_ASM_NASM_OBJECT_FORMAT} -o <OBJECT> <SOURCE>")
    set(CMAKE_DEPFILE_FLAGS_ASM_NASM "-MD <DEP_FILE> -MT <DEP_TARGET>")
endif()

set(jpegturbo_SOURCES)

list(APPEND jpegturbo_SOURCES 
    src/cderror.h
    src/cdjpeg.h
    #src/cjpeg.c #requires file io
    src/cmyk.h

    #src/djpeg.c #requires file io

    src/jaricom.c
    src/jcapimin.c
    src/jcapistd.c
    src/jcarith.c
    src/jccoefct.c
    #src/jccolext.c #included by others
    src/jccolor.c
    src/jcdctmgr.c
    src/jcdiffct.c
    src/jchuff.c
    src/jchuff.h
    src/jcicc.c
    src/jcinit.c
    src/jclhuff.c
    src/jclossls.c

    src/jcmainct.c
    src/jcmarker.c
    src/jcmaster.h
    src/jcmaster.c
    src/jcomapi.c
    src/jconfig.txt
    src/jcparam.c
    src/jcphuff.c
    src/jcprepct.c

    src/jcsample.c
    src/jctrans.c

    src/jdapimin.c
    src/jdapistd.c
    src/jdarith.c
    src/jdatadst-tj.c
    src/jdatadst.c
    src/jdatasrc-tj.c
    src/jdatasrc.c
    src/jdcoefct.c
    src/jdcoefct.h
    #src/jdcol565.c #included by others
    #src/jdcolext.c #included by others
    src/jdcolor.c
    src/jdct.h
    src/jddctmgr.c
    src/jddiffct.c
    src/jdhuff.c
    src/jdhuff.h
    src/jdicc.c
    src/jdinput.c
    src/jdlhuff.c
    src/jdlossls.c
    src/jdmainct.c
    src/jdmainct.h
    src/jdmarker.c
    src/jdmaster.c
    src/jdmaster.h
    src/jdmerge.c
    src/jdmerge.h
    #src/jdmrg565.c #included by others
    #src/jdmrgext.c #included by others
    src/jdphuff.c
    src/jdpostct.c
    src/jdsample.c
    src/jdsample.h
    src/jdtrans.c
    src/jerror.c
    src/jerror.h
    src/jfdctflt.c
    src/jfdctfst.c
    src/jfdctint.c
    src/jidctflt.c
    src/jidctfst.c
    src/jidctint.c
    src/jidctred.c
    src/jinclude.h
    src/jlossls.h
    src/jmemmgr.c
    src/jmemnobs.c
    src/jmemsys.h
    src/jmorecfg.h
    src/jpegapicomp.h
    src/jpegint.h
    src/jpeglib.h

    src/jpeg_nbits.c
    src/jpeg_nbits.h
    src/jquant1.c
    src/jquant2.c
    src/jsamplecomp.h
    src/jsimd.h
    src/jsimddct.h

    #src/jstdhuff.c #included by others
    src/jutils.c
    src/libjpeg.map.in
    #src/md5 # included in CoreLib
    src/rdbmp.c
    src/rdcolmap.c
    src/rdgif.c
    src/rdjpgcom.c
    src/rdppm.c
    src/rdswitch.c
    src/rdtarga.c

    src/tjutil.c
    src/tjutil.h

    src/transupp.h
    src/transupp.c
    src/turbojpeg-mapfile
    src/turbojpeg-mapfile.jni
    #src/turbojpeg-mp.c #included by others
    src/turbojpeg.c
    src/turbojpeg.h
    src/wrbmp.c
    src/wrgif.c
    src/wrppm.c

    simd/jsimd.h

    VxJpgLib.h
    VxJpg.cpp
)

if(DEFINED TARGET_CPU_X64)

    list(APPEND jpegturbo_SOURCES

    simd/nasm/jcolsamp.inc
    simd/nasm/jdct.inc
    simd/nasm/jsimdcfg.inc
    simd/nasm/jsimdcfg.inc.h
    simd/nasm/jsimdext.inc

    #simd/x86_64/jccolext-avx2.asm
    #simd/x86_64/jccolext-sse2.asm
    simd/x86_64/jccolor-avx2.asm
    simd/x86_64/jccolor-sse2.asm
    simd/x86_64/jcgray-avx2.asm
    simd/x86_64/jcgray-sse2.asm
    #simd/x86_64/jcgryext-avx2.asm
    #simd/x86_64/jcgryext-sse2.asm
    simd/x86_64/jchuff-sse2.asm
    simd/x86_64/jcphuff-sse2.asm
    simd/x86_64/jcsample-avx2.asm
    simd/x86_64/jcsample-sse2.asm
    #simd/x86_64/jdcolext-avx2.asm
    #simd/x86_64/jdcolext-sse2.asm
    simd/x86_64/jdcolor-avx2.asm
    simd/x86_64/jdcolor-sse2.asm
    simd/x86_64/jdmerge-avx2.asm
    simd/x86_64/jdmerge-sse2.asm
    #simd/x86_64/jdmrgext-avx2.asm
    #simd/x86_64/jdmrgext-sse2.asm
    simd/x86_64/jdsample-avx2.asm
    simd/x86_64/jdsample-sse2.asm
    simd/x86_64/jfdctflt-sse.asm
    simd/x86_64/jfdctfst-sse2.asm
    simd/x86_64/jfdctint-avx2.asm
    simd/x86_64/jfdctint-sse2.asm
    simd/x86_64/jidctflt-sse2.asm
    simd/x86_64/jidctfst-sse2.asm
    simd/x86_64/jidctint-avx2.asm
    simd/x86_64/jidctint-sse2.asm
    simd/x86_64/jidctred-sse2.asm
    simd/x86_64/jquantf-sse2.asm
    simd/x86_64/jquanti-avx2.asm
    simd/x86_64/jquanti-sse2.asm
    simd/x86_64/jsimd.c
    simd/x86_64/jsimdcpu.asm
    )

    # Ensure x86_64 .asm sources are built with NASM, not the generic ASM compiler.
    set(_jpegturbo_nasm_sources)
    foreach(_jpegturbo_src IN LISTS jpegturbo_SOURCES)
        if(_jpegturbo_src MATCHES "^simd/x86_64/.*\\.asm$")
            list(APPEND _jpegturbo_nasm_sources ${_jpegturbo_src})
        endif()
    endforeach()
    if(_jpegturbo_nasm_sources)
        set_source_files_properties(${_jpegturbo_nasm_sources} PROPERTIES LANGUAGE ASM_NASM)
    endif()

endif()

if(DEFINED TARGET_CPU_ARM64 OR DEFINED TARGET_CPU_ARM32)
    list(APPEND jpegturbo_SOURCES 
        simd/arm/align.h
        #simd/arm/jccolor-neon.c
        simd/arm/jcgray-neon.c
        simd/arm/jchuff.h
        simd/arm/jcphuff-neon.c
        simd/arm/jcsample-neon.c
        #simd/arm/jdcolext-neon.c #included by others
        #simd/arm/jdcolor-neon.c #included by others
        simd/arm/jdmerge-neon.c
        #simd/arm/jdmrgext-neon.c #included by others
        simd/arm/jdsample-neon.c
        simd/arm/jfdctfst-neon.c
        simd/arm/jidctfst-neon.c
        #simd/arm/jfdctint-neon.c

        simd/arm/jidctred-neon.c
        simd/arm/jquanti-neon.c     
     )
endif()

if(DEFINED TARGET_CPU_ARM64)
    list(APPEND jpegturbo_SOURCES 
        #simd/arm/aarch64/jchuff-neon.c # implemented in jsimd_neon.S
        simd/arm/aarch64/jsimd.c
        simd/arm/aarch64/jsimd_neon.S
     )
elseif(DEFINED TARGET_CPU_ARM32)
    list(APPEND jpegturbo_SOURCES
        #simd/arm/aarch32/jchuff-neon.c
        simd/arm/aarch32/jsimd.c
        simd/arm/aarch32/jsimd_neon.S
    )
endif()

add_library(jpegturbo STATIC ${jpegturbo_SOURCES})

target_include_directories(jpegturbo 
	PRIVATE 
	${CMAKE_CURRENT_SOURCE_DIR} 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/simd
    ${CMAKE_CURRENT_SOURCE_DIR}/simd/nasm
    )

if(DEFINED TARGET_CPU_X64)
    target_include_directories(jpegturbo
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/simd/x86_64
        )
elseif(DEFINED TARGET_CPU_ARM64)
    target_include_directories(jpegturbo
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/simd/arm
        ${CMAKE_CURRENT_SOURCE_DIR}/simd/arm/aarch64
        )
elseif(DEFINED TARGET_CPU_ARM32)
    target_include_directories(jpegturbo
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/simd/arm
        ${CMAKE_CURRENT_SOURCE_DIR}/simd/arm/aarch32
        )
endif()

target_compile_definitions(jpegturbo
    PRIVATE
    ${jpegturbo_DEFINES}
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    # turn on optimization even when in debug mode
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
endif()




