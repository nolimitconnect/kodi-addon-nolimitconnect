
cmake_minimum_required (VERSION 3.22.1)

project(theora, LANGUAGES C CXX)

set(theora_SOURCES)

list(APPEND theora_SOURCES 
    lib/x86_vc/mmxencfrag_theora.c
    lib/x86_vc/mmxfdct_theora.c
    lib/x86_vc/mmxfrag_theora.c
    lib/x86_vc/mmxidct_theora.c
    lib/x86_vc/mmxstate_theora.c
    lib/x86_vc/x86enc_theora.c
    lib/x86_vc/x86state_theora.c
    lib/analyze_theora.c
    lib/apiwrapper_theora.c
    lib/bitpack_theora.c
    lib/cpu_theora.c
    lib/decapiwrapper_theora.c
    lib/decinfo_theora.c
    lib/decode_theora.c
    lib/dequant_theora.c
    lib/encapiwrapper_theora.c
    lib/encfrag_theora.c
    lib/encinfo_theora.c
    lib/encode_theora.c
    lib/encoder_disabled_theora.c
    lib/enquant_theora.c
    lib/fdct_theora.c
    lib/fragment_theora.c
    lib/huffdec_theora.c
    lib/huffenc_theora.c
    lib/idct_theora.c
    lib/info_theora.c
    lib/internal_theora.c
    lib/mathops_theora.c
    lib/mcenc_theora.c
    lib/quant_theora.c
    lib/rate_theora.c
    lib/state_theora.c
    lib/tokenize_theora.c
)

add_library(theora STATIC ${theora_SOURCES})

target_compile_definitions(theora
    PRIVATE
    _BIND_TO_CURRENT_CRT_VERSION
    LIBTHEORA_EXPORTS
)

nlc_target_add_supported_warning_options(theora C
    -Wno-shift-negative-value
    -Wno-shift-op-parentheses
    -Wno-shift-negative-value
    -Wno-parentheses
)

target_include_directories(theora
  PRIVATE
    include
    lib
    ../libogg/include
)


