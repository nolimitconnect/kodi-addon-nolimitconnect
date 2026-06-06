
cmake_minimum_required (VERSION 3.22.1)

project(mfx, LANGUAGES C CXX)

set(mfx_SOURCES)

list(APPEND mfx_SOURCES 
  src/main.cpp
  src/mfx_dispatcher.cpp
  src/mfx_dispatcher_log.cpp
  src/mfx_function_table.cpp
)

add_library(mfx STATIC ${mfx_SOURCES})

nlc_target_add_supported_warning_options(mfx CXX
    -Wno-undefined-bool-conversion
)

if(DEFINED TARGET_OS_WINDOWS)
    target_compile_definitions(mfx
      PRIVATE
        WIN64
    )
endif()

target_compile_definitions(mfx
    PRIVATE
    _LIB
    MFX_VA
)



