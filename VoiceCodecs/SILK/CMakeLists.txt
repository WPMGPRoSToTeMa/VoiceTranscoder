cmake_minimum_required(VERSION 3.3)

if(POLICY CMP0069)
    cmake_policy(SET CMP0069 NEW)
endif()
cmake_policy(SET CMP0063 NEW)
if(POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)
endif()

project(SILK C)

add_library(${PROJECT_NAME} STATIC
    SKP_Silk_A2NLSF.c
    SKP_Silk_allpass_int.c
    SKP_Silk_allpass_int_FLP.c
    SKP_Silk_ana_filt_bank_1.c
    SKP_Silk_apply_sine_window_FLP.c
    SKP_Silk_autocorrelation_FLP.c
    SKP_Silk_biquad.c
    SKP_Silk_biquad_alt.c
    SKP_Silk_burg_modified_FLP.c
    SKP_Silk_bwexpander.c
    SKP_Silk_bwexpander_32.c
    SKP_Silk_bwexpander_FLP.c
    SKP_Silk_CNG.c
    SKP_Silk_code_signs.c
    SKP_Silk_common_pitch_est_defines.h
    SKP_Silk_control_audio_bandwidth.c
    SKP_Silk_control_codec_FLP.c
    SKP_Silk_corrMatrix_FLP.c
    SKP_Silk_create_init_destroy.c
    SKP_Silk_decimate2_coarsest_FLP.c
    SKP_Silk_decimate2_coarse_FLP.c
    SKP_Silk_decoder_set_fs.c
    SKP_Silk_decode_core.c
    SKP_Silk_decode_frame.c
    SKP_Silk_decode_parameters.c
    SKP_Silk_decode_pitch.c
    SKP_Silk_decode_pulses.c
    SKP_Silk_dec_API.c
    SKP_Silk_define.h
    SKP_Silk_detect_SWB_input.c
    SKP_Silk_encode_frame_FLP.c
    SKP_Silk_encode_parameters.c
    SKP_Silk_encode_pulses.c
    SKP_Silk_enc_API.c
    SKP_Silk_energy_FLP.c
    SKP_Silk_find_LPC_FLP.c
    SKP_Silk_find_LTP_FLP.c
    SKP_Silk_find_pitch_lags_FLP.c
    SKP_Silk_find_pred_coefs_FLP.c
    SKP_Silk_gain_quant.c
    SKP_Silk_HP_variable_cutoff_FLP.c
    SKP_Silk_init_encoder_FLP.c
    SKP_Silk_Inlines.h
    SKP_Silk_inner_product_FLP.c
    SKP_Silk_interpolate.c
    SKP_Silk_k2a_FLP.c
    SKP_Silk_LBRR_reset.c
    SKP_Silk_levinsondurbin_FLP.c
    SKP_Silk_lin2log.c
    SKP_Silk_log2lin.c
    SKP_Silk_lowpass_int.c
    SKP_Silk_lowpass_short.c
    SKP_Silk_LPC_analysis_filter_FLP.c
    SKP_Silk_LPC_inv_pred_gain.c
    SKP_Silk_LPC_inv_pred_gain_FLP.c
    SKP_Silk_LPC_synthesis_filter.c
    SKP_Silk_LPC_synthesis_order16.c
    SKP_Silk_LP_variable_cutoff.c
    SKP_Silk_LSF_cos_table.c
    SKP_Silk_LTP_analysis_filter_FLP.c
    SKP_Silk_LTP_scale_ctrl_FLP.c
    SKP_Silk_MA.c
    SKP_Silk_macros.h
    SKP_Silk_main.h
    SKP_Silk_main_FLP.h
    SKP_Silk_NLSF2A.c
    SKP_Silk_NLSF2A_stable.c
    SKP_Silk_NLSF_MSVQ_decode.c
    SKP_Silk_NLSF_MSVQ_decode_FLP.c
    SKP_Silk_NLSF_MSVQ_encode_FLP.c
    SKP_Silk_NLSF_stabilize.c
    SKP_Silk_NLSF_VQ_rate_distortion_FLP.c
    SKP_Silk_NLSF_VQ_sum_error_FLP.c
    SKP_Silk_NLSF_VQ_weights_laroia_FLP.c
    SKP_Silk_noise_shape_analysis_FLP.c
    SKP_Silk_NSQ.c
    SKP_Silk_NSQ_del_dec.c
    SKP_Silk_pitch_analysis_core_FLP.c
    SKP_Silk_pitch_est_defines.h
    SKP_Silk_pitch_est_defines_FLP.h
    SKP_Silk_pitch_est_tables.c
    SKP_Silk_PLC.c SKP_Silk_PLC.h
    SKP_Silk_prefilter_FLP.c
    SKP_Silk_process_gains_FLP.c
    SKP_Silk_process_NLSFs_FLP.c
    SKP_Silk_quant_LTP_gains_FLP.c
    SKP_Silk_range_coder.c
    SKP_Silk_regularize_correlations_FLP.c
    SKP_Silk_resampler.c
    SKP_Silk_resampler_down2.c
    SKP_Silk_resampler_down2_3.c
    SKP_Silk_resampler_down3.c
    SKP_Silk_resampler_private.h
    SKP_Silk_resampler_private_AR2.c
    SKP_Silk_resampler_private_ARMA4.c
    SKP_Silk_resampler_private_copy.c
    SKP_Silk_resampler_private_down4.c
    SKP_Silk_resampler_private_down_FIR.c
    SKP_Silk_resampler_private_IIR_FIR.c
    SKP_Silk_resampler_private_up2_HQ.c
    SKP_Silk_resampler_private_up4.c
    SKP_Silk_resampler_rom.c SKP_Silk_resampler_rom.h
    SKP_Silk_resampler_structs.h
    SKP_Silk_resampler_up2.c
    SKP_Silk_residual_energy_FLP.c
    SKP_Silk_scale_copy_vector_FLP.c
    SKP_Silk_scale_vector_FLP.c
    SKP_Silk_schur_FLP.c
    SKP_Silk_setup_complexity.h
    SKP_Silk_shell_coder.c
    SKP_Silk_sigm_Q15.c
    SKP_Silk_SigProc_FIX.h
    SKP_Silk_SigProc_FLP.h
    SKP_Silk_solve_LS_FLP.c
    SKP_Silk_sort.c
    SKP_Silk_sort_FLP.c
    SKP_Silk_structs.h
    SKP_Silk_structs_FLP.h
    SKP_Silk_sum_sqr_shift.c
    SKP_Silk_tables.h
    SKP_Silk_tables_FLP.h
    SKP_Silk_tables_gain.c
    SKP_Silk_tables_LTP.c
    SKP_Silk_tables_NLSF_CB0_10.c SKP_Silk_tables_NLSF_CB0_10.h
    SKP_Silk_tables_NLSF_CB0_10_FLP.c
    SKP_Silk_tables_NLSF_CB0_16.c SKP_Silk_tables_NLSF_CB0_16.h
    SKP_Silk_tables_NLSF_CB0_16_FLP.c
    SKP_Silk_tables_NLSF_CB1_10.c SKP_Silk_tables_NLSF_CB1_10.h
    SKP_Silk_tables_NLSF_CB1_10_FLP.c
    SKP_Silk_tables_NLSF_CB1_16.c SKP_Silk_tables_NLSF_CB1_16.h
    SKP_Silk_tables_NLSF_CB1_16_FLP.c
    SKP_Silk_tables_other.c
    SKP_Silk_tables_other_FLP.c
    SKP_Silk_tables_pitch_lag.c
    SKP_Silk_tables_pulses_per_block.c
    SKP_Silk_tables_sign.c
    SKP_Silk_tables_type_offset.c
    SKP_Silk_tuning_parameters.h
    SKP_Silk_VAD.c
    SKP_Silk_VQ_nearest_neighbor_FLP.c
    SKP_Silk_warped_autocorrelation_FLP.c
    SKP_Silk_wrappers_FLP.c
)

target_include_directories(${PROJECT_NAME} PUBLIC interface)