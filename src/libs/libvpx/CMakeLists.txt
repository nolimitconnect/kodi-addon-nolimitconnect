
cmake_minimum_required (VERSION 3.16.3)

project(vpx, LANGUAGES C CXX)

# Use NASM/YASM for x64 assembly sources on desktop targets.
# Android ARM builds in this tree use C/NEON sources and should not initialize ASM here.
if(DEFINED TARGET_CPU_X64 AND NOT DEFINED TARGET_OS_ANDROID)
    enable_language(ASM_NASM)
    set(CMAKE_ASM_NASM_SOURCE_FILE_EXTENSIONS nasm asm)
    if(DEFINED TARGET_OS_WINDOWS)
        set(CMAKE_ASM_NASM_OBJECT_FORMAT win64)
    elseif(DEFINED TARGET_OS_LINUX)
        set(CMAKE_ASM_NASM_OBJECT_FORMAT elf64)
    endif()
endif()

set(vpx_SOURCES)

list(APPEND vpx_SOURCES 
    args.c
    ivfdec.c
    ivfenc.c
    md5_utils.c
    rate_hist.c
    video_reader.c
    video_writer.c
    # vpxdec.c // do not include when building windows target
    #vpxenc.c
    vpxstats.c
    warnings.c
    y4menc.c
    y4minput.c

    vp8_rtcd.h
    vp9_rtcd.h
    vpx_dsp_rtcd.h
    vpx_scale_rtcd.h

    vp8/vp8_cx_iface.c
    vp8/vp8_dx_iface.c
    vp8/vp8_ratectrl_rtc.cc
    vp8/vp8_ratectrl_rtc.h

    vp8/common/alloccommon.c
    vp8/common/alloccommon.h

    vp8/common/blockd.c
    vp8/common/blockd.h
    vp8/common/coefupdateprobs.h
    vp8/common/common.h
    vp8/common/context.c
    vp8/common/debugmodes.c
    vp8/common/default_coef_probs.h
    vp8/common/dequantize.c
    vp8/common/entropy.c
    vp8/common/entropy.h
    vp8/common/entropymode.c
    vp8/common/entropymode.h
    vp8/common/entropymv.c
    vp8/common/entropymv.h
    vp8/common/extend.c
    vp8/common/extend.h
    vp8/common/filter.c
    vp8/common/filter.h
    vp8/common/findnearmv.c
    vp8/common/findnearmv.h

    vp8/common/generic/systemdependent.c

    vp8/common/header.h
    vp8/common/idctllm.c
    vp8/common/idct_blk.c
    vp8/common/invtrans.h
    vp8/common/loopfilter.h
    vp8/common/loopfilter_filters.c
    vp8/common/mbpitch.c

    # no postproc
    #vp8/common/mfqe.c

    vp8/common/modecont.c
    vp8/common/modecont.h
    vp8/common/mv.h
    vp8/common/onyx.h
    vp8/common/onyxc_int.h
    vp8/common/onyxd.h

    # no postproc
    #vp8/common/postproc.c
    #vp8/common/postproc.h

    vp8/common/ppflags.h
    vp8/common/quant_common.c
    vp8/common/quant_common.h
    vp8/common/reconinter.c
    vp8/common/reconinter.h
    vp8/common/reconintra.c
    vp8/common/reconintra.h
    vp8/common/reconintra4x4.c
    vp8/common/reconintra4x4.h

    vp8/common/setupintrarecon.c
    vp8/common/setupintrarecon.h
    vp8/common/swapyv12buffer.c
    vp8/common/swapyv12buffer.h
    vp8/common/systemdependent.h
    vp8/common/threading.h
    vp8/common/treecoder.c
    vp8/common/treecoder.h
    vp8/common/vp8_entropymodedata.h
    vp8/common/vp8_loopfilter.c
    vp8/common/vp8_skin_detection.c
    vp8/common/vp8_skin_detection.h

    vp8/decoder/dboolhuff.c
    vp8/decoder/dboolhuff.h
    vp8/decoder/decodeframe.c
    vp8/decoder/decodemv.c
    vp8/decoder/decodemv.h
    vp8/decoder/decoderthreading.h
    vp8/decoder/detokenize.c
    vp8/decoder/detokenize.h
    vp8/decoder/ec_types.h

    # not used in any configuration
    #vp8/decoder/error_concealment.c
    #vp8/decoder/error_concealment.h

    vp8/decoder/onyxd_if.c
    vp8/decoder/onyxd_int.h
    vp8/decoder/threading.c
    vp8/decoder/treereader.h

    #vp8/encoder/bitstream.c
    #vp8/encoder/bitstream.h
    #vp8/encoder/block.h
    #vp8/encoder/boolhuff.c
    #vp8/encoder/boolhuff.h
    #vp8/encoder/copy_c.c
    #vp8/encoder/dct.c
    #vp8/encoder/dct_value_cost.h
    #vp8/encoder/dct_value_tokens.h
    #vp8/encoder/defaultcoefcounts.h

    #vp8/encoder/encodeframe.c
    #vp8/encoder/encodeframe.h
    #vp8/encoder/encodeintra.c
    #vp8/encoder/encodeintra.h
    #vp8/encoder/encodemb.c
    #vp8/encoder/encodemb.h
    #vp8/encoder/encodemv.c
    #vp8/encoder/encodemv.h
    #vp8/encoder/ethreading.c
    #vp8/encoder/ethreading.h
    #vp8/encoder/firstpass.c
    #vp8/encoder/firstpass.h
    #vp8/encoder/lookahead.c
    #vp8/encoder/lookahead.h
    #vp8/encoder/mcomp.c
    #vp8/encoder/mcomp.h
    #vp8/encoder/modecosts.c
    #vp8/encoder/modecosts.h

    # not used in any configuration
    #vp8/encoder/mr_dissim.c
    #vp8/encoder/mr_dissim.h

    #vp8/encoder/onyx_if.c
    #vp8/encoder/onyx_int.h
    #vp8/encoder/pickinter.c
    #vp8/encoder/pickinter.h
    #vp8/encoder/picklpf.c
    #vp8/encoder/picklpf.h
    #vp8/encoder/quantize.h
    #vp8/encoder/ratectrl.c
    #vp8/encoder/ratectrl.h
    #vp8/encoder/rdopt.c
    #vp8/encoder/rdopt.h
    #vp8/encoder/segmentation.c
    #vp8/encoder/segmentation.h
    #vp8/encoder/temporal_filter.c
    #vp8/encoder/temporal_filter.h
    #vp8/encoder/tokenize.c
    #vp8/encoder/tokenize.h
    #vp8/encoder/treewriter.c
    #vp8/encoder/treewriter.h
    #vp8/encoder/vp8_quantize.c


    vp9/ratectrl_rtc.cc
    vp9/ratectrl_rtc.h

    # not used in any configuration
    #vp9/simple_encode.cc
    #vp9/simple_encode.h

    #vp9/vp9_cx_iface.c
    #vp9/vp9_cx_iface.h
    vp9/vp9_dx_iface.c
    vp9/vp9_dx_iface.h
    vp9/vp9_iface_common.c
    vp9/vp9_iface_common.h

    vp9/common/vp9_alloccommon.c
    vp9/common/vp9_alloccommon.h
    vp9/common/vp9_blockd.c
    vp9/common/vp9_blockd.h
    vp9/common/vp9_common.h
    vp9/common/vp9_common_data.c
    vp9/common/vp9_common_data.h
    vp9/common/vp9_debugmodes.c
    vp9/common/vp9_entropy.c
    vp9/common/vp9_entropy.h
    vp9/common/vp9_entropymode.c
    vp9/common/vp9_entropymode.h
    vp9/common/vp9_entropymv.c
    vp9/common/vp9_entropymv.h
    vp9/common/vp9_enums.h
    vp9/common/vp9_filter.c
    vp9/common/vp9_filter.h
    vp9/common/vp9_frame_buffers.c
    vp9/common/vp9_frame_buffers.h
    vp9/common/vp9_idct.c
    vp9/common/vp9_idct.h
    vp9/common/vp9_loopfilter.c
    vp9/common/vp9_loopfilter.h

    # no postproc
    #vp9/common/vp9_mfqe.c
    #vp9/common/vp9_mfqe.h

    vp9/common/vp9_mv.h
    vp9/common/vp9_mvref_common.c
    vp9/common/vp9_mvref_common.h
    vp9/common/vp9_onyxc_int.h
    vp9/common/vp9_postproc.c
    vp9/common/vp9_postproc.h
    vp9/common/vp9_ppflags.h
    vp9/common/vp9_pred_common.c
    vp9/common/vp9_pred_common.h
    vp9/common/vp9_quant_common.c
    vp9/common/vp9_quant_common.h
    vp9/common/vp9_reconinter.c
    vp9/common/vp9_reconinter.h
    vp9/common/vp9_reconintra.c
    vp9/common/vp9_reconintra.h

    vp9/common/vp9_rtcd.c

    vp9/common/vp9_scale.c
    vp9/common/vp9_scale.h
    vp9/common/vp9_scan.c
    vp9/common/vp9_scan.h
    vp9/common/vp9_seg_common.c
    vp9/common/vp9_seg_common.h
    vp9/common/vp9_thread_common.c
    vp9/common/vp9_thread_common.h
    vp9/common/vp9_tile_common.c
    vp9/common/vp9_tile_common.h

    vp9/decoder/vp9_decodeframe.c
    vp9/decoder/vp9_decodeframe.h
    vp9/decoder/vp9_decodemv.c
    vp9/decoder/vp9_decodemv.h
    vp9/decoder/vp9_decoder.c
    vp9/decoder/vp9_decoder.h
    vp9/decoder/vp9_detokenize.c
    vp9/decoder/vp9_detokenize.h
    vp9/decoder/vp9_dsubexp.c
    vp9/decoder/vp9_dsubexp.h
    vp9/decoder/vp9_job_queue.c
    vp9/decoder/vp9_job_queue.h

    #vp9/encoder/vp9_alt_ref_aq.c
    #vp9/encoder/vp9_alt_ref_aq.h
    #vp9/encoder/vp9_aq_360.c
    #vp9/encoder/vp9_aq_360.h
    #vp9/encoder/vp9_aq_complexity.c
    #vp9/encoder/vp9_aq_complexity.h
    #vp9/encoder/vp9_aq_cyclicrefresh.c
    #vp9/encoder/vp9_aq_cyclicrefresh.h
    #vp9/encoder/vp9_aq_variance.c
    #vp9/encoder/vp9_aq_variance.h
    #vp9/encoder/vp9_bitstream.c
    #vp9/encoder/vp9_bitstream.h
    #vp9/encoder/vp9_block.h
    #vp9/encoder/vp9_blockiness.c
    #vp9/encoder/vp9_blockiness.h
    #vp9/encoder/vp9_context_tree.c
    #vp9/encoder/vp9_context_tree.h
    #vp9/encoder/vp9_cost.c
    #vp9/encoder/vp9_cost.h
    #vp9/encoder/vp9_dct.c

    #vp9/encoder/vp9_encodeframe.c
    #vp9/encoder/vp9_encodeframe.h
    #vp9/encoder/vp9_encodemb.c
    #vp9/encoder/vp9_encodemb.h
    #vp9/encoder/vp9_encodemv.c
    #vp9/encoder/vp9_encodemv.h
    #vp9/encoder/vp9_encoder.c
    #vp9/encoder/vp9_encoder.h
    #vp9/encoder/vp9_ethread.c
    #vp9/encoder/vp9_ethread.h
    #vp9/encoder/vp9_extend.c
    #vp9/encoder/vp9_extend.h
    #vp9/encoder/vp9_ext_ratectrl.c
    #vp9/encoder/vp9_ext_ratectrl.h
    #vp9/encoder/vp9_firstpass.c
    #vp9/encoder/vp9_firstpass.h
    # not in v1.13.1 vp9/encoder/vp9_firstpass_stats.h
    #vp9/encoder/vp9_frame_scale.c
    #vp9/encoder/vp9_job_queue.h
    #vp9/encoder/vp9_lookahead.c
    #vp9/encoder/vp9_lookahead.h
    #vp9/encoder/vp9_mbgraph.c
    #vp9/encoder/vp9_mbgraph.h
    #vp9/encoder/vp9_mcomp.c
    #vp9/encoder/vp9_mcomp.h
    #vp9/encoder/vp9_multi_thread.c
    #vp9/encoder/vp9_multi_thread.h
    #vp9/encoder/vp9_noise_estimate.c
    #vp9/encoder/vp9_noise_estimate.h
    #vp9/encoder/vp9_non_greedy_mv.c
    #vp9/encoder/vp9_non_greedy_mv.h
    #vp9/encoder/vp9_partition_models.h
    #vp9/encoder/vp9_picklpf.c
    #vp9/encoder/vp9_picklpf.h
    #vp9/encoder/vp9_pickmode.c
    #vp9/encoder/vp9_pickmode.h
    #vp9/encoder/vp9_quantize.c
    #vp9/encoder/vp9_quantize.h
    #vp9/encoder/vp9_ratectrl.c
    #vp9/encoder/vp9_ratectrl.h
    #vp9/encoder/vp9_rd.c
    #vp9/encoder/vp9_rd.h
    #vp9/encoder/vp9_rdopt.c
    #vp9/encoder/vp9_rdopt.h
    #vp9/encoder/vp9_resize.c
    #vp9/encoder/vp9_resize.h
    #vp9/encoder/vp9_segmentation.c
    #vp9/encoder/vp9_segmentation.h
    #vp9/encoder/vp9_skin_detection.c
    #vp9/encoder/vp9_skin_detection.h
    #vp9/encoder/vp9_speed_features.c
    #vp9/encoder/vp9_speed_features.h
    #vp9/encoder/vp9_subexp.c
    #vp9/encoder/vp9_subexp.h
    #vp9/encoder/vp9_svc_layercontext.c
    #vp9/encoder/vp9_svc_layercontext.h
    #vp9/encoder/vp9_temporal_filter.c
    #vp9/encoder/vp9_temporal_filter.h
    # not in v1.13.1 vp9/encoder/vp9_temporal_filter_constants.h
    #vp9/encoder/vp9_tokenize.c
    #vp9/encoder/vp9_tokenize.h
    # not in v1.13.1 vp9/encoder/vp9_tpl_model.c
    # not in v1.13.1 vp9/encoder/vp9_tpl_model.h
    #vp9/encoder/vp9_treewriter.c
    #vp9/encoder/vp9_treewriter.h

    vp9/decoder/vp9_decodeframe.c
    vp9/decoder/vp9_decodeframe.h
    vp9/decoder/vp9_decodemv.c
    vp9/decoder/vp9_decodemv.h
    vp9/decoder/vp9_decoder.c
    vp9/decoder/vp9_decoder.h
    vp9/decoder/vp9_detokenize.c
    vp9/decoder/vp9_detokenize.h
    vp9/decoder/vp9_dsubexp.c
    vp9/decoder/vp9_dsubexp.h
    vp9/decoder/vp9_job_queue.c
    vp9/decoder/vp9_job_queue.h

    vpx/src/vpx_codec.c
    vpx/src/vpx_decoder.c
    vpx/src/vpx_encoder.c
    vpx/src/vpx_image.c

    vpx_dsp/add_noise.c
    vpx_dsp/avg.c
    vpx_dsp/bitreader.c
    vpx_dsp/bitreader.h
    vpx_dsp/bitreader_buffer.c
    vpx_dsp/bitreader_buffer.h
    vpx_dsp/bitwriter.c
    vpx_dsp/bitwriter.h
    vpx_dsp/bitwriter_buffer.c
    vpx_dsp/bitwriter_buffer.h
    vpx_dsp/deblock.c
    vpx_dsp/fastssim.c
    vpx_dsp/fwd_txfm.c
    vpx_dsp/fwd_txfm.h
    vpx_dsp/intrapred.c
    vpx_dsp/inv_txfm.c
    vpx_dsp/inv_txfm.h
    vpx_dsp/loopfilter.c
    vpx_dsp/postproc.h
    vpx_dsp/prob.c
    vpx_dsp/prob.h
    vpx_dsp/psnr.c
    vpx_dsp/psnr.h
    vpx_dsp/psnrhvs.c
    vpx_dsp/quantize.c
    vpx_dsp/quantize.h
    vpx_dsp/sad.c
    vpx_dsp/skin_detection.c
    vpx_dsp/skin_detection.h
    # not in v1.13.1 vpx_dsp/sse.c

    # seems to be for statistics
    #vpx_dsp/ssim.c
    #vpx_dsp/ssim.h

    vpx_dsp/subtract.c
    # not in v1.13.1 vpx_dsp/sum_squares.c
    vpx_dsp/txfm_common.h
    vpx_dsp/variance.c
    vpx_dsp/variance.h
    vpx_dsp/vpx_convolve.c
    vpx_dsp/vpx_convolve.h
    vpx_dsp/vpx_dsp_common.h

    vpx_dsp/vpx_filter.h

    vpx_mem/vpx_mem.c
    vpx_mem/vpx_mem.h

    # not in v1.13.1 #vpx_ports/aarch32_cpudetect.c
    # not in v1.13.1 #vpx_ports/aarch64_cpudetect.c
    vpx_ports/arm.h
    # not in v1.13.1 vpx_ports/arm_cpudetect.h
    vpx_ports/arm_cpudetect.c # only in v1.13.1
    vpx_ports/asmdefs_mmi.h
    vpx_ports/bitops.h
    vpx_ports/compiler_attributes.h
    vpx_ports/emmintrin_compat.h

    #vpx_ports/float_control_word.asm
    vpx_ports/mem.h
    vpx_ports/mem_ops.h
    vpx_ports/mem_ops_aligned.h
    vpx_ports/static_assert.h
    vpx_ports/system_state.h
    vpx_ports/vpx_once.h
    vpx_ports/vpx_timer.h
    vpx_ports/x86.h
    #vpx_ports/x86_abi_support.asm

    vpx_scale/vpx_scale.h

    vpx_scale/yv12config.h

    vpx_scale/generic/gen_scalers.c
    vpx_scale/generic/vpx_scale.c
    vpx_scale/generic/yv12config.c
    vpx_scale/generic/yv12extend.c

    vpx_util/endian_inl.h
    vpx_util/vpx_atomics.h
    vpx_util/vpx_debug_util.c
    vpx_util/vpx_debug_util.h
    # not in v1.13.1 vpx_util/vpx_pthread.h
    vpx_util/vpx_thread.c
    vpx_util/vpx_thread.h
    vpx_util/vpx_timestamp.h
    #only if have libyuv
    #vpx_util/vpx_write_yuv_frame.c
    #vpx_util/vpx_write_yuv_frame.h

    vp9/common/vp9_rtcd.c
    vp8/common/rtcd.c
    vpx_dsp/vpx_dsp_rtcd.c
    vpx_scale/vpx_scale_rtcd.c

    #vpxdec.c
)

# if(NOT DEFINED TARGET_OS_WINDOWS)
#     list(APPEND vpx_SOURCES
#         vpxdec.c
#     )

# endif()


if(DEFINED TARGET_CPU_X64)
    list(APPEND vpx_SOURCES 

        vp9/common/x86/vp9_idct_intrin_sse2.c
        #vp9/common/x86/vp9_highbd_iht4x4_add_sse4.c

        vp8/common/x86/bilinear_filter_sse2.c
        vp8/common/x86/dequantize_mmx.asm
        vp8/common/x86/idct_blk_mmx.c
        vp8/common/x86/idct_blk_sse2.c
        vp8/common/x86/idctllm_mmx.asm
        vp8/common/x86/idctllm_sse2.asm
        vp8/common/x86/iwalsh_sse2.asm
        vp8/common/x86/loopfilter_block_sse2_x86_64.asm
        vp8/common/x86/loopfilter_sse2.asm
        vp8/common/x86/loopfilter_x86.c

        # no postproc
        #vp8/common/x86/mfqe_sse2.asm

        vp8/common/x86/recon_mmx.asm
        vp8/common/x86/recon_sse2.asm
        vp8/common/x86/subpixel_mmx.asm
        vp8/common/x86/subpixel_sse2.asm
        vp8/common/x86/subpixel_ssse3.asm
        vp8/common/x86/vp8_asm_stubs.c

        #vp8/encoder/x86/block_error_sse2.asm
        #vp8/encoder/x86/copy_sse2.asm
        #vp8/encoder/x86/copy_sse3.asm
        #vp8/encoder/x86/dct_sse2.asm


        #vp8/encoder/x86/fwalsh_sse2.asm
        #vp8/encoder/x86/quantize_sse4.c
        #vp8/encoder/x86/temporal_filter_apply_sse2.asm
        #vp8/encoder/x86/vp8_enc_stubs_sse2.c
        #vp8/encoder/x86/vp8_quantize_sse2.c
        #vp8/encoder/x86/vp8_quantize_ssse3.c

        #highbd files are for if CONFIG_VP9_HIGHBITDEPTH=1
        #vp9/common/x86/vp9_highbd_iht16x16_add_sse4.c
        #vp9/common/x86/vp9_highbd_iht4x4_add_sse4.c
        #vp9/common/x86/vp9_highbd_iht8x8_add_sse4.c

        vp9/common/x86/vp9_idct_intrin_sse2.c

        # no postproc
        #vp9/common/x86/vp9_mfqe_sse2.asm

        #highbd files are for if CONFIG_VP9_HIGHBITDEPTH=1
        #vp9/encoder/x86/highbd_temporal_filter_avx2.c
        #vp9/encoder/x86/highbd_temporal_filter_sse4.c
        #vp9/encoder/x86/highbd_temporal_filter_ssse3.c

        # not in v1.13.1 vp9/encoder/x86/temporal_filter_avx2.c
        #vp9/encoder/x86/temporal_filter_sse4.c
        # not in v1.13.1 vp9/encoder/x86/temporal_filter_ssse3.c
        #vp9/encoder/x86/vp9_dct_intrin_sse2.c
        #vp9/encoder/x86/vp9_dct_sse2.asm

        # not in v1.13.1 vp9/encoder/x86/vp9_error_avx2.c
        # not in v1.13.1 vp9/encoder/x86/vp9_diamond_search_sad_avx2.c # only in v1.13.1
        #vp9/encoder/x86/vp9_error_sse2.asm
        # not in v1.13.1 vp9/encoder/x86/vp9_frame_scale_ssse3.c
        #vp9/encoder/x86/vp9_highbd_block_error_intrin_sse2.c
        #vp9/encoder/x86/vp9_quantize_avx2.c
        #vp9/encoder/x86/vp9_quantize_sse2.c
        #vp9/encoder/x86/vp9_quantize_ssse3.c

        vpx_dsp/x86/add_noise_sse2.asm
        vpx_dsp/x86/avg_intrin_avx2.c
        vpx_dsp/x86/avg_intrin_sse2.c
        # not in v1.13.1 vpx_dsp/x86/avg_pred_avx2.c
        vpx_dsp/x86/avg_pred_sse2.c
        vpx_dsp/x86/avg_ssse3_x86_64.asm
        vpx_dsp/x86/bitdepth_conversion_avx2.h
        vpx_dsp/x86/bitdepth_conversion_sse2.asm
        vpx_dsp/x86/bitdepth_conversion_sse2.h
        vpx_dsp/x86/convolve_avx2.h
        vpx_dsp/x86/convolve.h
        vpx_dsp/x86/convolve_sse2.h
        vpx_dsp/x86/convolve_ssse3.h
        vpx_dsp/x86/deblock_sse2.asm
        vpx_dsp/x86/fwd_dct32x32_impl_avx2.h
        vpx_dsp/x86/fwd_dct32x32_impl_sse2.h
        vpx_dsp/x86/fwd_txfm_avx2.c
        vpx_dsp/x86/fwd_txfm_impl_sse2.h
        vpx_dsp/x86/fwd_txfm_sse2.c
        vpx_dsp/x86/fwd_txfm_sse2.h
        vpx_dsp/x86/fwd_txfm_ssse3_x86_64.asm

        #highbd files are for if CONFIG_VP9_HIGHBITDEPTH=1
        #vpx_dsp/x86/highbd_convolve_avx2.c
        #vpx_dsp/x86/highbd_idct16x16_add_sse2.c
        #vpx_dsp/x86/highbd_idct16x16_add_sse4.c
        #vpx_dsp/x86/highbd_idct32x32_add_sse2.c
        #vpx_dsp/x86/highbd_idct32x32_add_sse4.c
        #vpx_dsp/x86/highbd_idct4x4_add_sse2.c
        #vpx_dsp/x86/highbd_idct4x4_add_sse4.c
        #vpx_dsp/x86/highbd_idct8x8_add_sse2.c
        #vpx_dsp/x86/highbd_idct8x8_add_sse4.c
        #vpx_dsp/x86/highbd_intrapred_intrin_sse2.c
        #vpx_dsp/x86/highbd_intrapred_intrin_ssse3.c
        #vpx_dsp/x86/highbd_intrapred_sse2.asm
        #vpx_dsp/x86/highbd_inv_txfm_sse2.h
        #vpx_dsp/x86/highbd_inv_txfm_sse4.h
        #vpx_dsp/x86/highbd_loopfilter_sse2.c
        #vpx_dsp/x86/highbd_quantize_intrin_avx2.c
        #vpx_dsp/x86/highbd_quantize_intrin_sse2.c
        #vpx_dsp/x86/highbd_sad4d_avx2.c
        #vpx_dsp/x86/highbd_sad4d_sse2.asm
        #vpx_dsp/x86/highbd_sad_avx2.c
        #vpx_dsp/x86/highbd_sad_sse2.asm
        #vpx_dsp/x86/highbd_subpel_variance_impl_sse2.asm
        #vpx_dsp/x86/highbd_variance_impl_sse2.asm
        #vpx_dsp/x86/highbd_variance_sse2.c

        vpx_dsp/x86/intrapred_sse2.asm
        vpx_dsp/x86/intrapred_ssse3.asm
        # not in v1.13.1 vpx_dsp/x86/inv_txfm_avx2.c
        vpx_dsp/x86/inv_txfm_sse2.c
        vpx_dsp/x86/inv_txfm_sse2.h
        vpx_dsp/x86/inv_txfm_ssse3.c
        vpx_dsp/x86/inv_txfm_ssse3.h
        vpx_dsp/x86/inv_wht_sse2.asm
        vpx_dsp/x86/loopfilter_avx2.c
        vpx_dsp/x86/loopfilter_sse2.c
        vpx_dsp/x86/mem_sse2.h
        vpx_dsp/x86/post_proc_sse2.c
        vpx_dsp/x86/quantize_avx2.c
        vpx_dsp/x86/quantize_avx.c
        vpx_dsp/x86/quantize_sse2.c
        vpx_dsp/x86/quantize_sse2.h
        vpx_dsp/x86/quantize_ssse3.c
        vpx_dsp/x86/quantize_ssse3.h

        vpx_dsp/x86/sad4d_avx2.c
        vpx_dsp/x86/sad4d_avx512.c
        vpx_dsp/x86/sad4d_sse2.asm
        vpx_dsp/x86/sad_avx2.c
        vpx_dsp/x86/sad_sse2.asm
        # not in v1.13.1 vpx_dsp/x86/sse_avx2.c
        # not in v1.13.1 vpx_dsp/x86/sse_sse4.c
        vpx_dsp/x86/ssim_opt_x86_64.asm
        vpx_dsp/x86/subpel_variance_sse2.asm
        vpx_dsp/x86/subtract_avx2.c
        vpx_dsp/x86/subtract_sse2.asm
        vpx_dsp/x86/sum_squares_sse2.c
        vpx_dsp/x86/transpose_sse2.h
        vpx_dsp/x86/txfm_common_sse2.h
        vpx_dsp/x86/variance_avx2.c
        vpx_dsp/x86/variance_sse2.c
        vpx_dsp/x86/vpx_convolve_copy_sse2.asm
        vpx_dsp/x86/vpx_high_subpixel_8t_sse2.asm
        vpx_dsp/x86/vpx_high_subpixel_bilinear_sse2.asm
        vpx_dsp/x86/vpx_subpixel_4t_intrin_sse2.c
        vpx_dsp/x86/vpx_subpixel_8t_intrin_avx2.c
        vpx_dsp/x86/vpx_subpixel_8t_intrin_ssse3.c
        vpx_dsp/x86/vpx_subpixel_8t_sse2.asm
        vpx_dsp/x86/vpx_subpixel_8t_ssse3.asm
        vpx_dsp/x86/vpx_subpixel_bilinear_sse2.asm
        vpx_dsp/x86/vpx_subpixel_bilinear_ssse3.asm

        vpx_ports/emms_mmx.asm
        vpx_ports/emms_mmx.c
        vpx_ports/msvc.h  # only in v1.13.1
        #vp8/encoder/denoising.c
        #vp8/encoder/denoising.h
        #vp8/encoder/x86/denoising_sse2.c

        #vp9/encoder/vp9_denoiser.c
        #vp9/encoder/vp9_denoiser.h

        #vp9/encoder/x86/vp9_denoiser_sse2.c
        #vp9/encoder/x86/vp9_diamond_search_sad_avx.c
        #vp9/encoder/x86/vp9_error_avx2.c
        #vp9/encoder/x86/vp9_error_sse2.asm
        #vp9/encoder/x86/vp9_frame_scale_ssse3.c

        #vp9/encoder/x86/vp9_highbd_block_error_intrin_sse2.c

        #vp9/encoder/x86/vp9_quantize_avx2.c
        #vp9/encoder/x86/vp9_quantize_sse2.c
        #vp9/encoder/x86/vp9_quantize_ssse3.c

        vpx_dsp/sse.c
    )

    if(DEFINED TARGET_OS_WINDOWS)
        list(REMOVE_ITEM vpx_SOURCES
            vpx_ports/emms_mmx.c
        )

        # Windows add
        list(APPEND vpx_SOURCES
            vpx_config_win_x64.c
            vpx_config_win_x64.h
            vpx_config_win_x64.asm

            vp8_rtcd_win_x64.h
            vp9_rtcd_win_x64.h
            vpx_dsp_rtcd_win_x64.h
            vpx_scale_rtcd_win_x64.h

            vp8/common/postproc.c
            vp8/common/mfqe.c
            vp8/common/x86/mfqe_sse2.asm

        )
    else()
        # Linux only add
        if(DEFINED TARGET_OS_LINUX)
            list(APPEND vpx_SOURCES

                vpx_config_linux_x64.h
                vpx_config_linux_x64.c
                vpx_config_linux_x64.asm
                vpx_config_linux_x64.h
                vpx_config_linux_x64.c

                vp8/common/x86/mfqe_sse2.asm
                vp8/common/mfqe.c
                vp8/common/postproc.h
                vp8/common/postproc.c
                vp8_rtcd_linux_x64.h
                vp9_rtcd_linux_x64.h
                vpx_dsp_rtcd_linux_x64.h
                vpx_scale_rtcd_linux_x64.h
            )
        endif()
    endif()
else()
    list(APPEND vpx_SOURCES 
        vp8_rtcd_arm64.h
        vp9_rtcd_arm64.h
        vpx_dsp_rtcd_arm64.h
        vpx_scale_rtcd_arm64.h


        vp8/common/arm/loopfilter_arm.c
        vp8/common/arm/neon/bilinearpredict_neon.c
        vp8/common/arm/neon/copymem_neon.c
        vp8/common/arm/neon/dc_only_idct_add_neon.c
        vp8/common/arm/neon/dequantizeb_neon.c
        vp8/common/arm/neon/dequant_idct_neon.c
        vp8/common/arm/neon/idct_blk_neon.c

        vp8/common/arm/neon/iwalsh_neon.c
        vp8/common/arm/neon/loopfiltersimplehorizontaledge_neon.c
        vp8/common/arm/neon/loopfiltersimpleverticaledge_neon.c
        vp8/common/arm/neon/mbloopfilter_neon.c
        vp8/common/arm/neon/shortidct4x4llm_neon.c
        vp8/common/arm/neon/sixtappredict_neon.c
        vp8/common/arm/neon/vp8_loopfilter_neon.c

        #vp8/encoder/arm/neon/fastquantizeb_neon.c # only in libvpx 1.13.1
        #vp8/encoder/arm/neon/shortfdct_neon.c # only in libvpx 1.13.1
        #vp8/encoder/arm/neon/vp8_shortwalsh4x4_neon.c # only in libvpx 1.13.1

        #highbd files are for if CONFIG_VP9_HIGHBITDEPTH=1
        #vp9/common/arm/neon/vp9_highbd_iht16x16_add_neon.c
        #vp9/common/arm/neon/vp9_highbd_iht4x4_add_neon.c
        #vp9/common/arm/neon/vp9_highbd_iht8x8_add_neon.c

        vp9/common/arm/neon/vp9_iht16x16_add_neon.c
        vp9/common/arm/neon/vp9_iht4x4_add_neon.c
        vp9/common/arm/neon/vp9_iht8x8_add_neon.c
        vp9/common/arm/neon/vp9_iht_neon.h

        #vp9/encoder/arm/neon/vp9_dct_neon.c
        #denoiser not used
        #vp9/encoder/arm/neon/vp9_denoiser_neon.c
        #vp9/encoder/arm/neon/vp9_diamond_search_sad_neon.c
        #vp9/encoder/arm/neon/vp9_error_neon.c

        # sve not supported
        #vp9/encoder/arm/neon/vp9_error_sve.c

        #vp9/encoder/arm/neon/vp9_frame_scale_neon.c

        #highbd files are for if CONFIG_VP9_HIGHBITDEPTH=1
        #vp9/encoder/arm/neon/vp9_highbd_error_neon.c
        #vp9/encoder/arm/neon/vp9_highbd_temporal_filter_neon.c

        vp9/encoder/arm/neon/vp9_quantize_neon.c
        # not in v1.13.1 vp9/encoder/arm/neon/vp9_temporal_filter_neon.c

        vpx_dsp/arm/avg_neon.c
        vpx_dsp/arm/avg_pred_neon.c
        vpx_dsp/arm/deblock_neon.c
        vpx_dsp/arm/fdct16x16_neon.c
        vpx_dsp/arm/fdct16x16_neon.h
        vpx_dsp/arm/fdct32x32_neon.c
        vpx_dsp/arm/fdct32x32_neon.h
        vpx_dsp/arm/fdct4x4_neon.c
        vpx_dsp/arm/fdct4x4_neon.h
        vpx_dsp/arm/fdct8x8_neon.c
        vpx_dsp/arm/fdct8x8_neon.h
        vpx_dsp/arm/fdct_neon.h
        vpx_dsp/arm/fdct_partial_neon.c
        vpx_dsp/arm/hadamard_neon.c

        #highbd files are for if CONFIG_VP9_HIGHBITDEPTH=1
        # not in v1.13.1 #vpx_dsp/arm/highbd_avg_neon.c
        # not in v1.13.1 #vpx_dsp/arm/highbd_avg_pred_neon.c
        # not in v1.13.1 #vpx_dsp/armhighbd_convolve8_neon.h
        # not in v1.13.1 #vpx_dsp/armhighbd_convolve8_sve.h
        # not in v1.13.1 #vpx_dsp/armhighbd_hadamard_neon.c
        #vpx_dsp/arm/highbd_idct16x16_add_neon# not in v1.13.1 .c
        #vpx_dsp/arm/highbd_idct32x32_1024_add_neon.c
        #vpx_dsp/arm/highbd_idct32x32_135_add_neon.c
        #vpx_dsp/arm/highbd_idct32x32_34_add_neon.c
        #vpx_dsp/arm/highbd_idct32x32_add_neon.c
        #vpx_dsp/arm/highbd_idct4x4_add_neon.c
        #vpx_dsp/arm/highbd_idct8x8_add_neon.c
        #vpx_dsp/arm/highbd_idct_neon.h
        #vpx_dsp/arm/highbd_intrapred_neon.c
        #vpx_dsp/arm/highbd_loopfilter_neon.c
        #vpx_dsp/arm/highbd_quantize_neon.c
        # not in v1.13.1 #vpx_dsp/arm/highbd_sad4d_neon.c
        #vpx_dsp/arm/highbd_sad_neon.c
        # not in v1.13.1 #vpx_dsp/arm/highbd_sse_neon.c
        # not in v1.13.1 #vpx_dsp/arm/highbd_subpel_variance_neon.c
        #vpx_dsp/arm/highbd_variance_neon.c
        # not in v1.13.1 #vpx_dsp/arm/highbd_variance_neon_dotprod.c
        # not in v1.13.1 #vpx_dsp/arm/highbd_variance_sve.c
        #vpx_dsp/arm/highbd_vpx_convolve8_neon.c
        # not in v1.13.1 #vpx_dsp/arm/highbd_vpx_convolve8_sve.c
        # not in v1.13.1 #vpx_dsp/arm/highbd_vpx_convolve8_sve2.c
        #vpx_dsp/arm/highbd_vpx_convolve_avg_neon.c
        #vpx_dsp/arm/highbd_vpx_convolve_copy_neon.c


        vpx_dsp/arm/idct16x16_1_add_neon.c
        vpx_dsp/arm/idct16x16_add_neon.c
        vpx_dsp/arm/idct32x32_135_add_neon.c
        vpx_dsp/arm/idct32x32_1_add_neon.c
        vpx_dsp/arm/idct32x32_34_add_neon.c
        vpx_dsp/arm/idct32x32_add_neon.c
        #vpx_dsp/arm/idct4x4_1_add_neon.asm
        vpx_dsp/arm/idct4x4_1_add_neon.c
        #vpx_dsp/arm/idct4x4_add_neon.asm
        vpx_dsp/arm/idct4x4_add_neon.c
        vpx_dsp/arm/idct8x8_1_add_neon.c
        vpx_dsp/arm/idct8x8_add_neon.c
        #vpx_dsp/arm/idct_neon.asm
        vpx_dsp/arm/idct_neon.h
        vpx_dsp/arm/intrapred_neon.c
        #vpx_dsp/arm/intrapred_neon_asm.asm
        #vpx_dsp/arm/loopfilter_16_neon.asm
        #vpx_dsp/arm/loopfilter_4_neon.asm
        #vpx_dsp/arm/loopfilter_8_neon.asm
        vpx_dsp/arm/loopfilter_neon.c
        vpx_dsp/arm/mem_neon.h
        vpx_dsp/arm/quantize_neon.c
        vpx_dsp/arm/sad4d_neon.c
        # not in v1.13.1 vpx_dsp/arm/sad4d_neon_dotprod.c
        vpx_dsp/arm/sad_neon.c
        # not in v1.13.1 vpx_dsp/arm/sad_neon_dotprod.c
        #vpx_dsp/arm/save_reg_neon.asm
        # not in v1.13.1 vpx_dsp/arm/sse_neon.c
        # not in v1.13.1 vpx_dsp/arm/sse_neon_dotprod.c
        vpx_dsp/arm/subpel_variance_neon.c
        vpx_dsp/arm/subtract_neon.c
        vpx_dsp/arm/sum_neon.h
        vpx_dsp/arm/sum_squares_neon.c

        # sve not supported
        # not in v1.13.1 #vpx_dsp/arm/sum_squares_sve.c

        vpx_dsp/arm/transpose_neon.h
        vpx_dsp/arm/variance_neon.c
        # not in v1.13.1 vpx_dsp/arm/variance_neon_dotprod.c
        #vpx_dsp/arm/vpx_convolve8_avg_horiz_filter_type1_neon.asm
        #vpx_dsp/arm/vpx_convolve8_avg_horiz_filter_type2_neon.asm
        #vpx_dsp/arm/vpx_convolve8_avg_vert_filter_type1_neon.asm
        #vpx_dsp/arm/vpx_convolve8_avg_vert_filter_type2_neon.asm
        #vpx_dsp/arm/vpx_convolve8_horiz_filter_type1_neon.asm
        #vpx_dsp/arm/vpx_convolve8_horiz_filter_type2_neon.asm
        vpx_dsp/arm/vpx_convolve8_neon.c
        vpx_dsp/arm/vpx_convolve8_neon.h
        #vpx_dsp/arm/vpx_convolve8_neon_asm.c
        #vpx_dsp/arm/vpx_convolve8_neon_asm.h
        # not in v1.13.1 vpx_dsp/arm/vpx_convolve8_neon_dotprod.c
        # not in v1.13.1 vpx_dsp/arm/vpx_convolve8_neon_i8mm.c
        #vpx_dsp/arm/vpx_convolve8_vert_filter_type1_neon.asm
        #vpx_dsp/arm/vpx_convolve8_vert_filter_type2_neon.asm
        vpx_dsp/arm/vpx_convolve_avg_neon.c
        #vpx_dsp/arm/vpx_convolve_avg_neon_asm.asm
        vpx_dsp/arm/vpx_convolve_copy_neon.c
        #vpx_dsp/arm/vpx_convolve_copy_neon_asm.asm
        vpx_dsp/arm/vpx_convolve_neon.c
        # not in v1.13.1 #vpx_dsp/arm/vpx_neon_sve2_bridge.h
        # not in v1.13.1 #vpx_dsp/arm/vpx_neon_sve_bridge.h
        vpx_dsp/arm/vpx_scaled_convolve8_neon.c
    )

    if(DEFINED TARGET_OS_LINUX)
        list(APPEND vpx_SOURCES
            vpx_config_linux_arm64.c
            # not in v1.13.1 vpx_ports/aarch64_cpudetect.c
            )
    endif()
    
    if(DEFINED TARGET_OS_ANDROID)
        list(APPEND vpx_SOURCES 
            ../androidNdk/android/cpufeatures/cpu-features_ndk.c
            )
        if(DEFINED TARGET_CPU_ARM64)
            list(APPEND vpx_SOURCES
                vpx_config_android_arm64.c
                vpx_config_android_arm64.h
                # not in v1.13.1 #vpx_ports/aarch64_cpudetect.c
                )
        else()
            list(APPEND vpx_SOURCES
                vpx_config_android_arm32.c
                vpx_config_android_arm32.h
                # not in v1.13.1 #vpx_ports/aarch32_cpudetect.c
                )
        endif()

    endif()
endif()

if(DEFINED TARGET_CPU_ARM64)
    if(DEFINED TARGET_OS_LINUX)
        list(APPEND vpx_SOURCES
            vpx_config_linux_arm64.h
        )
    endif()

    if(DEFINED TARGET_OS_ANDROID)
        list(APPEND vpx_SOURCES
            vpx_config_android_arm64.h
        )
    endif()
endif()

foreach(vpx_src IN LISTS vpx_SOURCES)
    if(vpx_src MATCHES "\\.asm$")
        set_source_files_properties(${vpx_src} PROPERTIES LANGUAGE ASM_NASM)
    endif()
endforeach()

add_library(vpx STATIC ${vpx_SOURCES})

if(DEFINED TARGET_OS_WINDOWS)
    target_compile_definitions(vpx
        PRIVATE
            WIN32
            _CRT_SECURE_NO_WARNINGS
            _CRT_SECURE_NO_DEPRECATE
    )

else()
    nlc_target_add_supported_warning_options(vpx C
        -Wno-missing-prototypes
        -Wno-strict-prototypes
        -Wno-undef
        -Wno-shadow
        -Wno-unused-but-set-variable
    )
endif()

if(NOT DEFINED TARGET_OS_WINDOWS)
    if(DEFINED TARGET_CPU_X64)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mavx512f -mavx512cd -mavx512bw -mavx512dq -mavx512vl -mmmx -msse2 -msse3 -msse4.1 -mavx -mavx2")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx512f -mavx512cd -mavx512bw -mavx512dq -mavx512vl -mmmx -msse2 -msse3 -msse4.1 -mavx -mavx2")
    endif()

endif()

if(DEFINED TARGET_CPU_ARM64)
    if(DEFINED TARGET_OS_ANDROID)
    # android flags
    set(CMAKE_C_FLAGS "  -march=armv8.8-a -DNDEBUG -O3 -fPIC -Wdisabled-optimization -Wfloat-conversion -Wformat=2 -Wpointer-arith -Wtype-limits -Wcast-qual -Wvla -Wimplicit-function-declaration -Wmissing-declarations -Wmissing-prototypes -Wshadow -Wstrict-prototypes -Wuninitialized -Wunused -Wextra -Wundef -Wframe-larger-than=52000 -Winline" )
    #set(CMAKE_CXX_FLAGS "  -march=armv8-a -DNDEBUG -O3 -fPIC -Wall -Wdisabled-optimization -Wextra-semi -Wfloat-conversion -Wformat=2 -Wpointer-arith -Wtype-limits -Wcast-qual -Wvla -Wmissing-declarations -Wshadow -Wuninitialized -Wunused -Wextra -Wno-psabi -Wc++14-extensions -Wc++17-extensions -Wc++20-extensions -Wnon-virtual-dtor -std=gnu++11")
    endif()
    if(DEFINED TARGET_OS_LINUX)
        # error: inlining failed in call to ‘always_inline’ ‘vdotq_u32’: target specific option mismatch
        # to fix the intrisics error for gcc the -march needs to be -march=armv8.8-a instead of -march=armv8-a
        set(CMAKE_C_FLAGS " -march=armv8.8-a -DNDEBUG -O3 -fPIC -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0 -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -Wall -Wdisabled-optimization -Wfloat-conversion -Wformat=2 -Wpointer-arith -Wtype-limits -Wcast-qual -Wvla -Wimplicit-function-declaration -Wmissing-declarations -Wmissing-prototypes -Wshadow -Wstrict-prototypes -Wuninitialized -Wunused -Wextra -Wundef -Wframe-larger-than=52000 -std=gnu99")
        #set(CMAKE_CXX_FLAGS " -march=armv8-a -DNDEBUG -O3 -fPIC -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0 -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -Wall -Wdisabled-optimization -Wextra-semi -Wfloat-conversion -Wformat=2 -Wpointer-arith -Wtype-limits -Wcast-qual -Wvla -Wmissing-declarations -Wshadow -Wuninitialized -Wunused -Wextra -Wno-psabi -Wc++14-extensions -Wc++17-extensions -Wc++20-extensions -Wnon-virtual-dtor -std=gnu++11 -std=gnu++11")
    endif()
endif()


if(DEFINED TARGET_CPU_ARM64)
    if(DEFINED TARGET_OS_ANDROID)
        # android flags
        #set(CMAKE_C_FLAGS "  -march=armv8-a -DNDEBUG -O3 -fPIC -Wdisabled-optimization -Wfloat-conversion -Wformat=2 -Wpointer-arith -Wtype-limits -Wcast-qual -Wvla -Wimplicit-function-declaration -Wmissing-declarations -Wmissing-prototypes -Wshadow -Wstrict-prototypes -Wuninitialized -Wunused -Wextra -Wundef -Wframe-larger-than=52000 -Winline" )
        #set(CMAKE_CXX_FLAGS "  -march=armv8-a -DNDEBUG -O3 -fPIC -Wall -Wdisabled-optimization -Wextra-semi -Wfloat-conversion -Wformat=2 -Wpointer-arith -Wtype-limits -Wcast-qual -Wvla -Wmissing-declarations -Wshadow -Wuninitialized -Wunused -Wextra -Wno-psabi -Wc++14-extensions -Wc++17-extensions -Wc++20-extensions -Wnon-virtual-dtor -std=gnu++11")
    endif()
endif()

target_include_directories(vpx
  PRIVATE
    vpx
    vpx_dsp
)

if(DEFINED TARGET_OS_ANDROID)
    target_include_directories(vpx
      PRIVATE
        ../androidNdk/android/cpufeatures
    )
endif()
