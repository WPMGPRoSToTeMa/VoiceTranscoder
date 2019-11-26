cmake_minimum_required(VERSION 3.3)

if(POLICY CMP0069)
    cmake_policy(SET CMP0069 NEW)
endif()
cmake_policy(SET CMP0063 NEW)
if(POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)
endif()

project(Speex C)

add_library(${PROJECT_NAME} STATIC
    bits.c
    cb_search.c cb_search.h
    exc_10_16_table.c
    exc_10_32_table.c
    exc_20_32_table.c
    exc_5_256_table.c
    exc_5_64_table.c
    exc_8_128_table.c
    filters.c filters.h
    filters_sse.h
    gain_table.c
    gain_table_lbr.c
    hexc_10_32_table.c
    hexc_table.c
    high_lsp_tables.c
    lpc.c lpc.h
    lsp.c lsp.h
    lsp_tables_nb.c
    ltp.c ltp.h
    ltp_sse.h
    math_approx.c math_approx.h
    misc.c misc.h
    modes.c modes.h
    nb_celp.c nb_celp.h
    quant_lsp.c quant_lsp.h
    sb_celp.c sb_celp.h
    speex_callbacks.c
    speex_header.c
    stack_alloc.h
    stereo.c
    vbr.c vbr.h
    vq.c vq.h
)

target_include_directories(${PROJECT_NAME} PUBLIC speex)