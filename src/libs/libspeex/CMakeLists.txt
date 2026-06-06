
cmake_minimum_required (VERSION 3.22.1)

project(speex, LANGUAGES C CXX)

set(speex_SOURCES)

list(APPEND speex_SOURCES 
    bits_speex.c
    buffer_speex.c
    cb_search_speex.c
    exc_10_16_table_speex.c
    exc_10_32_table_speex.c
    exc_20_32_table_speex.c
    exc_5_256_table_speex.c
    exc_5_64_table_speex.c
    exc_8_128_table_speex.c
    fftwrap_speex.c
    filterbank_speex.c
    filters_speex.c
    gain_table_speex.c
    gain_table_lbr_speex.c
    hexc_10_32_table_speex.c
    hexc_table_speex.c
    high_lsp_tables_speex.c
    jitter_speex.c
    kiss_fft_speex.c
    kiss_fftr_speex.c
    lpc_speex.c
    lsp_speex.c
    lsp_tables_nb_speex.c
    ltp_speex.c
    mdf_speex.c
    modes_speex.c
    modes_wb_speex.c
    nb_celp_speex.c
    preprocess_speex.c
    quant_lsp_speex.c
    resample_speex.c
    sb_celp_speex.c
    scal_speex.c
    smallft_speex.c
    speex_speex.c
    speex_callbacks_speex.c
    speex_header_speex.c
    speex_window_speex.c
    stereo_speex.c
    vbr_speex.c
    vq_speex.c
)

add_library(speex STATIC ${speex_SOURCES})

