gcc-7.1 \
		-m32 \
		-Wno-narrowing \
		-msse4.1 \
-D HAVE_CONFIG_H \
-Ihlsdk/common -Ihlsdk/dlls -Ihlsdk/engine -Ihlsdk/pm_shared -Imetamod -Ispeex -Isilk -IHashers -IMultiThreading -IUtility \
-Iopus -IVoiceCodecs/Opus -IVoiceCodecs/Opus/celt -IVoiceCodecs/Opus/celt/x86 -IVoiceCodecs/Opus/silk -IVoiceCodecs/Opus/silk/float -IVoiceCodecs/Opus/silk/x86 -IVoiceCodecs/Opus/src \
	-c VoiceCodecs/Opus/silk/x86/NSQ_del_dec_sse.c \
-o VoiceTranscoderSSE41_1.o

gcc-7.1 \
		-m32 \
		-Wno-narrowing \
		-msse4.1 \
-D HAVE_CONFIG_H \
-Ihlsdk/common -Ihlsdk/dlls -Ihlsdk/engine -Ihlsdk/pm_shared -Imetamod -Ispeex -Isilk -IHashers -IMultiThreading -IUtility \
-Iopus -IVoiceCodecs/Opus -IVoiceCodecs/Opus/celt -IVoiceCodecs/Opus/celt/x86 -IVoiceCodecs/Opus/silk -IVoiceCodecs/Opus/silk/float -IVoiceCodecs/Opus/silk/x86 -IVoiceCodecs/Opus/src \
	-c VoiceCodecs/Opus/silk/x86/NSQ_sse.c \
-o VoiceTranscoderSSE41_2.o

gcc-7.1 \
		-m32 \
		-Wno-narrowing \
		-msse4.1 \
-D HAVE_CONFIG_H \
-Ihlsdk/common -Ihlsdk/dlls -Ihlsdk/engine -Ihlsdk/pm_shared -Imetamod -Ispeex -Isilk -IHashers -IMultiThreading -IUtility \
-Iopus -IVoiceCodecs/Opus -IVoiceCodecs/Opus/celt -IVoiceCodecs/Opus/celt/x86 -IVoiceCodecs/Opus/silk -IVoiceCodecs/Opus/silk/float -IVoiceCodecs/Opus/silk/x86 -IVoiceCodecs/Opus/src \
	-c VoiceCodecs/Opus/silk/x86/VQ_WMat_EC_sse.c \
-o VoiceTranscoderSSE41_3.o

g++-7.1 \
		-Os -fdata-sections -ffunction-sections -shared -m32 \
		-std=c++14 \
		-flto -s -Wno-narrowing \
		-msse2 \
		-fvisibility=hidden \
		-static-libgcc -static-libstdc++ \
-D HAVE_CONFIG_H \
-Ihlsdk/common -Ihlsdk/dlls -Ihlsdk/engine -Ihlsdk/pm_shared -Imetamod -Ispeex -Isilk -IHashers -IMultiThreading -IUtility \
-Iopus -IVoiceCodecs/Opus -IVoiceCodecs/Opus/celt -IVoiceCodecs/Opus/celt/x86 -IVoiceCodecs/Opus/silk -IVoiceCodecs/Opus/silk/float -IVoiceCodecs/Opus/silk/x86 -IVoiceCodecs/Opus/src \
	-x c++ Main.cpp \
	-x c++ API.cpp \
	-x c++ ThreadMode.cpp \
	-x c++ VoiceCodec_Speex.cpp \
	-x c++ VoiceCodec_SILK.cpp \
	-x c++ Hashers/CRC32.cpp \
	-x c++ MultiThreading/Mutex.cpp \
	-x c++ MultiThreading/Signal.cpp \
	-x c++ MultiThreading/Thread.cpp \
	-x c++ Utility/BinaryPattern.cpp \
	-x c++ Utility/Buffer.cpp \
	-x c++ Utility/EngineUTIL.cpp \
	-x c++ Utility/FunctionHook_Beginning.cpp \
	-x c++ Utility/JmpOpcode.cpp \
	-x c++ Utility/MemoryUnlocker.cpp \
	-x c++ Utility/MetaUTIL.cpp \
	-x c++ Utility/Module.cpp \
	-x c++ Utility/Section.cpp \
	-x c++ Utility/SteamID.cpp \
	-x c++ Utility/UtilFunctions.cpp \
	-x c VoiceCodecs/Speex/speex_callbacks.c \
	-x c VoiceCodecs/Speex/speex_header.c \
	-x c VoiceCodecs/Speex/stereo.c \
	-x c VoiceCodecs/Speex/vbr.c \
	-x c VoiceCodecs/Speex/vq_.c \
	-x c VoiceCodecs/Speex/bits.c \
	-x c VoiceCodecs/Speex/cb_search.c \
	-x c VoiceCodecs/Speex/exc_5_64_table.c \
	-x c VoiceCodecs/Speex/exc_5_256_table.c \
	-x c VoiceCodecs/Speex/exc_8_128_table.c \
	-x c VoiceCodecs/Speex/exc_10_16_table.c \
	-x c VoiceCodecs/Speex/exc_10_32_table.c \
	-x c VoiceCodecs/Speex/exc_20_32_table.c \
	-x c VoiceCodecs/Speex/filters.c \
	-x c VoiceCodecs/Speex/gain_table.c \
	-x c VoiceCodecs/Speex/gain_table_lbr.c \
	-x c VoiceCodecs/Speex/hexc_10_32_table.c \
	-x c VoiceCodecs/Speex/hexc_table.c \
	-x c VoiceCodecs/Speex/high_lsp_tables.c \
	-x c VoiceCodecs/Speex/lpc.c \
	-x c VoiceCodecs/Speex/lsp.c \
	-x c VoiceCodecs/Speex/lsp_tables_nb.c \
	-x c VoiceCodecs/Speex/ltp.c \
	-x c VoiceCodecs/Speex/math_approx.c \
	-x c VoiceCodecs/Speex/misc.c \
	-x c VoiceCodecs/Speex/modes_.c \
	-x c VoiceCodecs/Speex/nb_celp.c \
	-x c VoiceCodecs/Speex/quant_lsp.c \
	-x c VoiceCodecs/Speex/sb_celp.c \
	-x c VoiceCodecs/SILK/SKP_Silk_tables_type_offset.c \
	-x c VoiceCodecs/SILK/SKP_Silk_VAD.c \
	-x c VoiceCodecs/SILK/SKP_Silk_VQ_nearest_neighbor_FLP.c \
	-x c VoiceCodecs/SILK/SKP_Silk_warped_autocorrelation_FLP.c \
	-x c VoiceCodecs/SILK/SKP_Silk_wrappers_FLP.c \
	-x c VoiceCodecs/SILK/SKP_Silk_A2NLSF.c \
	-x c VoiceCodecs/SILK/SKP_Silk_allpass_int.c \
	-x c VoiceCodecs/SILK/SKP_Silk_allpass_int_FLP.c \
	-x c VoiceCodecs/SILK/SKP_Silk_ana_filt_bank_1.c \
	-x c VoiceCodecs/SILK/SKP_Silk_apply_sine_window_FLP.c \
	-x c VoiceCodecs/SILK/SKP_Silk_autocorrelation_FLP.c \
	-x c VoiceCodecs/SILK/SKP_Silk_biquad.c \
	-x c VoiceCodecs/SILK/SKP_Silk_biquad_alt.c \
	-x c VoiceCodecs/SILK/SKP_Silk_burg_modified_FLP.c \
	-x c VoiceCodecs/SILK/SKP_Silk_bwexpander.c \
	-x c VoiceCodecs/SILK/SKP_Silk_bwexpander_32.c \
	-x c VoiceCodecs/SILK/SKP_Silk_bwexpander_FLP.c \
	-x c VoiceCodecs/SILK/SKP_Silk_CNG.c \
	-x c VoiceCodecs/SILK/SKP_Silk_code_signs.c \
	-x c VoiceCodecs/SILK/SKP_Silk_control_audio_bandwidth.c \
	-x c VoiceCodecs/SILK/SKP_Silk_control_codec_FLP.c \
	-x c VoiceCodecs/SILK/SKP_Silk_corrMatrix_FLP.c \
	-x c VoiceCodecs/SILK/SKP_Silk_create_init_destroy.c \
	-x c VoiceCodecs/SILK/SKP_Silk_dec_API.c \
	-x c VoiceCodecs/SILK/SKP_Silk_decimate2_coarse_FLP.c \
	-x c VoiceCodecs/SILK/SKP_Silk_decimate2_coarsest_FLP.c \
	-x c VoiceCodecs/SILK/SKP_Silk_decode_core.c \
	-x c VoiceCodecs/SILK/SKP_Silk_decode_frame.c \
	-x c VoiceCodecs/SILK/SKP_Silk_decode_parameters.c \
	-x c VoiceCodecs/SILK/SKP_Silk_decode_pitch.c \
	-x c VoiceCodecs/SILK/SKP_Silk_decode_pulses.c \
	-x c VoiceCodecs/SILK/SKP_Silk_decoder_set_fs.c \
	-x c VoiceCodecs/SILK/SKP_Silk_detect_SWB_input.c \
	-x c VoiceCodecs/SILK/SKP_Silk_enc_API.c \
	-x c VoiceCodecs/SILK/SKP_Silk_encode_frame_FLP.c \
	-x c VoiceCodecs/SILK/SKP_Silk_encode_parameters.c \
	-x c VoiceCodecs/SILK/SKP_Silk_encode_pulses.c \
	-x c VoiceCodecs/SILK/SKP_Silk_energy_FLP.c \
	-x c VoiceCodecs/SILK/SKP_Silk_find_LPC_FLP.c \
	-x c VoiceCodecs/SILK/SKP_Silk_find_LTP_FLP.c \
	-x c VoiceCodecs/SILK/SKP_Silk_find_pitch_lags_FLP.c \
	-x c VoiceCodecs/SILK/SKP_Silk_find_pred_coefs_FLP.c \
	-x c VoiceCodecs/SILK/SKP_Silk_gain_quant.c \
	-x c VoiceCodecs/SILK/SKP_Silk_HP_variable_cutoff_FLP.c \
	-x c VoiceCodecs/SILK/SKP_Silk_init_encoder_FLP.c \
	-x c VoiceCodecs/SILK/SKP_Silk_inner_product_FLP.c \
	-x c VoiceCodecs/SILK/SKP_Silk_interpolate.c \
	-x c VoiceCodecs/SILK/SKP_Silk_k2a_FLP.c \
	-x c VoiceCodecs/SILK/SKP_Silk_LBRR_reset.c \
	-x c VoiceCodecs/SILK/SKP_Silk_levinsondurbin_FLP.c \
	-x c VoiceCodecs/SILK/SKP_Silk_lin2log.c \
	-x c VoiceCodecs/SILK/SKP_Silk_log2lin.c \
	-x c VoiceCodecs/SILK/SKP_Silk_lowpass_int.c \
	-x c VoiceCodecs/SILK/SKP_Silk_lowpass_short.c \
	-x c VoiceCodecs/SILK/SKP_Silk_LP_variable_cutoff.c \
	-x c VoiceCodecs/SILK/SKP_Silk_LPC_analysis_filter_FLP.c \
	-x c VoiceCodecs/SILK/SKP_Silk_LPC_inv_pred_gain.c \
	-x c VoiceCodecs/SILK/SKP_Silk_LPC_inv_pred_gain_FLP.c \
	-x c VoiceCodecs/SILK/SKP_Silk_LPC_synthesis_filter.c \
	-x c VoiceCodecs/SILK/SKP_Silk_LPC_synthesis_order16.c \
	-x c VoiceCodecs/SILK/SKP_Silk_LSF_cos_table.c \
	-x c VoiceCodecs/SILK/SKP_Silk_LTP_analysis_filter_FLP.c \
	-x c VoiceCodecs/SILK/SKP_Silk_LTP_scale_ctrl_FLP.c \
	-x c VoiceCodecs/SILK/SKP_Silk_MA.c \
	-x c VoiceCodecs/SILK/SKP_Silk_NLSF_MSVQ_decode.c \
	-x c VoiceCodecs/SILK/SKP_Silk_NLSF_MSVQ_decode_FLP.c \
	-x c VoiceCodecs/SILK/SKP_Silk_NLSF_MSVQ_encode_FLP.c \
	-x c VoiceCodecs/SILK/SKP_Silk_NLSF_stabilize.c \
	-x c VoiceCodecs/SILK/SKP_Silk_NLSF_VQ_rate_distortion_FLP.c \
	-x c VoiceCodecs/SILK/SKP_Silk_NLSF_VQ_sum_error_FLP.c \
	-x c VoiceCodecs/SILK/SKP_Silk_NLSF_VQ_weights_laroia_FLP.c \
	-x c VoiceCodecs/SILK/SKP_Silk_NLSF2A.c \
	-x c VoiceCodecs/SILK/SKP_Silk_NLSF2A_stable.c \
	-x c VoiceCodecs/SILK/SKP_Silk_noise_shape_analysis_FLP.c \
	-x c VoiceCodecs/SILK/SKP_Silk_NSQ.c \
	-x c VoiceCodecs/SILK/SKP_Silk_NSQ_del_dec.c \
	-x c VoiceCodecs/SILK/SKP_Silk_pitch_analysis_core_FLP.c \
	-x c VoiceCodecs/SILK/SKP_Silk_pitch_est_tables.c \
	-x c VoiceCodecs/SILK/SKP_Silk_PLC.c \
	-x c VoiceCodecs/SILK/SKP_Silk_prefilter_FLP.c \
	-x c VoiceCodecs/SILK/SKP_Silk_process_gains_FLP.c \
	-x c VoiceCodecs/SILK/SKP_Silk_process_NLSFs_FLP.c \
	-x c VoiceCodecs/SILK/SKP_Silk_quant_LTP_gains_FLP.c \
	-x c VoiceCodecs/SILK/SKP_Silk_range_coder.c \
	-x c VoiceCodecs/SILK/SKP_Silk_regularize_correlations_FLP.c \
	-x c VoiceCodecs/SILK/SKP_Silk_resampler.c \
	-x c VoiceCodecs/SILK/SKP_Silk_resampler_down2.c \
	-x c VoiceCodecs/SILK/SKP_Silk_resampler_down2_3.c \
	-x c VoiceCodecs/SILK/SKP_Silk_resampler_down3.c \
	-x c VoiceCodecs/SILK/SKP_Silk_resampler_private_AR2.c \
	-x c VoiceCodecs/SILK/SKP_Silk_resampler_private_ARMA4.c \
	-x c VoiceCodecs/SILK/SKP_Silk_resampler_private_copy.c \
	-x c VoiceCodecs/SILK/SKP_Silk_resampler_private_down_FIR.c \
	-x c VoiceCodecs/SILK/SKP_Silk_resampler_private_down4.c \
	-x c VoiceCodecs/SILK/SKP_Silk_resampler_private_IIR_FIR.c \
	-x c VoiceCodecs/SILK/SKP_Silk_resampler_private_up2_HQ.c \
	-x c VoiceCodecs/SILK/SKP_Silk_resampler_private_up4.c \
	-x c VoiceCodecs/SILK/SKP_Silk_resampler_rom.c \
	-x c VoiceCodecs/SILK/SKP_Silk_resampler_up2.c \
	-x c VoiceCodecs/SILK/SKP_Silk_residual_energy_FLP.c \
	-x c VoiceCodecs/SILK/SKP_Silk_scale_copy_vector_FLP.c \
	-x c VoiceCodecs/SILK/SKP_Silk_scale_vector_FLP.c \
	-x c VoiceCodecs/SILK/SKP_Silk_schur_FLP.c \
	-x c VoiceCodecs/SILK/SKP_Silk_shell_coder.c \
	-x c VoiceCodecs/SILK/SKP_Silk_sigm_Q15.c \
	-x c VoiceCodecs/SILK/SKP_Silk_solve_LS_FLP.c \
	-x c VoiceCodecs/SILK/SKP_Silk_sort.c \
	-x c VoiceCodecs/SILK/SKP_Silk_sort_FLP.c \
	-x c VoiceCodecs/SILK/SKP_Silk_sum_sqr_shift.c \
	-x c VoiceCodecs/SILK/SKP_Silk_tables_gain.c \
	-x c VoiceCodecs/SILK/SKP_Silk_tables_LTP.c \
	-x c VoiceCodecs/SILK/SKP_Silk_tables_NLSF_CB0_10.c \
	-x c VoiceCodecs/SILK/SKP_Silk_tables_NLSF_CB0_10_FLP.c \
	-x c VoiceCodecs/SILK/SKP_Silk_tables_NLSF_CB0_16.c \
	-x c VoiceCodecs/SILK/SKP_Silk_tables_NLSF_CB0_16_FLP.c \
	-x c VoiceCodecs/SILK/SKP_Silk_tables_NLSF_CB1_10.c \
	-x c VoiceCodecs/SILK/SKP_Silk_tables_NLSF_CB1_10_FLP.c \
	-x c VoiceCodecs/SILK/SKP_Silk_tables_NLSF_CB1_16.c \
	-x c VoiceCodecs/SILK/SKP_Silk_tables_NLSF_CB1_16_FLP.c \
	-x c VoiceCodecs/SILK/SKP_Silk_tables_other.c \
	-x c VoiceCodecs/SILK/SKP_Silk_tables_other_FLP.c \
	-x c VoiceCodecs/SILK/SKP_Silk_tables_pitch_lag.c \
	-x c VoiceCodecs/SILK/SKP_Silk_tables_pulses_per_block.c \
	-x c VoiceCodecs/SILK/SKP_Silk_tables_sign.c \
	-x c VoiceCodecs/Opus/celt/bands.c \
	-x c VoiceCodecs/Opus/celt/celt.c \
	-x c VoiceCodecs/Opus/celt/celt_decoder.c \
	-x c VoiceCodecs/Opus/celt/celt_encoder.c \
	-x c VoiceCodecs/Opus/celt/celt_lpc.c \
	-x c VoiceCodecs/Opus/celt/cwrs.c \
	-x c VoiceCodecs/Opus/celt/entcode.c \
	-x c VoiceCodecs/Opus/celt/entdec.c \
	-x c VoiceCodecs/Opus/celt/entenc.c \
	-x c VoiceCodecs/Opus/celt/kiss_fft.c \
	-x c VoiceCodecs/Opus/celt/laplace.c \
	-x c VoiceCodecs/Opus/celt/mathops.c \
	-x c VoiceCodecs/Opus/celt/mdct.c \
	-x c VoiceCodecs/Opus/celt/modes.c \
	-x c VoiceCodecs/Opus/celt/pitch.c \
	-x c VoiceCodecs/Opus/celt/quant_bands.c \
	-x c VoiceCodecs/Opus/celt/rate.c \
	-x c VoiceCodecs/Opus/celt/vq.c \
	-x c VoiceCodecs/Opus/celt/x86/celt_lpc_sse.c \
	-x c VoiceCodecs/Opus/celt/x86/pitch_sse.c \
	-x c VoiceCodecs/Opus/celt/x86/pitch_sse2.c \
	-x c VoiceCodecs/Opus/celt/x86/pitch_sse4_1.c \
	-x c VoiceCodecs/Opus/celt/x86/vq_sse2.c \
	-x c VoiceCodecs/Opus/celt/x86/x86cpu.c \
	-x c VoiceCodecs/Opus/celt/x86/x86_celt_map.c \
	-x c VoiceCodecs/Opus/silk/A2NLSF.c \
	-x c VoiceCodecs/Opus/silk/ana_filt_bank_1.c \
	-x c VoiceCodecs/Opus/silk/biquad_alt.c \
	-x c VoiceCodecs/Opus/silk/bwexpander.c \
	-x c VoiceCodecs/Opus/silk/bwexpander_32.c \
	-x c VoiceCodecs/Opus/silk/check_control_input.c \
	-x c VoiceCodecs/Opus/silk/CNG.c \
	-x c VoiceCodecs/Opus/silk/code_signs.c \
	-x c VoiceCodecs/Opus/silk/control_audio_bandwidth.c \
	-x c VoiceCodecs/Opus/silk/control_codec.c \
	-x c VoiceCodecs/Opus/silk/control_SNR.c \
	-x c VoiceCodecs/Opus/silk/debug.c \
	-x c VoiceCodecs/Opus/silk/decoder_set_fs.c \
	-x c VoiceCodecs/Opus/silk/decode_core.c \
	-x c VoiceCodecs/Opus/silk/decode_frame.c \
	-x c VoiceCodecs/Opus/silk/decode_indices.c \
	-x c VoiceCodecs/Opus/silk/decode_parameters.c \
	-x c VoiceCodecs/Opus/silk/decode_pitch.c \
	-x c VoiceCodecs/Opus/silk/decode_pulses.c \
	-x c VoiceCodecs/Opus/silk/dec_API.c \
	-x c VoiceCodecs/Opus/silk/encode_indices.c \
	-x c VoiceCodecs/Opus/silk/encode_pulses.c \
	-x c VoiceCodecs/Opus/silk/enc_API.c \
	-x c VoiceCodecs/Opus/silk/float/apply_sine_window_FLP.c \
	-x c VoiceCodecs/Opus/silk/float/autocorrelation_FLP.c \
	-x c VoiceCodecs/Opus/silk/float/burg_modified_FLP.c \
	-x c VoiceCodecs/Opus/silk/float/bwexpander_FLP.c \
	-x c VoiceCodecs/Opus/silk/float/corrMatrix_FLP.c \
	-x c VoiceCodecs/Opus/silk/float/encode_frame_FLP.c \
	-x c VoiceCodecs/Opus/silk/float/energy_FLP.c \
	-x c VoiceCodecs/Opus/silk/float/find_LPC_FLP.c \
	-x c VoiceCodecs/Opus/silk/float/find_LTP_FLP.c \
	-x c VoiceCodecs/Opus/silk/float/find_pitch_lags_FLP.c \
	-x c VoiceCodecs/Opus/silk/float/find_pred_coefs_FLP.c \
	-x c VoiceCodecs/Opus/silk/float/inner_product_FLP.c \
	-x c VoiceCodecs/Opus/silk/float/k2a_FLP.c \
	-x c VoiceCodecs/Opus/silk/float/LPC_analysis_filter_FLP.c \
	-x c VoiceCodecs/Opus/silk/float/LPC_inv_pred_gain_FLP.c \
	-x c VoiceCodecs/Opus/silk/float/LTP_analysis_filter_FLP.c \
	-x c VoiceCodecs/Opus/silk/float/LTP_scale_ctrl_FLP.c \
	-x c VoiceCodecs/Opus/silk/float/noise_shape_analysis_FLP.c \
	-x c VoiceCodecs/Opus/silk/float/pitch_analysis_core_FLP.c \
	-x c VoiceCodecs/Opus/silk/float/process_gains_FLP.c \
	-x c VoiceCodecs/Opus/silk/float/regularize_correlations_FLP.c \
	-x c VoiceCodecs/Opus/silk/float/residual_energy_FLP.c \
	-x c VoiceCodecs/Opus/silk/float/scale_copy_vector_FLP.c \
	-x c VoiceCodecs/Opus/silk/float/scale_vector_FLP.c \
	-x c VoiceCodecs/Opus/silk/float/schur_FLP.c \
	-x c VoiceCodecs/Opus/silk/float/sort_FLP.c \
	-x c VoiceCodecs/Opus/silk/float/warped_autocorrelation_FLP.c \
	-x c VoiceCodecs/Opus/silk/float/wrappers_FLP.c \
	-x c VoiceCodecs/Opus/silk/gain_quant.c \
	-x c VoiceCodecs/Opus/silk/HP_variable_cutoff.c \
	-x c VoiceCodecs/Opus/silk/init_decoder.c \
	-x c VoiceCodecs/Opus/silk/init_encoder.c \
	-x c VoiceCodecs/Opus/silk/inner_prod_aligned.c \
	-x c VoiceCodecs/Opus/silk/interpolate.c \
	-x c VoiceCodecs/Opus/silk/lin2log.c \
	-x c VoiceCodecs/Opus/silk/log2lin.c \
	-x c VoiceCodecs/Opus/silk/LPC_analysis_filter.c \
	-x c VoiceCodecs/Opus/silk/LPC_fit.c \
	-x c VoiceCodecs/Opus/silk/LPC_inv_pred_gain.c \
	-x c VoiceCodecs/Opus/silk/LP_variable_cutoff.c \
	-x c VoiceCodecs/Opus/silk/NLSF2A.c \
	-x c VoiceCodecs/Opus/silk/NLSF_decode.c \
	-x c VoiceCodecs/Opus/silk/NLSF_del_dec_quant.c \
	-x c VoiceCodecs/Opus/silk/NLSF_encode.c \
	-x c VoiceCodecs/Opus/silk/NLSF_stabilize.c \
	-x c VoiceCodecs/Opus/silk/NLSF_unpack.c \
	-x c VoiceCodecs/Opus/silk/NLSF_VQ.c \
	-x c VoiceCodecs/Opus/silk/NLSF_VQ_weights_laroia.c \
	-x c VoiceCodecs/Opus/silk/NSQ.c \
	-x c VoiceCodecs/Opus/silk/NSQ_del_dec.c \
	-x c VoiceCodecs/Opus/silk/pitch_est_tables.c \
	-x c VoiceCodecs/Opus/silk/PLC.c \
	-x c VoiceCodecs/Opus/silk/process_NLSFs.c \
	-x c VoiceCodecs/Opus/silk/quant_LTP_gains.c \
	-x c VoiceCodecs/Opus/silk/resampler.c \
	-x c VoiceCodecs/Opus/silk/resampler_down2.c \
	-x c VoiceCodecs/Opus/silk/resampler_down2_3.c \
	-x c VoiceCodecs/Opus/silk/resampler_private_AR2.c \
	-x c VoiceCodecs/Opus/silk/resampler_private_down_FIR.c \
	-x c VoiceCodecs/Opus/silk/resampler_private_IIR_FIR.c \
	-x c VoiceCodecs/Opus/silk/resampler_private_up2_HQ.c \
	-x c VoiceCodecs/Opus/silk/resampler_rom.c \
	-x c VoiceCodecs/Opus/silk/shell_coder.c \
	-x c VoiceCodecs/Opus/silk/sigm_Q15.c \
	-x c VoiceCodecs/Opus/silk/sort.c \
	-x c VoiceCodecs/Opus/silk/stereo_decode_pred.c \
	-x c VoiceCodecs/Opus/silk/stereo_encode_pred.c \
	-x c VoiceCodecs/Opus/silk/stereo_find_predictor.c \
	-x c VoiceCodecs/Opus/silk/stereo_LR_to_MS.c \
	-x c VoiceCodecs/Opus/silk/stereo_MS_to_LR.c \
	-x c VoiceCodecs/Opus/silk/stereo_quant_pred.c \
	-x c VoiceCodecs/Opus/silk/sum_sqr_shift.c \
	-x c VoiceCodecs/Opus/silk/tables_gain.c \
	-x c VoiceCodecs/Opus/silk/tables_LTP.c \
	-x c VoiceCodecs/Opus/silk/tables_NLSF_CB_NB_MB.c \
	-x c VoiceCodecs/Opus/silk/tables_NLSF_CB_WB.c \
	-x c VoiceCodecs/Opus/silk/tables_other.c \
	-x c VoiceCodecs/Opus/silk/tables_pitch_lag.c \
	-x c VoiceCodecs/Opus/silk/tables_pulses_per_block.c \
	-x c VoiceCodecs/Opus/silk/table_LSF_cos.c \
	-x c VoiceCodecs/Opus/silk/VAD.c \
	-x c VoiceCodecs/Opus/silk/VQ_WMat_EC.c \
	-x c VoiceCodecs/Opus/silk/x86/VAD_sse.c \
	-x c VoiceCodecs/Opus/silk/x86/x86_silk_map.c \
	-x c VoiceCodecs/Opus/src/analysis.c \
	-x c VoiceCodecs/Opus/src/mlp.c \
	-x c VoiceCodecs/Opus/src/mlp_data.c \
	-x c VoiceCodecs/Opus/src/opus.c \
	-x c VoiceCodecs/Opus/src/opus_decoder.c \
	-x c VoiceCodecs/Opus/src/opus_encoder.c \
	-x c VoiceCodecs/Opus/src/opus_multistream.c \
	-x c VoiceCodecs/Opus/src/opus_multistream_decoder.c \
	-x c VoiceCodecs/Opus/src/opus_multistream_encoder.c \
	-x c VoiceCodecs/Opus/src/repacketizer.c \
	VoiceTranscoderSSE41_1.o \
	VoiceTranscoderSSE41_2.o \
	VoiceTranscoderSSE41_3.o \
-Wl,--gc-sections \
-o VoiceTranscoder.so