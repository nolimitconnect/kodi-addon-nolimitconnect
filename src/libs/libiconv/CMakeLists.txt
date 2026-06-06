
cmake_minimum_required (VERSION 3.22.1)

project(iconv, LANGUAGES C CXX)

set(iconv_SOURCES)

list(APPEND iconv_SOURCES 
    iconv_iconv.c
    localcharset_iconv.c
    relocatable_iconv.c
)

add_library(iconv STATIC ${iconv_SOURCES})

nlc_target_add_supported_warning_options(iconv C
    -Wno-static-in-inline
    -Wno-parentheses-equality
)

if(DEFINED TARGET_OS_WINDOWS)
    target_compile_definitions(iconv
      PRIVATE
        _WIN64
        _WINDOWS
        WINDOWS
        _USRDLL
        BUILDING_LIBICONV
        BUILDING_LIBCHARSET
        LIBICONV_EXPORTS
    )

# _WIN64;_DEBUG;_WINDOWS;_USRDLL;BUILDING_LIBICONV;BUILDING_LIBCHARSET;LIBICONV_EXPORTS;%(PreprocessorDefinitions)

endif()

if(DEFINED TARGET_OS_LINUX)
    target_compile_definitions(iconv
      PRIVATE
        USING_STATIC_LIBICONV
    )
endif()

if(DEFINED TARGET_OS_ANDROID)
    target_compile_definitions(iconv
      PRIVATE
        USING_STATIC_LIBICONV
    )
endif()

