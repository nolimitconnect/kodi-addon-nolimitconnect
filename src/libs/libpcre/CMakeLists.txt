
cmake_minimum_required (VERSION 3.22.1)

project(pcre, LANGUAGES C CXX)

set(pcre_SOURCES)

list(APPEND pcre_SOURCES 
    config_pcre.c
    pcre_chartables.c
    pcre_compile.c
    pcre_dfa_exec.c
    pcre_exec.c
    pcre_fullinfo.c
    pcre_get.c
    pcre_globals.c
    pcre_info.c
    pcre_maketables.c
    pcre_newline.c
    pcre_ord2utf8.c
    pcre_refcount.c
    pcre_scanner.cc
    pcre_stringpiece.cc
    pcre_study.c
    pcre_tables.c
    pcre_try_flipped.c
    pcre_ucp_searchfuncs.c
    pcre_valid_utf8.c
    pcre_version.c
    pcre_xclass.c
    pcrecpp.cc
#    pcregrep.c
    pcreposix.c
    dftables_pcre.c
    ucptable_pcre.c
)

add_library(pcre STATIC ${pcre_SOURCES})

nlc_target_add_supported_warning_options(pcre C
    -Wno-string-compare
)


