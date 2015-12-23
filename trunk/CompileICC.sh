/opt/intel/bin/icc \
		-mia32 -O3 -s -fasm-blocks -funroll-loops -fomit-frame-pointer -fno-rtti -fno-stack-protector -falign-functions=2 -fno-builtin -fno-exceptions \
		-Wno-unknown-pragmas -static-intel -shared -static-libgcc -no-intel-extensions \
-Ihlsdk/common -Ihlsdk/dlls -Ihlsdk/engine -Ihlsdk/pm_shared -Imetamod -Ispeex -Isilk -IHashers -IMultiThreading -IUtility \
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
	Utility/Time.cpp \
	Utility/UtilFunctions.cpp \
	VoiceCodecs/Speex/speex_callbacks.c \
	VoiceCodecs/Speex/speex_header.c \
	VoiceCodecs/Speex/stereo.c \
	VoiceCodecs/Speex/vbr.c \
	VoiceCodecs/Speex/vq.c \
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
	VoiceCodecs/Speex/modes.c \
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
-ldl -lm \
-o VoiceTranscoderICC.so