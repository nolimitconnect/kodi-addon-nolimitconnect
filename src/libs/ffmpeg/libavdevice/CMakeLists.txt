
cmake_minimum_required (VERSION 3.22.1)

project(avdevice, LANGUAGES C CXX)

set(avdevice_SOURCES)

list(APPEND avdevice_SOURCES 
    alldevices.c
    alsa.c
    alsa_dec.c
    alsa_enc.c
    avdevice.c
    bktr.c
    caca.c
    decklink_common.cpp
    decklink_dec.cpp
    decklink_dec_c.c
    decklink_enc.cpp
    decklink_enc_c.c
    dshow.c
    dshow_common.c
    dshow_crossbar.c
    dshow_enummediatypes.c
    dshow_enumpins.c
    dshow_filter.c
    dshow_pin.c
    fbdev_common.c
    fbdev_dec.c
    fbdev_enc.c
    file_open.c
    gdigrab.c
    iec61883.c
    jack.c
    lavfi.c
    libcdio.c
    libdc1394.c
    openal-dec.c
    opengl_enc.c
    oss.c
    oss_dec.c
    oss_enc.c
    pulse_audio_common.c
    pulse_audio_dec.c
    pulse_audio_enc.c
    sndio.c
    sndio_dec.c
    sndio_enc.c
    timefilter.c
    utils.c
    v4l2-common.c
    v4l2.c
    v4l2enc.c
    vfwcap.c
    xcbgrab.c
#	xv.c

)

add_library(avdevice STATIC ${avdevice_SOURCES})

target_include_directories(avdevice
  PRIVATE
    ../../libcurl/include
    ../../libcurl/lib
    ../../libplayer-nlc
)

nlc_target_add_supported_warning_options(avdevice C
    -Wno-deprecated-declarations
)

