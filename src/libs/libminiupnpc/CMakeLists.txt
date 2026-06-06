
cmake_minimum_required (VERSION 3.22.1)

project(miniupnpc, LANGUAGES C)

set (MINIUPNPC_VERSION 2.2)
set (MINIUPNPC_API_VERSION 8)

option (UPNPC_BUILD_STATIC "Build static library" TRUE)

set(miniupnpc_SOURCES)

list(APPEND miniupnpc_SOURCES
    include/igd_desc_parse.h
    include/miniupnpc.h
    include/miniupnpcstrings.h
    include/miniupnpctypes.h
    include/miniupnpc_declspec.h
    include/miniwget.h
    include/portlistingparse.h
    include/upnpcommands.h
    include/upnpdev.h
    include/upnperrors.h
    include/upnpreplyparse.h
    src/addr_is_reserved.c
    src/addr_is_reserved.h
    src/codelength.h
    src/connecthostport.c
    src/connecthostport.h
    src/igd_desc_parse.c
    src/listdevices.c
    #src/minihttptestserver.c
    src/minisoap.c
    src/minisoap.h
    src/minissdpc.c
    src/minissdpc.h
    src/miniupnpc.c
    #src/miniupnpcmodule.c
    src/miniupnpc_socketdef.h
    src/miniwget.c
    src/miniwget_private.h
    src/minixml.c
    src/minixml.h
    #src/minixmlvalid.c
    src/portlistingparse.c
    src/receivedata.c
    src/receivedata.h
    src/upnpc.c
    src/upnpcommands.c
    src/upnpdev.c
    src/upnperrors.c
    src/upnpreplyparse.c
    src/win32_snprintf.h
)

add_library(miniupnpc STATIC ${miniupnpc_SOURCES})

target_compile_definitions(miniupnpc PRIVATE MINIUPNP_STATICLIB)

target_include_directories(miniupnpc 
	PRIVATE 
	include
	..)

