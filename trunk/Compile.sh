/opt/intel/bin/icc \
		-Os -fdata-sections -ffunction-sections -m32 -shared \
		-fno-builtin -fno-rtti -Qoption,cpp,--treat_func_as_string_literal_cpp -no-intel-extensions -fno-stack-protector -std=c++14 \
		-ipo -s -static-libgcc -static-intel -static-libstdc++ \
		-msse2 -fp-model strict -fomit-frame-pointer -g0 \
		-fvisibility=hidden -fPIC \
-D HAVE_CONFIG_H \
-Ihlsdk/common -Ihlsdk/dlls -Ihlsdk/engine -Ihlsdk/pm_shared -Imetamod -Ispeex -Isilk -IHashers -IMultiThreading -IUtility \
-Iopus -IVoiceCodecs/Opus -IVoiceCodecs/Opus/celt -IVoiceCodecs/Opus/celt/x86 -IVoiceCodecs/Opus/silk -IVoiceCodecs/Opus/silk/float -IVoiceCodecs/Opus/silk/x86 -IVoiceCodecs/Opus/src \
	Main.cpp \
	API.cpp \
	ThreadMode.cpp \
	VoiceCodec_Speex.cpp \
	VoiceCodec_SILK.cpp \
	Hashers/CRC32.cpp \
	MultiThreading/Mutex.cpp \
	MultiThreading/Signal.cpp \
	MultiThreading/Thread.cpp \
	Utility/BinaryPattern.cpp \
	Utility/Buffer.cpp \
	Utility/EngineUTIL.cpp \
	Utility/FunctionHook_Beginning.cpp \
	Utility/JmpOpcode.cpp \
	Utility/MemoryUnlocker.cpp \
	Utility/MetaUTIL.cpp \
	Utility/Module.cpp \
	Utility/Section.cpp \
	Utility/SteamID.cpp \
	Utility/UtilFunctions.cpp \
	VoiceCodecs/Speex/speex_callbacks.c \
	VoiceCodecs/Speex/speex_header.c \
	VoiceCodecs/Speex/stereo.c \
	VoiceCodecs/Speex/vbr.c \
	VoiceCodecs/Speex/vq_.c \
	VoiceCodecs/Speex/bits.c \
	VoiceCodecs/Speex/cb_search.c \
	VoiceCodecs/Speex/exc_5_64_table.c \
	VoiceCodecs/Speex/exc_5_256_table.c \
	VoiceCodecs/Speex/exc_8_128_table.c \
	VoiceCodecs/Speex/exc_10_16_table.c \
	VoiceCodecs/Speex/exc_10_32_table.c \
	VoiceCodecs/Speex/exc_20_32_table.c \
	VoiceCodecs/Speex/filters.c \
	VoiceCodecs/Speex/gain_table.c \
	VoiceCodecs/Speex/gain_table_lbr.c \
	VoiceCodecs/Speex/hexc_10_32_table.c \
	VoiceCodecs/Speex/hexc_table.c \
	VoiceCodecs/Speex/high_lsp_tables.c \
	VoiceCodecs/Speex/lpc.c \
	VoiceCodecs/Speex/lsp.c \
	VoiceCodecs/Speex/lsp_tables_nb.c \
	VoiceCodecs/Speex/ltp.c \
	VoiceCodecs/Speex/math_approx.c \
	VoiceCodecs/Speex/misc.c \
	VoiceCodecs/Speex/modes_.c \
	VoiceCodecs/Speex/nb_celp.c \
	VoiceCodecs/Speex/quant_lsp.c \
	VoiceCodecs/Speex/sb_celp.c \
	VoiceCodecs/SILK/SKP_Silk_tables_type_offset.c \
	VoiceCodecs/SILK/SKP_Silk_VAD.c \
	VoiceCodecs/SILK/SKP_Silk_VQ_nearest_neighbor_FLP.c \
	VoiceCodecs/SILK/SKP_Silk_warped_autocorrelation_FLP.c \
	VoiceCodecs/SILK/SKP_Silk_wrappers_FLP.c \
	VoiceCodecs/SILK/SKP_Silk_A2NLSF.c \
	VoiceCodecs/SILK/SKP_Silk_allpass_int.c \
	VoiceCodecs/SILK/SKP_Silk_allpass_int_FLP.c \
	VoiceCodecs/SILK/SKP_Silk_ana_filt_bank_1.c \
	VoiceCodecs/SILK/SKP_Silk_apply_sine_window_FLP.c \
	VoiceCodecs/SILK/SKP_Silk_autocorrelation_FLP.c \
	VoiceCodecs/SILK/SKP_Silk_biquad.c \
	VoiceCodecs/SILK/SKP_Silk_biquad_alt.c \
	VoiceCodecs/SILK/SKP_Silk_burg_modified_FLP.c \
	VoiceCodecs/SILK/SKP_Silk_bwexpander.c \
	VoiceCodecs/SILK/SKP_Silk_bwexpander_32.c \
	VoiceCodecs/SILK/SKP_Silk_bwexpander_FLP.c \
	VoiceCodecs/SILK/SKP_Silk_CNG.c \
	VoiceCodecs/SILK/SKP_Silk_code_signs.c \
	VoiceCodecs/SILK/SKP_Silk_control_audio_bandwidth.c \
	VoiceCodecs/SILK/SKP_Silk_control_codec_FLP.c \
	VoiceCodecs/SILK/SKP_Silk_corrMatrix_FLP.c \
	VoiceCodecs/SILK/SKP_Silk_create_init_destroy.c \
	VoiceCodecs/SILK/SKP_Silk_dec_API.c \
	VoiceCodecs/SILK/SKP_Silk_decimate2_coarse_FLP.c \
	VoiceCodecs/SILK/SKP_Silk_decimate2_coarsest_FLP.c \
	VoiceCodecs/SILK/SKP_Silk_decode_core.c \
	VoiceCodecs/SILK/SKP_Silk_decode_frame.c \
	VoiceCodecs/SILK/SKP_Silk_decode_parameters.c \
	VoiceCodecs/SILK/SKP_Silk_decode_pitch.c \
	VoiceCodecs/SILK/SKP_Silk_decode_pulses.c \
	VoiceCodecs/SILK/SKP_Silk_decoder_set_fs.c \
	VoiceCodecs/SILK/SKP_Silk_detect_SWB_input.c \
	VoiceCodecs/SILK/SKP_Silk_enc_API.c \
	VoiceCodecs/SILK/SKP_Silk_encode_frame_FLP.c \
	VoiceCodecs/SILK/SKP_Silk_encode_parameters.c \
	VoiceCodecs/SILK/SKP_Silk_encode_pulses.c \
	VoiceCodecs/SILK/SKP_Silk_energy_FLP.c \
	VoiceCodecs/SILK/SKP_Silk_find_LPC_FLP.c \
	VoiceCodecs/SILK/SKP_Silk_find_LTP_FLP.c \
	VoiceCodecs/SILK/SKP_Silk_find_pitch_lags_FLP.c \
	VoiceCodecs/SILK/SKP_Silk_find_pred_coefs_FLP.c \
	VoiceCodecs/SILK/SKP_Silk_gain_quant.c \
	VoiceCodecs/SILK/SKP_Silk_HP_variable_cutoff_FLP.c \
	VoiceCodecs/SILK/SKP_Silk_init_encoder_FLP.c \
	VoiceCodecs/SILK/SKP_Silk_inner_product_FLP.c \
	VoiceCodecs/SILK/SKP_Silk_interpolate.c \
	VoiceCodecs/SILK/SKP_Silk_k2a_FLP.c \
	VoiceCodecs/SILK/SKP_Silk_LBRR_reset.c \
	VoiceCodecs/SILK/SKP_Silk_levinsondurbin_FLP.c \
	VoiceCodecs/SILK/SKP_Silk_lin2log.c \
	VoiceCodecs/SILK/SKP_Silk_log2lin.c \
	VoiceCodecs/SILK/SKP_Silk_lowpass_int.c \
	VoiceCodecs/SILK/SKP_Silk_lowpass_short.c \
	VoiceCodecs/SILK/SKP_Silk_LP_variable_cutoff.c \
	VoiceCodecs/SILK/SKP_Silk_LPC_analysis_filter_FLP.c \
	VoiceCodecs/SILK/SKP_Silk_LPC_inv_pred_gain.c \
	VoiceCodecs/SILK/SKP_Silk_LPC_inv_pred_gain_FLP.c \
	VoiceCodecs/SILK/SKP_Silk_LPC_synthesis_filter.c \
	VoiceCodecs/SILK/SKP_Silk_LPC_synthesis_order16.c \
	VoiceCodecs/SILK/SKP_Silk_LSF_cos_table.c \
	VoiceCodecs/SILK/SKP_Silk_LTP_analysis_filter_FLP.c \
	VoiceCodecs/SILK/SKP_Silk_LTP_scale_ctrl_FLP.c \
	VoiceCodecs/SILK/SKP_Silk_MA.c \
	VoiceCodecs/SILK/SKP_Silk_NLSF_MSVQ_decode.c \
	VoiceCodecs/SILK/SKP_Silk_NLSF_MSVQ_decode_FLP.c \
	VoiceCodecs/SILK/SKP_Silk_NLSF_MSVQ_encode_FLP.c \
	VoiceCodecs/SILK/SKP_Silk_NLSF_stabilize.c \
	VoiceCodecs/SILK/SKP_Silk_NLSF_VQ_rate_distortion_FLP.c \
	VoiceCodecs/SILK/SKP_Silk_NLSF_VQ_sum_error_FLP.c \
	VoiceCodecs/SILK/SKP_Silk_NLSF_VQ_weights_laroia_FLP.c \
	VoiceCodecs/SILK/SKP_Silk_NLSF2A.c \
	VoiceCodecs/SILK/SKP_Silk_NLSF2A_stable.c \
	VoiceCodecs/SILK/SKP_Silk_noise_shape_analysis_FLP.c \
	VoiceCodecs/SILK/SKP_Silk_NSQ.c \
	VoiceCodecs/SILK/SKP_Silk_NSQ_del_dec.c \
	VoiceCodecs/SILK/SKP_Silk_pitch_analysis_core_FLP.c \
	VoiceCodecs/SILK/SKP_Silk_pitch_est_tables.c \
	VoiceCodecs/SILK/SKP_Silk_PLC.c \
	VoiceCodecs/SILK/SKP_Silk_prefilter_FLP.c \
	VoiceCodecs/SILK/SKP_Silk_process_gains_FLP.c \
	VoiceCodecs/SILK/SKP_Silk_process_NLSFs_FLP.c \
	VoiceCodecs/SILK/SKP_Silk_quant_LTP_gains_FLP.c \
	VoiceCodecs/SILK/SKP_Silk_range_coder.c \
	VoiceCodecs/SILK/SKP_Silk_regularize_correlations_FLP.c \
	VoiceCodecs/SILK/SKP_Silk_resampler.c \
	VoiceCodecs/SILK/SKP_Silk_resampler_down2.c \
	VoiceCodecs/SILK/SKP_Silk_resampler_down2_3.c \
	VoiceCodecs/SILK/SKP_Silk_resampler_down3.c \
	VoiceCodecs/SILK/SKP_Silk_resampler_private_AR2.c \
	VoiceCodecs/SILK/SKP_Silk_resampler_private_ARMA4.c \
	VoiceCodecs/SILK/SKP_Silk_resampler_private_copy.c \
	VoiceCodecs/SILK/SKP_Silk_resampler_private_down_FIR.c \
	VoiceCodecs/SILK/SKP_Silk_resampler_private_down4.c \
	VoiceCodecs/SILK/SKP_Silk_resampler_private_IIR_FIR.c \
	VoiceCodecs/SILK/SKP_Silk_resampler_private_up2_HQ.c \
	VoiceCodecs/SILK/SKP_Silk_resampler_private_up4.c \
	VoiceCodecs/SILK/SKP_Silk_resampler_rom.c \
	VoiceCodecs/SILK/SKP_Silk_resampler_up2.c \
	VoiceCodecs/SILK/SKP_Silk_residual_energy_FLP.c \
	VoiceCodecs/SILK/SKP_Silk_scale_copy_vector_FLP.c \
	VoiceCodecs/SILK/SKP_Silk_scale_vector_FLP.c \
	VoiceCodecs/SILK/SKP_Silk_schur_FLP.c \
	VoiceCodecs/SILK/SKP_Silk_shell_coder.c \
	VoiceCodecs/SILK/SKP_Silk_sigm_Q15.c \
	VoiceCodecs/SILK/SKP_Silk_solve_LS_FLP.c \
	VoiceCodecs/SILK/SKP_Silk_sort.c \
	VoiceCodecs/SILK/SKP_Silk_sort_FLP.c \
	VoiceCodecs/SILK/SKP_Silk_sum_sqr_shift.c \
	VoiceCodecs/SILK/SKP_Silk_tables_gain.c \
	VoiceCodecs/SILK/SKP_Silk_tables_LTP.c \
	VoiceCodecs/SILK/SKP_Silk_tables_NLSF_CB0_10.c \
	VoiceCodecs/SILK/SKP_Silk_tables_NLSF_CB0_10_FLP.c \
	VoiceCodecs/SILK/SKP_Silk_tables_NLSF_CB0_16.c \
	VoiceCodecs/SILK/SKP_Silk_tables_NLSF_CB0_16_FLP.c \
	VoiceCodecs/SILK/SKP_Silk_tables_NLSF_CB1_10.c \
	VoiceCodecs/SILK/SKP_Silk_tables_NLSF_CB1_10_FLP.c \
	VoiceCodecs/SILK/SKP_Silk_tables_NLSF_CB1_16.c \
	VoiceCodecs/SILK/SKP_Silk_tables_NLSF_CB1_16_FLP.c \
	VoiceCodecs/SILK/SKP_Silk_tables_other.c \
	VoiceCodecs/SILK/SKP_Silk_tables_other_FLP.c \
	VoiceCodecs/SILK/SKP_Silk_tables_pitch_lag.c \
	VoiceCodecs/SILK/SKP_Silk_tables_pulses_per_block.c \
	VoiceCodecs/SILK/SKP_Silk_tables_sign.c \
	VoiceCodecs/Opus/celt/bands.c \
	VoiceCodecs/Opus/celt/celt.c \
	VoiceCodecs/Opus/celt/celt_decoder.c \
	VoiceCodecs/Opus/celt/celt_encoder.c \
	VoiceCodecs/Opus/celt/celt_lpc.c \
	VoiceCodecs/Opus/celt/cwrs.c \
	VoiceCodecs/Opus/celt/entcode.c \
	VoiceCodecs/Opus/celt/entdec.c \
	VoiceCodecs/Opus/celt/entenc.c \
	VoiceCodecs/Opus/celt/kiss_fft.c \
	VoiceCodecs/Opus/celt/laplace.c \
	VoiceCodecs/Opus/celt/mathops.c \
	VoiceCodecs/Opus/celt/mdct.c \
	VoiceCodecs/Opus/celt/modes.c \
	VoiceCodecs/Opus/celt/pitch.c \
	VoiceCodecs/Opus/celt/quant_bands.c \
	VoiceCodecs/Opus/celt/rate.c \
	VoiceCodecs/Opus/celt/vq.c \
	VoiceCodecs/Opus/celt/x86/celt_lpc_sse.c \
	VoiceCodecs/Opus/celt/x86/pitch_sse.c \
	VoiceCodecs/Opus/celt/x86/pitch_sse2.c \
	VoiceCodecs/Opus/celt/x86/pitch_sse4_1.c \
	VoiceCodecs/Opus/celt/x86/vq_sse2.c \
	VoiceCodecs/Opus/celt/x86/x86cpu.c \
	VoiceCodecs/Opus/celt/x86/x86_celt_map.c \
	VoiceCodecs/Opus/silk/A2NLSF.c \
	VoiceCodecs/Opus/silk/ana_filt_bank_1.c \
	VoiceCodecs/Opus/silk/biquad_alt.c \
	VoiceCodecs/Opus/silk/bwexpander.c \
	VoiceCodecs/Opus/silk/bwexpander_32.c \
	VoiceCodecs/Opus/silk/check_control_input.c \
	VoiceCodecs/Opus/silk/CNG.c \
	VoiceCodecs/Opus/silk/code_signs.c \
	VoiceCodecs/Opus/silk/control_audio_bandwidth.c \
	VoiceCodecs/Opus/silk/control_codec.c \
	VoiceCodecs/Opus/silk/control_SNR.c \
	VoiceCodecs/Opus/silk/debug.c \
	VoiceCodecs/Opus/silk/decoder_set_fs.c \
	VoiceCodecs/Opus/silk/decode_core.c \
	VoiceCodecs/Opus/silk/decode_frame.c \
	VoiceCodecs/Opus/silk/decode_indices.c \
	VoiceCodecs/Opus/silk/decode_parameters.c \
	VoiceCodecs/Opus/silk/decode_pitch.c \
	VoiceCodecs/Opus/silk/decode_pulses.c \
	VoiceCodecs/Opus/silk/dec_API.c \
	VoiceCodecs/Opus/silk/encode_indices.c \
	VoiceCodecs/Opus/silk/encode_pulses.c \
	VoiceCodecs/Opus/silk/enc_API.c \
	VoiceCodecs/Opus/silk/float/apply_sine_window_FLP.c \
	VoiceCodecs/Opus/silk/float/autocorrelation_FLP.c \
	VoiceCodecs/Opus/silk/float/burg_modified_FLP.c \
	VoiceCodecs/Opus/silk/float/bwexpander_FLP.c \
	VoiceCodecs/Opus/silk/float/corrMatrix_FLP.c \
	VoiceCodecs/Opus/silk/float/encode_frame_FLP.c \
	VoiceCodecs/Opus/silk/float/energy_FLP.c \
	VoiceCodecs/Opus/silk/float/find_LPC_FLP.c \
	VoiceCodecs/Opus/silk/float/find_LTP_FLP.c \
	VoiceCodecs/Opus/silk/float/find_pitch_lags_FLP.c \
	VoiceCodecs/Opus/silk/float/find_pred_coefs_FLP.c \
	VoiceCodecs/Opus/silk/float/inner_product_FLP.c \
	VoiceCodecs/Opus/silk/float/k2a_FLP.c \
	VoiceCodecs/Opus/silk/float/LPC_analysis_filter_FLP.c \
	VoiceCodecs/Opus/silk/float/LPC_inv_pred_gain_FLP.c \
	VoiceCodecs/Opus/silk/float/LTP_analysis_filter_FLP.c \
	VoiceCodecs/Opus/silk/float/LTP_scale_ctrl_FLP.c \
	VoiceCodecs/Opus/silk/float/noise_shape_analysis_FLP.c \
	VoiceCodecs/Opus/silk/float/pitch_analysis_core_FLP.c \
	VoiceCodecs/Opus/silk/float/process_gains_FLP.c \
	VoiceCodecs/Opus/silk/float/regularize_correlations_FLP.c \
	VoiceCodecs/Opus/silk/float/residual_energy_FLP.c \
	VoiceCodecs/Opus/silk/float/scale_copy_vector_FLP.c \
	VoiceCodecs/Opus/silk/float/scale_vector_FLP.c \
	VoiceCodecs/Opus/silk/float/schur_FLP.c \
	VoiceCodecs/Opus/silk/float/sort_FLP.c \
	VoiceCodecs/Opus/silk/float/warped_autocorrelation_FLP.c \
	VoiceCodecs/Opus/silk/float/wrappers_FLP.c \
	VoiceCodecs/Opus/silk/gain_quant.c \
	VoiceCodecs/Opus/silk/HP_variable_cutoff.c \
	VoiceCodecs/Opus/silk/init_decoder.c \
	VoiceCodecs/Opus/silk/init_encoder.c \
	VoiceCodecs/Opus/silk/inner_prod_aligned.c \
	VoiceCodecs/Opus/silk/interpolate.c \
	VoiceCodecs/Opus/silk/lin2log.c \
	VoiceCodecs/Opus/silk/log2lin.c \
	VoiceCodecs/Opus/silk/LPC_analysis_filter.c \
	VoiceCodecs/Opus/silk/LPC_fit.c \
	VoiceCodecs/Opus/silk/LPC_inv_pred_gain.c \
	VoiceCodecs/Opus/silk/LP_variable_cutoff.c \
	VoiceCodecs/Opus/silk/NLSF2A.c \
	VoiceCodecs/Opus/silk/NLSF_decode.c \
	VoiceCodecs/Opus/silk/NLSF_del_dec_quant.c \
	VoiceCodecs/Opus/silk/NLSF_encode.c \
	VoiceCodecs/Opus/silk/NLSF_stabilize.c \
	VoiceCodecs/Opus/silk/NLSF_unpack.c \
	VoiceCodecs/Opus/silk/NLSF_VQ.c \
	VoiceCodecs/Opus/silk/NLSF_VQ_weights_laroia.c \
	VoiceCodecs/Opus/silk/NSQ.c \
	VoiceCodecs/Opus/silk/NSQ_del_dec.c \
	VoiceCodecs/Opus/silk/pitch_est_tables.c \
	VoiceCodecs/Opus/silk/PLC.c \
	VoiceCodecs/Opus/silk/process_NLSFs.c \
	VoiceCodecs/Opus/silk/quant_LTP_gains.c \
	VoiceCodecs/Opus/silk/resampler.c \
	VoiceCodecs/Opus/silk/resampler_down2.c \
	VoiceCodecs/Opus/silk/resampler_down2_3.c \
	VoiceCodecs/Opus/silk/resampler_private_AR2.c \
	VoiceCodecs/Opus/silk/resampler_private_down_FIR.c \
	VoiceCodecs/Opus/silk/resampler_private_IIR_FIR.c \
	VoiceCodecs/Opus/silk/resampler_private_up2_HQ.c \
	VoiceCodecs/Opus/silk/resampler_rom.c \
	VoiceCodecs/Opus/silk/shell_coder.c \
	VoiceCodecs/Opus/silk/sigm_Q15.c \
	VoiceCodecs/Opus/silk/sort.c \
	VoiceCodecs/Opus/silk/stereo_decode_pred.c \
	VoiceCodecs/Opus/silk/stereo_encode_pred.c \
	VoiceCodecs/Opus/silk/stereo_find_predictor.c \
	VoiceCodecs/Opus/silk/stereo_LR_to_MS.c \
	VoiceCodecs/Opus/silk/stereo_MS_to_LR.c \
	VoiceCodecs/Opus/silk/stereo_quant_pred.c \
	VoiceCodecs/Opus/silk/sum_sqr_shift.c \
	VoiceCodecs/Opus/silk/tables_gain.c \
	VoiceCodecs/Opus/silk/tables_LTP.c \
	VoiceCodecs/Opus/silk/tables_NLSF_CB_NB_MB.c \
	VoiceCodecs/Opus/silk/tables_NLSF_CB_WB.c \
	VoiceCodecs/Opus/silk/tables_other.c \
	VoiceCodecs/Opus/silk/tables_pitch_lag.c \
	VoiceCodecs/Opus/silk/tables_pulses_per_block.c \
	VoiceCodecs/Opus/silk/table_LSF_cos.c \
	VoiceCodecs/Opus/silk/VAD.c \
	VoiceCodecs/Opus/silk/VQ_WMat_EC.c \
	VoiceCodecs/Opus/silk/x86/NSQ_del_dec_sse.c \
	VoiceCodecs/Opus/silk/x86/NSQ_sse.c \
	VoiceCodecs/Opus/silk/x86/VAD_sse.c \
	VoiceCodecs/Opus/silk/x86/VQ_WMat_EC_sse.c \
	VoiceCodecs/Opus/silk/x86/x86_silk_map.c \
	VoiceCodecs/Opus/src/analysis.c \
	VoiceCodecs/Opus/src/mlp.c \
	VoiceCodecs/Opus/src/mlp_data.c \
	VoiceCodecs/Opus/src/opus.c \
	VoiceCodecs/Opus/src/opus_decoder.c \
	VoiceCodecs/Opus/src/opus_encoder.c \
	VoiceCodecs/Opus/src/opus_multistream.c \
	VoiceCodecs/Opus/src/opus_multistream_decoder.c \
	VoiceCodecs/Opus/src/opus_multistream_encoder.c \
	VoiceCodecs/Opus/src/repacketizer.c \
-lrt -ldl -lm -lpthread -Wl,--gc-sections \
-o VoiceTranscoder.so