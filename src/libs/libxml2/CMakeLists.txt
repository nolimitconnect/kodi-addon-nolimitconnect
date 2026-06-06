
cmake_minimum_required (VERSION 3.22.1)

project(xml2, LANGUAGES C CXX)

set(xml2_SOURCES)

list(APPEND xml2_SOURCES 
    tree_xml2.c
    error_xml2.c
    list_xml2.c
    threads_xml2.c
    hash_xml2.c
    dict_xml2.c
    c14n_xml2.c
    catalog_xml2.c
    chvalid_xml2.c
    debugXML_xml2.c
    DOCBparser_xml2.c
    encoding_xml2.c
    entities_xml2.c
    globals_xml2.c
    HTMLparser_xml2.c
    HTMLtree_xml2.c
    legacy_xml2.c
    nanoftp_xml2.c
    nanohttp_xml2.c
    parser_xml2.c
    parserInternals_xml2.c
    pattern_xml2.c
    relaxng_xml2.c
    SAX_xml2.c
    SAX2_xml2.c
    schematron_xml2.c
    trio_xml2.c
#    trionan_xml2.c
    triostr_xml2.c
    uri_xml2.c
    valid_xml2.c
    xinclude_xml2.c
    xlink_xml2.c
    xmlIO_xml2.c
#    xmllint_xml2.c
    xmlmemory_xml2.c
    xmlmodule_xml2.c
    xmlreader_xml2.c
    xmlregexp_xml2.c
    xmlsave_xml2.c
    xmlschemas_xml2.c
    xmlschemastypes_xml2.c
    xmlstring_xml2.c
    xmlunicode_xml2.c
    xmlwriter_xml2.c
    xpath_xml2.c
    xpointer_xml2.c
)

add_library(xml2 STATIC ${xml2_SOURCES})

target_compile_definitions(xml2
    PRIVATE
    LIBXML_STATIC
)

nlc_target_add_supported_warning_options(xml2 C
    -Wno-enum-compare
    -Wno-enum-conversion
    -Wno-empty-body
    -Wno-switch
    -Wno-tautological-pointer-compare
    -Wno-tautological-compare
)



