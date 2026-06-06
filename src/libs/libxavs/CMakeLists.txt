
cmake_minimum_required (VERSION 3.16.3)

project(xavs, LANGUAGES C CXX)

set(xavs_SOURCES)

list(APPEND xavs_SOURCES 
	common/common.c
	common/cpu.c
	common/csp.c
	common/dct.c
	common/frame.c
	common/macroblock.c
	common/mc.c
	common/mdate.c
	common/pixel.c
	common/predict.c
	common/quant.c
	common/set.c
	encoder/analyse.c
	encoder/cavlc.c
	encoder/encoder.c
	encoder/eval.c
	encoder/lookahead.c
	encoder/macroblock.c
	encoder/me.c
	encoder/ratecontrol.c
	encoder/set.c
	extras/getopt.c
)

add_library(xavs STATIC ${xavs_SOURCES})

target_compile_definitions(xavs
    PRIVATE
    __XAVS__
)

target_include_directories(xavs
  PRIVATE
    common
    encoder
    extras
)