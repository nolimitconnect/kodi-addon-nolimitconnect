
cmake_minimum_required (VERSION 3.22.1)

project(libwav-decoder, LANGUAGES CXX)

set(libwav-decoder_SOURCES)

list(APPEND libwav-decoder_SOURCES 
    wav_decoder.h
    wav_decoder.cpp
    WavMgr.h
    WavMgr.cpp
)

add_library(wav-decoder STATIC ${libwav-decoder_SOURCES})


