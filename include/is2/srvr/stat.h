//! ----------------------------------------------------------------------------
//! Copyright Edgio Inc.
//!
//! \file:    TODO
//! \details: TODO
//!
//! Licensed under the terms of the Apache 2.0 open source license.
//! Please refer to the LICENSE file in the project root for the terms.
//! ----------------------------------------------------------------------------
#ifndef _STAT_H
#define _STAT_H
//! ----------------------------------------------------------------------------
//! Includes
//! ----------------------------------------------------------------------------
// For fixed size types
#include <stdint.h>
#include <map>
#include <list>
namespace ns_is2 {
//! ----------------------------------------------------------------------------
//! Types
//! ----------------------------------------------------------------------------
typedef std::map<uint16_t, uint32_t > status_code_count_map_t;
//! ----------------------------------------------------------------------------
//! xstat
//! ----------------------------------------------------------------------------
typedef struct xstat_struct
{
        double m_sum_x;
        double m_sum_x2;
        double m_min;
        double m_max;
        uint64_t m_num;
        double min() const { return m_min; }
        double max() const { return m_max; }
        double mean() const { return (m_num > 0) ? m_sum_x / m_num : 0.0; }
        double var() const { return (m_num > 0) ? (m_sum_x2 - m_sum_x) / m_num : 0.0; }
        double stdev() const;
        xstat_struct():
                m_sum_x(0.0),
                m_sum_x2(0.0),
                m_min(1000000000.0),
                m_max(0.0),
                m_num(0)
        {}
        void clear()
        {
                m_sum_x = 0.0;
                m_sum_x2 = 0.0;
                m_min = 1000000000.0;
                m_max = 0.0;
                m_num = 0;
        };
} xstat_t;
//! ----------------------------------------------------------------------------
//! counter stats
//! ----------------------------------------------------------------------------
typedef struct t_stat_cntr_struct
{
        // Response stats
        uint64_t m_reqs;
        uint64_t m_bytes_read;
        uint64_t m_bytes_written;
        uint64_t m_resp_status_2xx;
        uint64_t m_resp_status_3xx;
        uint64_t m_resp_status_4xx;
        uint64_t m_resp_status_5xx;
        // upstream stats
        uint64_t m_upsv_reqs;
        uint64_t m_upsv_bytes_read;
        uint64_t m_upsv_bytes_written;
        uint64_t m_upsv_resp_status_2xx;
        uint64_t m_upsv_resp_status_3xx;
        uint64_t m_upsv_resp_status_4xx;
        uint64_t m_upsv_resp_status_5xx;
        t_stat_cntr_struct():
                m_reqs(0),
                m_bytes_read(0),
                m_bytes_written(0),
                m_resp_status_2xx(0),
                m_resp_status_3xx(0),
                m_resp_status_4xx(0),
                m_resp_status_5xx(0),
                m_upsv_reqs(0),
                m_upsv_bytes_read(0),
                m_upsv_bytes_written(0),
                m_upsv_resp_status_2xx(0),
                m_upsv_resp_status_3xx(0),
                m_upsv_resp_status_4xx(0),
                m_upsv_resp_status_5xx(0)
        {}
        void clear()
        {
                m_reqs = 0;
                m_bytes_read = 0;
                m_bytes_written = 0;
                m_resp_status_2xx = 0;
                m_resp_status_3xx = 0;
                m_resp_status_4xx = 0;
                m_resp_status_5xx = 0;
                m_upsv_reqs = 0;
                m_upsv_bytes_read = 0;
                m_upsv_bytes_written = 0;
                m_upsv_resp_status_2xx = 0;
                m_upsv_resp_status_3xx = 0;
                m_upsv_resp_status_4xx = 0;
                m_upsv_resp_status_5xx = 0;
        }
} t_stat_cntr_t;
typedef std::list <t_stat_cntr_t> t_stat_cntr_list_t;
//! ----------------------------------------------------------------------------
//! calculated stats
//! ----------------------------------------------------------------------------
typedef struct t_stat_calc_struct
{
        // clnt
        uint64_t m_req_delta;
        float m_req_s;
        float m_bytes_read_s;
        float m_bytes_write_s;
        float m_resp_status_2xx_pcnt;
        float m_resp_status_3xx_pcnt;
        float m_resp_status_4xx_pcnt;
        float m_resp_status_5xx_pcnt;
        // upstream
        uint64_t m_upsv_req_delta;
        uint64_t m_upsv_resp_delta;
        float m_upsv_req_s;
        float m_upsv_bytes_read_s;
        float m_upsv_bytes_write_s;
        float m_upsv_resp_status_2xx_pcnt;
        float m_upsv_resp_status_3xx_pcnt;
        float m_upsv_resp_status_4xx_pcnt;
        float m_upsv_resp_status_5xx_pcnt;
        t_stat_calc_struct():
                m_req_delta(0),
                m_req_s(0.0),
                m_bytes_read_s(0.0),
                m_bytes_write_s(0.0),
                m_resp_status_2xx_pcnt(0.0),
                m_resp_status_3xx_pcnt(0.0),
                m_resp_status_4xx_pcnt(0.0),
                m_resp_status_5xx_pcnt(0.0),
                m_upsv_req_delta(0),
                m_upsv_resp_delta(0),
                m_upsv_req_s(0.0),
                m_upsv_bytes_read_s(0.0),
                m_upsv_bytes_write_s(0.0),
                m_upsv_resp_status_2xx_pcnt(0.0),
                m_upsv_resp_status_3xx_pcnt(0.0),
                m_upsv_resp_status_4xx_pcnt(0.0),
                m_upsv_resp_status_5xx_pcnt(0.0)
        {}
        void clear()
        {
                m_req_delta = 0;
                m_req_s = 0.0;
                m_bytes_read_s = 0.0;
                m_bytes_write_s = 0.0;
                m_resp_status_2xx_pcnt = 0.0;
                m_resp_status_3xx_pcnt = 0.0;
                m_resp_status_4xx_pcnt = 0.0;
                m_resp_status_5xx_pcnt = 0.0;
                m_upsv_req_delta = 0;
                m_upsv_resp_delta = 0;
                m_upsv_req_s = 0.0;
                m_upsv_bytes_read_s = 0.0;
                m_upsv_bytes_write_s = 0.0;
                m_upsv_resp_status_2xx_pcnt = 0.0;
                m_upsv_resp_status_3xx_pcnt = 0.0;
                m_upsv_resp_status_4xx_pcnt = 0.0;
                m_upsv_resp_status_5xx_pcnt = 0.0;
        }
} t_stat_calc_t;
//! ----------------------------------------------------------------------------
//! Prototypes
//! ----------------------------------------------------------------------------
void update_stat(xstat_t &ao_stat, double a_val);
void add_stat(xstat_t &ao_stat, const xstat_t &a_from_stat);
void clear_stat(xstat_t &ao_stat);
void show_stat(const xstat_t &ao_stat);
} //namespace ns_is2 {
#endif
