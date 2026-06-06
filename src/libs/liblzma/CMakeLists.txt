
cmake_minimum_required (VERSION 3.22.1)

project(lzma, LANGUAGES C CXX)

set(lzma_SOURCES)

list(APPEND lzma_SOURCES 
    check/check.c
    check/crc32_fast.c
    check/crc32_small.c
    check/crc32_table.c
    check/crc32_tablegen.c
    check/crc64_fast.c
    check/crc64_small.c
    check/crc64_table.c
    check/crc64_tablegen.c
    check/sha256.c
    common/alone_decoder.c
    common/alone_encoder.c
    common/auto_decoder.c
    common/block_buffer_decoder.c
    common/block_buffer_encoder.c
    common/block_decoder.c
    common/block_encoder.c
    common/block_header_decoder.c
    common/block_header_encoder.c
    common/block_util.c
    common/common_lzma.c
    common/easy_buffer_encoder.c
    common/easy_decoder_memusage.c
    common/easy_encoder.c
    common/easy_encoder_memusage.c
    common/easy_preset.c
    common/filter_buffer_decoder.c
    common/filter_buffer_encoder.c
    common/filter_common.c
    common/filter_decoder.c
    common/filter_encoder.c
    common/filter_flags_decoder.c
    common/filter_flags_encoder.c
    common/hardware_physmem.c
    common/index.c
    common/index_decoder.c
    common/index_encoder.c
    common/index_hash.c
    common/stream_buffer_decoder.c
    common/stream_buffer_encoder.c
    common/stream_decoder.c
    common/stream_encoder.c
    common/stream_flags_common.c
    common/stream_flags_decoder.c
    common/stream_flags_encoder.c
    common/vli_decoder.c
    common/vli_encoder.c
    common/vli_size.c
    delta/delta_common.c
    delta/delta_decoder.c
    delta/delta_encoder.c
    lz/lz_decoder.c
    lz/lz_encoder.c
    lz/lz_encoder_mf.c
    lzma/fastpos_table.c
    lzma/fastpos_tablegen.c
    lzma/lzma2_decoder.c
    lzma/lzma2_encoder.c
    lzma/lzma_decoder.c
    lzma/lzma_encoder.c
    lzma/lzma_encoder_optimum_fast.c
    lzma/lzma_encoder_optimum_normal.c
    lzma/lzma_encoder_presets.c
    rangecoder/price_table.c
    rangecoder/price_tablegen.c
    simple/arm.c
    simple/armthumb.c
    simple/ia64.c
    simple/powerpc.c
    simple/simple_coder.c
    simple/simple_decoder.c
    simple/simple_encoder.c
    simple/sparc.c
    simple/x86.c
)

add_library(lzma STATIC ${lzma_SOURCES})

nlc_target_add_supported_warning_options(lzma C
    -Wno-unused-value
)


