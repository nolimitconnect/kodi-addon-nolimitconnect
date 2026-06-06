
cmake_minimum_required (VERSION 3.22.1)

project(fribidi, LANGUAGES C CXX)

set(fribidi_SOURCES)

list(APPEND fribidi_SOURCES 
    lib/fribidi-arabic.c
    lib/fribidi-bidi-types.c
    lib/fribidi-bidi.c
    lib/fribidi-deprecated.c
    lib/fribidi-joining-types.c
    lib/fribidi-joining.c
    lib/fribidi-mem.c
    lib/fribidi-mirroring.c
    lib/fribidi-run.c
    lib/fribidi-shape.c
    lib/fribidi.c
    charset/fribidi-char-sets.c
    charset/fribidi-char-sets-cap-rtl.c
    charset/fribidi-char-sets-cp1255.c
    charset/fribidi-char-sets-cp1256.c
    charset/fribidi-char-sets-iso8859-6.c
    charset/fribidi-char-sets-iso8859-8.c
    charset/fribidi-char-sets-utf8.c
)

add_library(fribidi STATIC ${fribidi_SOURCES})

target_include_directories(fribidi
  PRIVATE
    lib
    charset
)

nlc_target_add_supported_warning_options(fribidi C
    -Wno-switch
    -Wno-enum-conversion
)


