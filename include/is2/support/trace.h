//! ----------------------------------------------------------------------------
//! Copyright Edgio Inc.
//!
//! \file:    TODO
//! \details: TODO
//!
//! Licensed under the terms of the Apache 2.0 open source license.
//! Please refer to the LICENSE file in the project root for the terms.
//! ----------------------------------------------------------------------------
#ifndef _TRACE_H
#define _TRACE_H
//! ----------------------------------------------------------------------------
//! Includes
//! ----------------------------------------------------------------------------
#include <stdint.h>
#include <stdio.h>
#include <string>
#include <is2/support/time_util.h>
//! ----------------------------------------------------------------------------
//! trace macros
//! ----------------------------------------------------------------------------
// TODO -open file if NULL???
#ifndef TRC_PRINT
#define TRC_PRINT(_level, ...) \
        do { \
        if(ns_is2::g_trc_log_file)\
        {\
        if(ns_is2::g_trc_log_level >= _level) { \
        fprintf(ns_is2::g_trc_log_file, "%.3f %s %s:%s.%d: ", \
                        ((double)ns_is2::get_time_ms())/1000.0, \
                        ns_is2::trc_log_level_str(_level), \
                        __FILE__, __FUNCTION__, __LINE__); \
        fprintf(ns_is2::g_trc_log_file, __VA_ARGS__); \
        fflush(ns_is2::g_trc_log_file); \
        } \
        } \
        } while(0)
#endif
#ifndef TRC_MEM
#define TRC_MEM(_level, _buf, _len) \
        do { \
        if(ns_is2::g_trc_log_file)\
        {\
        if(ns_is2::g_trc_log_level >= _level) { \
        ns_is2::trc_mem_display(ns_is2::g_trc_log_file, _buf, _len);\
        fflush(ns_is2::g_trc_log_file); \
        } \
        } \
        } while(0)
#endif
//! ----------------------------------------------------------------------------
//! trace levels
//! ----------------------------------------------------------------------------
#ifndef TRC_ERROR
#define TRC_ERROR(...)  TRC_PRINT(ns_is2::TRC_LOG_LEVEL_ERROR, __VA_ARGS__)
#endif
#ifndef TRC_WARN
#define TRC_WARN(...)  TRC_PRINT(ns_is2::TRC_LOG_LEVEL_WARN, __VA_ARGS__)
#endif
#ifndef TRC_DEBUG
#define TRC_DEBUG(...)  TRC_PRINT(ns_is2::TRC_LOG_LEVEL_DEBUG, __VA_ARGS__)
#endif
#ifndef TRC_VERBOSE
#define TRC_VERBOSE(...)  TRC_PRINT(ns_is2::TRC_LOG_LEVEL_VERBOSE, __VA_ARGS__)
#endif
#ifndef TRC_ALL
#define TRC_ALL(...)  TRC_PRINT(ns_is2::TRC_LOG_LEVEL_ALL, __VA_ARGS__)
#endif
#ifndef TRC_ALL_MEM
#define TRC_ALL_MEM(_buf,_len) TRC_MEM(ns_is2::TRC_LOG_LEVEL_ALL, _buf, _len)
#endif
// TODO -open file if NULL???
#ifndef TRC_OUTPUT
#define TRC_OUTPUT(...) \
        do { \
        if(ns_is2::g_trc_out_file)\
        {\
        fprintf(ns_is2::g_trc_out_file, __VA_ARGS__); \
        fflush(ns_is2::g_trc_out_file); \
        }\
        } while(0)
#endif
namespace ns_is2 {
//! ----------------------------------------------------------------------------
//! trc enum
//! ----------------------------------------------------------------------------
#ifndef _TRC_LOG_LEVEL_T
#define TRC_LOG_LEVEL_MAP(XX)\
        XX(0,  NONE,        N)\
        XX(1,  ERROR,       E)\
        XX(2,  WARN,        W)\
        XX(3,  DEBUG,       D)\
        XX(4,  VERBOSE,     V)\
        XX(5,  ALL,         A)
typedef enum trc_level_enum
{
#define XX(num, name, string) TRC_LOG_LEVEL_##name = num,
        TRC_LOG_LEVEL_MAP(XX)
#undef XX
} trc_log_level_t;
#endif
const char *trc_log_level_str(trc_log_level_t a_level);
//! ----------------------------------------------------------------------------
//! Open logs
//! ----------------------------------------------------------------------------
void trc_log_level_set(trc_log_level_t a_level);
int32_t trc_log_file_open(const std::string &a_file);
int32_t trc_log_file_close(void);
//! ----------------------------------------------------------------------------
//! Externs
//! ----------------------------------------------------------------------------
extern trc_log_level_t g_trc_log_level;
extern FILE* g_trc_log_file;
extern FILE* g_trc_out_file;
//! ----------------------------------------------------------------------------
//! Utilities
//! ----------------------------------------------------------------------------
void trc_mem_display(FILE *a_file, const uint8_t *a_mem_buf, uint32_t a_length);
} // namespace ns_is2 {
#endif // NDEBUG_H_
