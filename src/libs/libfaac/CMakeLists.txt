
cmake_minimum_required (VERSION 3.16.3)

project(faac, LANGUAGES C CXX)

set(faac_SOURCES)

list(APPEND faac_SOURCES 
	aacquant.c
	backpred.c
	bitstream.c
	channels.c
	fft.c
	filtbank.c
	frame.c
	huffman.c
	ltp.c
	midside.c
	psychkni.c
	tns.c
	util.c
)

add_library(faac STATIC ${faac_SOURCES})
