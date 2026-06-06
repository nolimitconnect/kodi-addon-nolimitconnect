
cmake_minimum_required (VERSION 3.22.1)

project(twolame, LANGUAGES C CXX)

set(twolame_SOURCES)

list(APPEND twolame_SOURCES 
    ath_twolame.c
    availbits_twolame.c
    bitbuffer_twolame.c
    crc_twolame.c
    dab_twolame.c
    encode_twolame.c
    energy_twolame.c
    fft_twolame.c
    get_set_twolame.c
    mem_twolame.c
    psycho_0_twolame.c
    psycho_1_twolame.c
    psycho_2_twolame.c
    psycho_3_twolame.c
    psycho_4_twolame.c
    psycho_n1_twolame.c
    subband_twolame.c
    twolame_twolame.c
    util_twolame.c
)

add_library(twolame STATIC ${twolame_SOURCES})

nlc_target_add_supported_warning_options(twolame C
    -Wno-deprecated-non-prototype
)


