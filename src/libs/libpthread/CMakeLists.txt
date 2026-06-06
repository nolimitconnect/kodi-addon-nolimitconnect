
cmake_minimum_required (VERSION 3.16.3)

project(pthread, LANGUAGES C CXX)

set(pthread_SOURCES)

list(APPEND pthread_SOURCES 
    pthread.c
)

add_library(pthread STATIC ${pthread_SOURCES})
