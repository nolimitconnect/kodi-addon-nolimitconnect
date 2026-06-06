cmake_minimum_required(VERSION 3.16)
project(webrtc_apm_lib LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# APM library - header-based with basic utilities, full modules will be added as needed
include_directories(
    BEFORE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/include
    ${CMAKE_CURRENT_SOURCE_DIR}/api
    ${CMAKE_CURRENT_SOURCE_DIR}/common_audio
    ${CMAKE_CURRENT_SOURCE_DIR}/rtc_base
    ${CMAKE_CURRENT_SOURCE_DIR}/system_wrappers/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../abseil-cpp
)

if(DEFINED TARGET_OS_ANDROID AND DEFINED ANDROID_NDK)
    include_directories(BEFORE "${ANDROID_NDK}/sources/android/cpufeatures")
endif()

# Define WebRTC platform macros (Linux)
if(DEFINED TARGET_OS_LINUX  OR DEFINED TARGET_OS_ANDROID)
    add_compile_definitions(
        WEBRTC_LINUX
        WEBRTC_POSIX
        _GNU_SOURCE
        WEBRTC_APM_DEBUG_DUMP=0
        RTC_DISABLE_VP9
    )
    if(DEFINED TARGET_CPU_X64)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-error -fno-exceptions -fno-rtti -fPIC -mavx2 -mfma")
    endif()
    if(DEFINED TARGET_CPU_ARM64)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-error -fno-exceptions -fno-rtti -fPIC")
    endif()
endif()

if(DEFINED TARGET_OS_ANDROID)
    add_compile_definitions(WEBRTC_ANDROID)
endif()

if(DEFINED TARGET_CPU_ARM64)
    add_compile_definitions(WEBRTC_ARCH_ARM64)
endif()

if(DEFINED TARGET_CPU_X64)
    add_compile_definitions(WEBRTC_ARCH_X86_64
        WEBRTC_ARCH_X86_FAMILY
        WEBRTC_ARCH_64_BITS
        WEBRTC_ARCH_LITTLE_ENDIAN)
endif()

if(DEFINED TARGET_OS_WINDOWS)
    add_compile_definitions(
        WEBRTC_WIN
        WEBRTC_APM_DEBUG_DUMP=0
        RTC_DISABLE_VP9
        NOMINMAX
    )
endif()

# Logging stubs + explicit template instantiations
set(LOGGING_STUBS
    "${CMAKE_CURRENT_SOURCE_DIR}/logging_stubs.cc"
)

# Core APM modules
set(APM_MODULE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/WebRtcAec.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/WebRtcAec.cpp"

    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/adaptive_fir_filter.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/adaptive_fir_filter_erl.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/aec_state.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/aec3_common.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/aec3_fft.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/alignment_mixer.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/api_call_jitter_metrics.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/block_buffer.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/block_delay_buffer.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/block_framer.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/block_processor.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/block_processor_metrics.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/clockdrift_detector.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/coarse_filter_update_gain.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/comfort_noise_generator.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/config_selector.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/decimator.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/dominant_nearend_detector.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/downsampled_render_buffer.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/echo_audibility.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/echo_canceller3.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/echo_path_delay_estimator.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/echo_path_variability.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/echo_remover.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/echo_remover_metrics.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/erl_estimator.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/erle_estimator.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/fft_buffer.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/filter_analyzer.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/frame_blocker.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/fullband_erle_estimator.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/matched_filter.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/matched_filter_lag_aggregator.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/moving_average.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/multi_channel_content_detector.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/refined_filter_update_gain.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/render_buffer.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/render_delay_buffer.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/render_delay_controller.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/render_delay_controller_metrics.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/render_signal_analyzer.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/residual_echo_estimator.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/reverb_decay_estimator.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/reverb_frequency_response.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/reverb_model.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/reverb_model_estimator.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/signal_dependent_erle_estimator.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/spectrum_buffer.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/stationarity_estimator.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/subband_erle_estimator.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/subband_nearend_detector.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/subtractor.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/subtractor_output.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/subtractor_output_analyzer.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/suppression_filter.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/suppression_gain.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/transparent_mode.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/agc/agc_manager_direct.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/agc/agc.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/agc/legacy/analog_agc.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/agc/legacy/digital_agc.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/agc/loudness_histogram.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/agc/utility.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/agc2/adaptive_digital_gain_controller.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/agc2/agc2_testing_common.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/agc2/biquad_filter.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/agc2/clipping_predictor.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/agc2/clipping_predictor_level_buffer.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/agc2/compute_interpolated_gain_curve.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/agc2/cpu_features.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/agc2/fixed_digital_level_estimator.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/agc2/gain_applier.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/agc2/input_volume_controller.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/agc2/input_volume_stats_reporter.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/agc2/interpolated_gain_curve.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/agc2/limiter.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/agc2/limiter_db_gain_curve.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/agc2/noise_level_estimator.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/agc2/saturation_protector.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/agc2/saturation_protector_buffer.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/agc2/speech_level_estimator.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/agc2/speech_level_estimator_experimental_impl.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/agc2/speech_level_estimator_impl.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/agc2/speech_probability_buffer.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/agc2/vector_float_frame.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/audio_buffer.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/audio_processing_impl.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/gain_controller2.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/capture_levels_adjuster/audio_samples_scaler.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/capture_levels_adjuster/capture_levels_adjuster.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/capture_mixer/audio_content_analyzer.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/capture_mixer/capture_mixer.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/capture_mixer/channel_content_remixer.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/capture_mixer/dc_levels_estimator.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/capture_mixer/energy_estimator.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/capture_mixer/remixing_logic.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/capture_mixer/saturation_estimator.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/echo_detector/circular_buffer.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/echo_detector/mean_variance_estimator.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/echo_detector/moving_max.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/echo_detector/normalized_covariance_estimator.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/gain_control_impl.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/high_pass_filter.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/include/aec_dump.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec_dump/null_aec_dump_factory.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/include/audio_frame_proxies.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/logging/apm_data_dumper.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/ns/fast_math.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/ns/histograms.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/ns/noise_estimator.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/ns/noise_suppressor.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/ns/ns_fft.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/ns/prior_signal_model.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/ns/prior_signal_model_estimator.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/ns/quantile_noise_estimator.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/ns/signal_model.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/ns/signal_model_estimator.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/ns/speech_probability_estimator.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/ns/suppression_params.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/ns/wiener_filter.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/post_filter.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/residual_echo_detector.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/rms_level.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/splitting_filter.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/three_band_filter_bank.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/utility/cascaded_biquad_filter.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/utility/delay_estimator.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/utility/delay_estimator_wrapper.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/vad/gmm.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/vad/pitch_based_vad.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/vad/pitch_internal.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/vad/pole_zero_filter.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/vad/standalone_vad.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/vad/voice_activity_detector.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/vad/vad_circular_buffer.cc"
)

if(DEFINED TARGET_CPU_X64)
    list(APPEND APM_MODULE_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/adaptive_fir_filter_avx2.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/adaptive_fir_filter_erl_avx2.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/fft_data_avx2.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/matched_filter_avx2.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/aec3/vector_math_avx2.cc"
    )
endif()

# System wrappers (clock, thread safety, etc.)
set(SYSTEM_WRAPPER_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/system_wrappers/source/clock.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/system_wrappers/source/metrics.cc"
)

# CPU feature detection
set(CPU_INFO_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/rtc_base/cpu_info.cc"
)

# Logging and string building utilities
set(RTC_BASE_UTIL_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/rtc_base/strings/string_builder.cc"
)

# Common audio utilities
set(COMMON_AUDIO_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/audio_converter.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/audio_util.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/channel_buffer.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/fir_filter_c.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/fir_filter_factory.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/real_fourier.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/real_fourier_ooura.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/smoothing_filter.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/third_party/ooura/fft_size_256/fft4g.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/third_party/ooura/fft_size_128/ooura_fft.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/wav_file.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/wav_header.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/window_generator.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/resampler/push_resampler.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/resampler/push_sinc_resampler.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/resampler/resampler.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/resampler/sinc_resampler.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/signal_processing/auto_corr_to_refl_coef.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/signal_processing/auto_correlation.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/signal_processing/complex_bit_reverse.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/signal_processing/complex_fft.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/signal_processing/copy_set_operations.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/signal_processing/cross_correlation.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/signal_processing/division_operations.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/signal_processing/dot_product_with_scale.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/signal_processing/downsample_fast.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/signal_processing/energy.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/signal_processing/filter_ar.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/signal_processing/filter_ar_fast_q12.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/signal_processing/filter_ma_fast_q12.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/signal_processing/get_hanning_window.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/signal_processing/get_scaling_square.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/signal_processing/levinson_durbin.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/signal_processing/lpc_to_refl_coef.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/signal_processing/min_max_operations.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/signal_processing/randomization_functions.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/signal_processing/real_fft.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/signal_processing/refl_coef_to_lpc.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/signal_processing/resample.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/signal_processing/resample_48khz.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/signal_processing/resample_by_2.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/signal_processing/resample_by_2_internal.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/signal_processing/resample_fractional.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/signal_processing/spl_inl.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/signal_processing/spl_init.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/signal_processing/spl_sqrt.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/signal_processing/splitting_filter.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/signal_processing/sqrt_of_one_minus_x_squared.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/signal_processing/vector_operations.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/signal_processing/vector_scaling_operations.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/vad/vad.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/vad/vad_core.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/vad/vad_filterbank.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/vad/vad_gmm.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/vad/vad_sp.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/vad/webrtc_vad.c"
)

if(DEFINED TARGET_CPU_X64)
    list(APPEND COMMON_AUDIO_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/fir_filter_avx2.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/fir_filter_sse.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/resampler/sinc_resampler_avx2.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/resampler/sinc_resampler_sse.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/third_party/ooura/fft_size_128/ooura_fft_sse2.cc"
    )
endif()

if(DEFINED TARGET_CPU_ARM64)
    list(APPEND COMMON_AUDIO_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/fir_filter_neon.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/resampler/sinc_resampler_neon.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/common_audio/third_party/ooura/fft_size_128/ooura_fft_neon.cc"
    )
endif()

# API support sources needed by the builtin APM builder and environment
set(APM_API_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/api/audio/audio_processing.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/api/audio/audio_processing_statistics.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/api/audio/builtin_audio_processing_builder.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/api/audio/echo_canceller3_config.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/api/environment/deprecated_global_field_trials.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/api/environment/environment_factory.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/api/field_trials.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/api/rtc_event_log/rtc_event_log.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/api/rtc_event_log/rtc_event_log_factory.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/api/task_queue/task_queue_base.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/api/task_queue/pending_task_safety_flag.cc"
)

if(DEFINED TARGET_OS_WINDOWS)
    list(APPEND APM_API_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/api/task_queue/default_task_queue_factory_win.cc"
    )
endif()

if(DEFINED TARGET_POSIX)
    list(APPEND APM_API_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/api/task_queue/default_task_queue_factory_stdlib.cc"
    )
endif()

# Minimal rtc_base sources required by APM
set(APM_RTC_BASE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/rtc_base/denormal_disabler.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/rtc_base/event.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/rtc_base/log_sinks.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/rtc_base/memory/aligned_malloc.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/rtc_base/race_checker.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/rtc_base/string_encode.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/rtc_base/string_utils.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/rtc_base/synchronization/sequence_checker_internal.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/rtc_base/experiments/field_trial_parser.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/rtc_base/time_utils.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/rtc_base/system_time.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/rtc_base/event_tracer.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/rtc_base/trace_categories.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/rtc_base/synchronization/yield_policy.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/rtc_base/system/file_wrapper.cc"
)

if(DEFINED TARGET_OS_WINDOWS)
    list(APPEND APM_RTC_BASE_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/rtc_base/platform_thread.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/rtc_base/platform_thread_types.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/rtc_base/task_queue_win.cc"
    )
endif()

if(DEFINED TARGET_POSIX)
    list(APPEND APM_RTC_BASE_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/rtc_base/platform_thread.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/rtc_base/platform_thread_types.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/rtc_base/task_queue_stdlib.cc"
    )
endif()

# Stub sources for removed RNNoise-dependent components
set(APM_STUB_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/stubs/field_trials_registry_stub.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/stubs/vad_audio_proc_stub.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/stubs/vad_wrapper_stub.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/stubs/warn_current_thread_is_deadlocked_stub.cc"
)

set(APM_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/apm_wrapper.cc"
    ${LOGGING_STUBS}
    ${APM_MODULE_SOURCES}
    ${COMMON_AUDIO_SOURCES}
    ${APM_API_SOURCES}
    ${APM_RTC_BASE_SOURCES}
    ${APM_STUB_SOURCES}
    ${SYSTEM_WRAPPER_SOURCES}
    ${CPU_INFO_SOURCES}
    ${RTC_BASE_UTIL_SOURCES}
)

if(DEFINED TARGET_OS_ANDROID AND DEFINED ANDROID_NDK)
    list(APPEND APM_SOURCES
        "${ANDROID_NDK}/sources/android/cpufeatures/cpu-features.c"
    )
endif()

# Future phases:
# file(GLOB VAD_SOURCES ...)         # Voice Activity Detection

# Create static library
add_library(webrtc-aec STATIC ${APM_SOURCES})

if(MSVC)
    # Strip default /Od /RTC1 from debug configuration and apply /O2 /RTC-
    string(REPLACE "/Od" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
    string(REPLACE "/Od" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    string(REPLACE "/RTC1" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
    string(REPLACE "/RTC1" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    
    target_compile_options(webrtc-aec PRIVATE
        $<$<CONFIG:Debug>:/O2 /RTC->
        $<$<CONFIG:Release>:/O2>
    )
else()
    target_compile_options(webrtc-aec PRIVATE
        $<$<CONFIG:Debug>:-O2>
        $<$<CONFIG:Release>:-O2>
    )
endif()


target_compile_definitions(webrtc-aec
    PRIVATE
    WEBRTC_APM_DEBUG_DUMP=0
)

if(DEFINED TARGET_POSIX)
    target_compile_definitions(webrtc-aec
        PRIVATE
        WEBRTC_POSIX
    )
endif()

if(DEFINED TARGET_OS_WINDOWS)
    target_compile_definitions(webrtc-aec
        PRIVATE
        WEBRTC_WIN
    )
endif()

if(DEFINED TARGET_OS_APPLE)
    target_compile_definitions(webrtc-aec
        PRIVATE
        WEBRTC_MAC
    )
endif()

if(DEFINED TARGET_CPU_ARM64)
    target_compile_definitions(webrtc-aec
        PRIVATE
        WEBRTC_HAS_NEON
    )
endif()



# Set public include directories for users of this library
target_include_directories(webrtc-aec PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/include
    ${CMAKE_CURRENT_SOURCE_DIR}/api
    ${CMAKE_CURRENT_SOURCE_DIR}/../abseil-cpp
)

# Library output name
set_target_properties(webrtc-aec PROPERTIES
    OUTPUT_NAME webrtc_apm
)







