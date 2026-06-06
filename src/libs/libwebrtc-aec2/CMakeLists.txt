
cmake_minimum_required(VERSION 3.28.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(webrtc_aec2 LANGUAGES CXX)
add_subdirectory(apm)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
