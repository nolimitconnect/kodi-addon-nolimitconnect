
cmake_minimum_required (VERSION 3.22.1)

project(ffmpegopus, LANGUAGES C CXX ASM)

set(ffmpegopus_SOURCES)


if(DEFINED TARGET_OS_WINDOWS)
    list(APPEND ffmpegopus_SOURCES
        celt/x86/x86cpu.c
        celt/x86/x86_celt_map.c
        celt/x86/pitch_sse.c
        celt/x86/pitch_sse2.c
        celt/x86/vq_sse2.c
        celt/x86/celt_lpc_sse4_1.c
        celt/x86/pitch_sse4_1.c
        silk/x86/NSQ_sse4_1.c
        silk/x86/NSQ_del_dec_sse4_1.c
        silk/x86/VAD_sse4_1.c
        silk/x86/VQ_WMat_EC_sse4_1.c
        silk/x86/x86_silk_map.c
        celt/x86/pitch_avx.c
    )
endif()

if(DEFINED TARGET_OS_LINUX)
    if(DEFINED TARGET_CPU_ARM64)
        list(APPEND ffmpegopus_SOURCES
        celt/arm/arm_celt_map.c
        celt/arm/armcpu.c
        celt/arm/celt_mdct_ne10.c
        celt/arm/celt_neon_intr.c
        celt/arm/pitch_neon_intr.c
        silk/arm/arm_silk_map.c
        silk/arm/biquad_alt_neon_intr.c
        silk/arm/LPC_inv_pred_gain_neon_intr.c
        silk/arm/NSQ_del_dec_neon_intr.c
        silk/arm/NSQ_neon.c

        #celt/arm/celt_fft_ne10.c
        #celt/arm/celt_pitch_xcorr_arm.s
        #celt/arm/celt_pitch_xcorr_arm-gnu.S
    )
    else()
        list(APPEND ffmpegopus_SOURCES
            #silk/fixed/x86/vector_ops_FIX_sse4_1.c
            #silk/fixed/x86/burg_modified_FIX_sse4_1.c
            celt/x86/x86cpu.c
            celt/x86/x86_celt_map.c
            celt/x86/pitch_sse.c
            celt/x86/pitch_sse2.c
            celt/x86/vq_sse2.c
            celt/x86/celt_lpc_sse4_1.c
            celt/x86/pitch_sse4_1.c
            silk/x86/NSQ_sse4_1.c
            silk/x86/NSQ_del_dec_sse4_1.c
            silk/x86/VAD_sse4_1.c
            silk/x86/VQ_WMat_EC_sse4_1.c
            silk/x86/x86_silk_map.c
            celt/x86/pitch_avx.c
        )
    endif()
endif()

if(DEFINED TARGET_OS_ANDROID)
    if(DEFINED TARGET_CPU_ARM64)
        list(APPEND ffmpegopus_SOURCES
            celt/arm/arm_celt_map.c
            celt/arm/armcpu.c
            celt/arm/celt_mdct_ne10.c
            celt/arm/celt_neon_intr.c
            celt/arm/pitch_neon_intr.c
            silk/arm/arm_silk_map.c
            silk/arm/biquad_alt_neon_intr.c
            silk/arm/LPC_inv_pred_gain_neon_intr.c
            silk/arm/NSQ_del_dec_neon_intr.c
            silk/arm/NSQ_neon.c

            #celt/arm/celt_fft_ne10.c
            #celt/arm/celt_pitch_xcorr_arm.s
            #celt/arm/celt_pitch_xcorr_arm-gnu.S
        )
    elseif(DEFINED TARGET_CPU_ARM32)
        list(APPEND ffmpegopus_SOURCES
            celt/arm/arm_celt_map.c
            celt/arm/armcpu.c
            celt/arm/celt_mdct_ne10.c
            celt/arm/celt_neon_intr.c
            celt/arm/pitch_neon_intr.c
            silk/arm/arm_silk_map.c
            silk/arm/biquad_alt_neon_intr.c
            silk/arm/LPC_inv_pred_gain_neon_intr.c
            silk/arm/NSQ_del_dec_neon_intr.c
            silk/arm/NSQ_neon.c
            #celt/arm/celt_fft_ne10.c
            #celt/arm/celt_pitch_xcorr_arm.s
            #celt/arm/celt_pitch_xcorr_arm-gnu.S
        )
    endif()
endif()

list(APPEND ffmpegopus_SOURCES 
    celt/bands.c
    celt/celt.c
    celt/celt_encoder.c
    celt/celt_decoder.c
    celt/cwrs.c
    celt/entcode.c
    celt/entdec.c
    celt/entenc.c
    celt/kiss_fft.c
    celt/laplace.c
    celt/mathops.c
    celt/mdct.c
    celt/modes.c
    celt/pitch.c
    celt/celt_lpc.c
    celt/quant_bands.c
    celt/rate.c
    celt/vq.c
    silk/CNG.c
    silk/code_signs.c
    silk/init_decoder.c
    silk/decode_core.c
    silk/decode_frame.c
    silk/decode_parameters.c
    silk/decode_indices.c
    silk/decode_pulses.c
    silk/decoder_set_fs.c
    silk/dec_API.c
    silk/enc_API.c
    silk/encode_indices.c
    silk/encode_pulses.c
    silk/gain_quant.c
    silk/interpolate.c
    silk/LP_variable_cutoff.c
    silk/NLSF_decode.c
    silk/NSQ.c
    silk/NSQ_del_dec.c
    silk/PLC.c
    silk/shell_coder.c
    silk/tables_gain.c
    silk/tables_LTP.c
    silk/tables_NLSF_CB_NB_MB.c
    silk/tables_NLSF_CB_WB.c
    silk/tables_other.c
    silk/tables_pitch_lag.c
    silk/tables_pulses_per_block.c
    silk/VAD.c
    silk/control_audio_bandwidth.c
    silk/quant_LTP_gains.c
    silk/VQ_WMat_EC.c
    silk/HP_variable_cutoff.c
    silk/NLSF_encode.c
    silk/NLSF_VQ.c
    silk/NLSF_unpack.c
    silk/NLSF_del_dec_quant.c
    silk/process_NLSFs.c
    silk/stereo_LR_to_MS.c
    silk/stereo_MS_to_LR.c
    silk/check_control_input.c
    silk/control_SNR.c
    silk/init_encoder.c
    silk/control_codec.c
    silk/A2NLSF.c
    silk/ana_filt_bank_1.c
    silk/biquad_alt.c
    silk/bwexpander_32.c
    silk/bwexpander.c
    silk/debug.c
    silk/decode_pitch.c
    silk/inner_prod_aligned.c
    silk/lin2log.c
    silk/log2lin.c
    silk/LPC_analysis_filter.c
    silk/LPC_inv_pred_gain.c
    silk/table_LSF_cos.c
    silk/NLSF2A.c
    silk/NLSF_stabilize.c
    silk/NLSF_VQ_weights_laroia.c
    silk/pitch_est_tables.c
    silk/resampler.c
    silk/resampler_down2_3.c
    silk/resampler_down2.c
    silk/resampler_private_AR2.c
    silk/resampler_private_down_FIR.c
    silk/resampler_private_IIR_FIR.c
    silk/resampler_private_up2_HQ.c
    silk/resampler_rom.c
    silk/sigm_Q15.c
    silk/sort.c
    silk/sum_sqr_shift.c
    silk/stereo_decode_pred.c
    silk/stereo_encode_pred.c
    silk/stereo_find_predictor.c
    silk/stereo_quant_pred.c
    silk/LPC_fit.c
    silk/float/apply_sine_window_FLP.c
    silk/float/corrMatrix_FLP.c
    silk/float/encode_frame_FLP.c
    silk/float/find_LPC_FLP.c
    silk/float/find_LTP_FLP.c
    silk/float/find_pitch_lags_FLP.c
    silk/float/find_pred_coefs_FLP.c
    silk/float/LPC_analysis_filter_FLP.c
    silk/float/LTP_analysis_filter_FLP.c
    silk/float/LTP_scale_ctrl_FLP.c
    silk/float/noise_shape_analysis_FLP.c
    silk/float/process_gains_FLP.c
    silk/float/regularize_correlations_FLP.c
    silk/float/residual_energy_FLP.c
    silk/float/warped_autocorrelation_FLP.c
    silk/float/wrappers_FLP.c
    silk/float/autocorrelation_FLP.c
    silk/float/burg_modified_FLP.c
    silk/float/bwexpander_FLP.c
    silk/float/energy_FLP.c
    silk/float/inner_product_FLP.c
    silk/float/k2a_FLP.c
    silk/float/LPC_inv_pred_gain_FLP.c
    silk/float/pitch_analysis_core_FLP.c
    silk/float/scale_copy_vector_FLP.c
    silk/float/scale_vector_FLP.c
    silk/float/schur_FLP.c
    silk/float/sort_FLP.c
    src/extensions.c
    src/opus.c
    src/opus_decoder.c
    src/opus_encoder.c
    src/opus_multistream.c
    src/opus_multistream_encoder.c
    src/opus_multistream_decoder.c
    src/repacketizer.c
    src/opus_projection_encoder.c
    src/opus_projection_decoder.c
    src/mapping_matrix.c
    src/analysis.c
    src/mlp.c
    src/mlp_data.c
    OpusCodec.cpp
)

set(ffmpegopus_HEADERS)
list(APPEND ffmpegopus_HEADERS
    config_ffmpegopus.h
    config_opus_aarch64.h
    config_opus_arm.h
    config_opus_x64.h
    celt/arch.h
    celt/bands.h
    celt/celt.h
    celt/celt_lpc.h
    celt/cpu_support.h
    celt/cwrs.h
    celt/ecintrin.h
    celt/entcode.h
    celt/entdec.h
    celt/entenc.h
    celt/fixed_c5x.h
    celt/fixed_c6x.h
    celt/fixed_debug.h
    celt/fixed_generic.h
    celt/float_cast.h
    celt/kiss_fft.h
    celt/laplace.h
    celt/mathops.h
    celt/mdct.h
    celt/meson.build
    celt/mfrngcod.h
    celt/modes.h
    celt/os_support.h
    celt/pitch.h
    celt/quant_bands.h
    celt/rate.h
    celt/stack_alloc.h
    celt/static_modes_fixed.h
    celt/static_modes_fixed_arm_ne10.h
    celt/static_modes_float.h
    celt/static_modes_float_arm_ne10.h
    celt/vq.h
    celt/_kiss_fft_guts.h
    silk/API.h
    silk/control.h
    silk/debug.h
    silk/define.h
    silk/errors.h
    silk/Inlines.h
    silk/MacroCount.h
    silk/MacroDebug.h
    silk/macros.h
    silk/main.h
    silk/NSQ.h
    silk/pitch_est_defines.h
    silk/PLC.h
    silk/resampler_private.h
    silk/resampler_rom.h
    silk/resampler_structs.h
    silk/SigProc_FIX.h
    silk/structs.h
    silk/tables.h
    silk/tuning_parameters.h
    silk/typedef.h
    OpusCodec.h
)

if(NOT DEFINED TARGET_OS_ANDROID)
    foreach(ffmpeg_src IN LISTS ffmpegopus_SOURCES)
        if(ffmpeg_src MATCHES "\\.asm$")
            set_source_files_properties(${ffmpeg_src} PROPERTIES LANGUAGE ASM_NASM)
        endif()
    endforeach()
endif()


add_library(ffmpegopus STATIC ${ffmpegopus_SOURCES} ${ffmpegopus_HEADERS})

if(MSVC)
    # Strip default /Od /RTC1 from debug configuration and apply /O2 /RTC-
    string(REPLACE "/Od" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
    string(REPLACE "/Od" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    string(REPLACE "/RTC1" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
    string(REPLACE "/RTC1" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    
    target_compile_options(ffmpegopus PRIVATE
        $<$<CONFIG:Debug>:/O2 /RTC->
        $<$<CONFIG:Release>:/O2>
    )
else()
    # Override optimization flags for this library
    target_compile_options(ffmpegopus PRIVATE
        $<$<CONFIG:Debug>:-O2>
        $<$<CONFIG:Release>:-O2>
    )
endif()

target_compile_definitions(ffmpegopus
    PRIVATE
    OPUS_BUILD=1
    USE_ALLOCA=1
    HAVE_CONFIG_H=1
    OPUS_HAVE_RTCD=1
)

if(DEFINED TARGET_CPU_X64)
    if(NOT MSVC)
        message(STATUS "Adding opus flags -msse3 -msse4.1")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse3 -msse4.1")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse3 -msse4.1")
    endif()
    target_compile_definitions(ffmpegopus
        PRIVATE
        OPUS_X86_MAY_HAVE_SSE4_1=1
    )

endif()

if(DEFINED TARGET_CPU_ARM64)
    target_compile_definitions(ffmpegopus
        PRIVATE
        CPU_ARM
        OPUS_ARM_PRESUME_NEON=1
        OPUS_ARM_PRESUME_NEON_INTR=1
    )
endif()

target_include_directories(ffmpegopus
    PRIVATE
    ./
    silk
    silk/float
    silk/fixed
    celt
    celt/float
    celt/fixed
    include
    ../
)





