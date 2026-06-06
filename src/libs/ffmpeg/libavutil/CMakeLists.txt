
cmake_minimum_required (VERSION 3.22.1)

project(avutil, LANGUAGES C CXX ASM)

if(DEFINED TARGET_OS_ANDROID)
    #android/aarch64 uses clang as assembler
elseif(DEFINED TARGET_OS_WINDOWS)
    enable_language(ASM_NASM)
    string(APPEND CMAKE_ASM_NASM_FLAGS "-Pconfig_ffmpeg_w64.asm ")
else()
    enable_language(ASM_NASM)
    set(FFMPEG_CONFIG_ASM "${PROJECT_SRC_ROOT_DIR}/libs/ffmpeg/config_ffmpeg_linux.asm")
    string(APPEND CMAKE_ASM_NASM_FLAGS " -P${FFMPEG_CONFIG_ASM} ")
endif()

set(avutil_SOURCES)

if(DEFINED TARGET_CPU_X64)
    list(APPEND avutil_SOURCES 
        x86/cpu.c
        x86/fixed_dsp_init.c
        x86/float_dsp_init.c
        x86/imgutils_init.c
        x86/lls_init.c
        x86/pixelutils_init.c
        x86/tx_float_init.c
        x86/cpuid_avutil.asm
        x86/emms_avutil.asm
        x86/fixed_dsp.asm
        x86/float_dsp.asm
        x86/imgutils.asm
        x86/lls.asm
        x86/pixelutils.asm
        x86/tx_float.asm
        x86/x86inc.asm
        x86/x86util.asm
    )
endif()

if(DEFINED TARGET_CPU_ARM64)
    list(APPEND avutil_SOURCES 
        # tx_float_neon.S causes error
        # error: relocation R_AARCH64_ADD_ABS_LO12_NC cannot be used against symbol ff_tx_tab_4096_float; recompile with -fPIC
        # even though is compiled with -DPIC and -fPIC
        # there seems to be no solution and older version of ffmpeg did not have tx_float_init.c and tx_float_neon.S
        # so they are removed from the compile

        aarch64/cpu.c
        aarch64/float_dsp_init.c
        #aarch64/tx_float_init.c
        #included by others aarch64/asm.S
        aarch64/float_dsp_neon.S
        #aarch64/tx_float_neon.S
    )
elseif(DEFINED TARGET_CPU_ARM32)
    list(APPEND avutil_SOURCES
        arm/cpu.c
        arm/float_dsp_init_arm.c
        arm/float_dsp_init_neon.c
        #included by others arm/asm.S
        arm/float_dsp_init_vfp.c
        arm/float_dsp_neon.S
        arm/float_dsp_vfp.S
    )
endif()

list(APPEND avutil_SOURCES 
    ../compat/strtod.c
    ../compat/atomics/pthread/stdatomic.c
    adler32.c
    aes.c
    aes_ctr.c
    ambient_viewing_environment.c
    audio_fifo.c
    avsscanf.c
    avstring.c
    base64.c
    blowfish.c
    bprint.c
    buffer.c
    camellia.c
    cast5.c
    channel_layout.c
    cpu_avutil.c
    crc.c
    csp.c
    des.c
    detection_bbox.c
    dict.c
    display.c
    dovi_meta.c
    downmix_info.c
    encryption_info.c
    error.c
    eval.c   
    fifo.c
    file.c
    file_open.c
    film_grain_params.c
    fixed_dsp_avutil.c
    float_dsp_avutil.c
    float2half.c
    frame.c
    half2float.c
    hash.c
    hdr_dynamic_metadata.c
    hdr_dynamic_vivid_metadata.c
    hmac.c
    hwcontext.c
    imgutils_avutil.c
    integer.c
    intmath.c
    lfg.c
    lls_avutil.c
    log.c
    log2_tab.c
    lzo.c
    macos_kperf.c
    mastering_display_metadata.c
    mathematics.c
    md5.c
    mem.c
    murmur3.c
    opt.c
    parseutils.c
    pca.c
    pixdesc.c
    pixelutils_avutil.c
    random_seed.c
    rational.c
    rc4.c
    reverse.c
    ripemd.c
    samplefmt.c
    sha.c
    sha512.c
    slicethread.c
    spherical.c
    stereo3d.c
    tea.c
    threadmessage.c
    time_avutil.c
    timecode.c
    tree.c
    twofish.c
    tx.c
    tx_double.c
    tx_float_avutil.c
    tx_int32.c
    tx_template.c   
    utils.c
    uuid.c
    version.c
    video_enc_params.c
    xga_font_data.c
    xtea.c
)

if(NOT DEFINED TARGET_OS_ANDROID)
    foreach(ffmpeg_src IN LISTS avutil_SOURCES)
        if(ffmpeg_src MATCHES "\\.asm$")
            set_source_files_properties(${ffmpeg_src} PROPERTIES LANGUAGE ASM_NASM)
        endif()
    endforeach()
endif()

add_library(avutil STATIC ${avutil_SOURCES})

target_include_directories(avutil
  PRIVATE
    ../../libcurl/include
    ../../libcurl/lib
    ../../libplayer-nlc
)

nlc_target_add_supported_warning_options(avutil C
    -Wno-deprecated-declarations
    -Wno-pointer-sign
    -Wno-switch
    -Wno-parentheses
    -Wno-implicit-const-int-float-conversion
)

target_compile_definitions(avutil
    PRIVATE
    ${ffmpeg_DEFINES}
)

if(DEFINED TARGET_OS_WINDOWS)
    target_compile_definitions(avutil
        PRIVATE
        TARGET_OS_WINDOWS
        LIB_STATIC
        WIN64
        _LIB
    )
endif()

