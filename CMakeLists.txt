cmake_minimum_required(VERSION 3.20)

project(kodi-addon-nolimitconnect LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(NLC_PLATFORM_DEFINES)

if(WIN32)
  list(APPEND NLC_PLATFORM_DEFINES TARGET_OS_WINDOWS=1 TARGET_CPU_X64=1)
else()
  list(APPEND NLC_PLATFORM_DEFINES TARGET_POSIX=1)

  if(ANDROID)
    list(APPEND NLC_PLATFORM_DEFINES TARGET_OS_ANDROID=1)
    if(CMAKE_ANDROID_ARCH_ABI STREQUAL "arm64-v8a")
      list(APPEND NLC_PLATFORM_DEFINES TARGET_CPU_ARM64=1)
    elseif(CMAKE_ANDROID_ARCH_ABI STREQUAL "x86_64")
      list(APPEND NLC_PLATFORM_DEFINES TARGET_CPU_X64=1)
    else()
      message(FATAL_ERROR "Unsupported Android ABI '${CMAKE_ANDROID_ARCH_ABI}'. Only arm64-v8a and x86_64 are supported.")
    endif()
  elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    list(APPEND NLC_PLATFORM_DEFINES TARGET_OS_LINUX=1)
    string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" NLC_SYSTEM_PROCESSOR_LOWER)
    if(NLC_SYSTEM_PROCESSOR_LOWER MATCHES "^(aarch64|arm64)$")
      list(APPEND NLC_PLATFORM_DEFINES TARGET_CPU_ARM64=1)
    elseif(NLC_SYSTEM_PROCESSOR_LOWER MATCHES "^(x86_64|amd64|x64)$")
      list(APPEND NLC_PLATFORM_DEFINES TARGET_CPU_X64=1)
    else()
      message(FATAL_ERROR "Unsupported CPU '${CMAKE_SYSTEM_PROCESSOR}'. Only x86_64 and aarch64 are supported.")
    endif()
  elseif(APPLE)
    list(APPEND NLC_PLATFORM_DEFINES TARGET_OS_APPLE=1)
    message(FATAL_ERROR "Apple platform is not supported by the vendored NLC libs in this build.")
  else()
    message(FATAL_ERROR "Unsupported platform '${CMAKE_SYSTEM_NAME}' for vendored NLC libs.")
  endif()
endif()

option(NLC_ENABLE_DEV_STUBS "Enable development stubs for non-wired components" ON)

set(NLC_LIBS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/libs" CACHE PATH
  "Path to imported NoLimitConnect libs tree (contains GuiInterface, libptopengine, CoreLib, etc.)")

set(NLC_KODI_SOURCE_CANDIDATES
  "$ENV{KODI_SOURCE_DIR}"
  "${CMAKE_CURRENT_SOURCE_DIR}/../kodi-source"
  "$ENV{HOME}/kodi-source"
  "/home/${USER}/kodi-source"
)

if(NOT DEFINED KODI_SOURCE_DIR)
  foreach(candidate IN LISTS NLC_KODI_SOURCE_CANDIDATES)
    if(candidate AND EXISTS "${candidate}/xbmc/addons/kodi-dev-kit/include/kodi/AddonBase.h")
      set(KODI_SOURCE_DIR "${candidate}" CACHE PATH "Path to Kodi source tree")
      break()
    endif()
  endforeach()
endif()

if(NOT DEFINED KODI_SOURCE_DIR OR KODI_SOURCE_DIR STREQUAL "")
  message(FATAL_ERROR "KODI_SOURCE_DIR is required. Set -DKODI_SOURCE_DIR=/path/to/kodi-source or export KODI_SOURCE_DIR.")
endif()

set(KODI_DEVKIT_INCLUDE_HINTS
  "${KODI_SOURCE_DIR}/xbmc/addons/kodi-dev-kit/include"
  "${KODI_SOURCE_DIR}/addons/kodi-dev-kit/include"
)

find_path(KODI_DEVKIT_INCLUDE_DIR
  NAMES kodi/AddonBase.h
  PATHS ${KODI_DEVKIT_INCLUDE_HINTS}
  NO_DEFAULT_PATH
)

if(NOT KODI_DEVKIT_INCLUDE_DIR)
  message(FATAL_ERROR "Could not locate kodi/AddonBase.h under KODI_SOURCE_DIR='${KODI_SOURCE_DIR}'")
endif()

set(NLC_LIBS_INCLUDE_DIRS)
if(EXISTS "${NLC_LIBS_DIR}")
  list(APPEND NLC_LIBS_INCLUDE_DIRS
    "${NLC_LIBS_DIR}"
    "${NLC_LIBS_DIR}/GuiInterface"
    "${NLC_LIBS_DIR}/CoreLib"
    "${NLC_LIBS_DIR}/libptopengine"
  )
  message(STATUS "Using NLC libs from: ${NLC_LIBS_DIR}")
else()
  message(WARNING "NLC libs directory not found at '${NLC_LIBS_DIR}'. Configure with -DNLC_LIBS_DIR=/path/to/libs once copied.")
endif()

set(NLC_SOURCES
  src/addon/addon.cpp
  src/addon/FromGuiBridge.cpp
  src/addon/NlcAddon.cpp
  src/addon/ToGuiBridge.cpp
  src/core/SignOnFlow.cpp
  src/gui/WindowMain.cpp
)

add_library(kodi-addon-nolimitconnect SHARED ${NLC_SOURCES})

target_include_directories(kodi-addon-nolimitconnect
  PRIVATE
    ${KODI_DEVKIT_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${NLC_LIBS_INCLUDE_DIRS}
)

target_compile_definitions(kodi-addon-nolimitconnect
  PRIVATE
    ${NLC_PLATFORM_DEFINES}
    $<$<BOOL:${NLC_ENABLE_DEV_STUBS}>:NLC_ENABLE_DEV_STUBS=1>
)

set_target_properties(kodi-addon-nolimitconnect PROPERTIES
  OUTPUT_NAME "kodi-addon-nolimitconnect"
)
