
cmake_minimum_required (VERSION 3.16.3)

project(plist, LANGUAGES C CXX)

set(plist_SOURCES)

list(APPEND plist_SOURCES 
    blocksort.c
    bzlib.c
    compress.c
    crctable.c
    decompress.c
    huffman.c
    randtable.c
)

add_library(plist STATIC ${plist_SOURCES})
