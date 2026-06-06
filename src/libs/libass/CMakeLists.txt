
cmake_minimum_required (VERSION 3.22.1)

project(ass, LANGUAGES C CXX)

set(ass_SOURCES)

list(APPEND ass_SOURCES 
   ass.c
   ass_bitmap.c
   ass_blur.c
   ass_cache.c
   ass_coretext.c
   ass_directwrite.c
   ass_drawing.c
   ass_font.c
   ass_fontconfig.c
   ass_fontselect.c
   ass_library.c
   ass_parse.c
   ass_rasterizer.c
   ass_rasterizer_c.c
   ass_render.c
   ass_render_api.c
   ass_shaper.c
   ass_string.c
   ass_strtod.c
   ass_utils.c

)

add_library(ass STATIC ${ass_SOURCES})

target_include_directories(ass
    PRIVATE
    ../libfreetype/include
)

nlc_target_add_supported_warning_options(ass C
    -Wno-constant-conversion
)

