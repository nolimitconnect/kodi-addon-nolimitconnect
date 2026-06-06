
cmake_minimum_required(VERSION 3.28.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(webrtc_aec LANGUAGES CXX)

get_directory_property(_nlc_webrtc_aec_saved_includes INCLUDE_DIRECTORIES)
get_directory_property(_nlc_webrtc_aec_saved_definitions COMPILE_DEFINITIONS)

if(MSVC)
	# set_property(DIRECTORY PROPERTY INCLUDE_DIRECTORIES "")
	# Ensure standard Windows flags and exception handling are included
	set(CMAKE_CXX_FLAGS_INIT "${CMAKE_CXX_FLAGS_INIT} /DWIN32 /D_WINDOWS /EHsc")
endif()

set_directory_properties(PROPERTIES
	INCLUDE_DIRECTORIES ""
	COMPILE_DEFINITIONS ""
)
add_subdirectory(abseil-cpp)

set_directory_properties(PROPERTIES
	INCLUDE_DIRECTORIES "${_nlc_webrtc_aec_saved_includes}"
	COMPILE_DEFINITIONS "${_nlc_webrtc_aec_saved_definitions}"
)
add_subdirectory(apm)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
