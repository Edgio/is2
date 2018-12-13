//: ----------------------------------------------------------------------------
//: Copyright (C) 2018 Verizon.  All Rights Reserved.
//: All Rights Reserved
//:
//: \file:    proxy_h.cc
//: \details: TODO
//: \author:  Reed P. Morrison
//: \date:    01/19/2016
//:
//:   Licensed under the Apache License, Version 2.0 (the "License");
//:   you may not use this file except in compliance with the License.
//:   You may obtain a copy of the License at
//:
//:       http://www.apache.org/licenses/LICENSE-2.0
//:
//:   Unless required by applicable law or agreed to in writing, software
//:   distributed under the License is distributed on an "AS IS" BASIS,
//:   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//:   See the License for the specific language governing permissions and
//:   limitations under the License.
//:
//: ----------------------------------------------------------------------------
//: ----------------------------------------------------------------------------
//: includes
//: ----------------------------------------------------------------------------
#include "srvr/t_srvr.h"
#include "srvr/ups_session.h"
#include "support/ndebug.h"
#include "nconn/nconn_tls.h"
#include "http_parser/http_parser.h"
#include "dns/nresolver.h"
#include "is2/support/nbq.h"
#include "is2/support/trace.h"
#include "is2/status.h"
#include "is2/srvr/subr.h"
#include "is2/srvr/session.h"
#include "is2/srvr/base_u.h"
#include "is2/srvr/resp.h"
#include "is2/handler/proxy_h.h"
//: ----------------------------------------------------------------------------
//: constants
//: ----------------------------------------------------------------------------
#ifdef ASYNC_DNS_SUPPORT
#ifndef STATUS_QUEUED_ASYNC_DNS
#define STATUS_QUEUED_ASYNC_DNS -3
#endif
#endif
//: ----------------------------------------------------------------------------
//: macros
//: ----------------------------------------------------------------------------
#define CHECK_FOR_NULL_ERROR_DEBUG(_data) \
        do {\
                if(!_data) {\
                        NDBG_PRINT("Error.\n");\
                        return STATUS_ERROR;\
                }\
        } while(0);

#define CHECK_FOR_NULL_ERROR(_data) \
        do {\
                if(!_data) {\
                        return STATUS_ERROR;\
                }\
        } while(0);

#define _SET_NCONN_OPT(_conn, _opt, _buf, _len) \
        do { \
                int _status = 0; \
                _status = _conn.set_opt((_opt), (_buf), (_len)); \
                if (_status != nconn::NC_STATUS_OK) { \
                        TRC_ERROR("set_opt %d.  Status: %d.\n", \
                                   _opt, _status); \
                        return STATUS_ERROR;\
                } \
        } while(0)

namespace ns_is2 {
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
#ifdef ASYNC_DNS_SUPPORT
int32_t adns_resolved_cb(const host_info *a_host_info, void *a_data)
{
        //NDBG_PRINT("%sRESOLVE_CB%s: a_host_info: %p a_data: %p\n",
        //                ANSI_COLOR_FG_WHITE, ANSI_COLOR_OFF,
        //                a_host_info, a_data);
        if(!a_data)
        {
                TRC_ERROR("subr == NULL\n");
                return STATUS_ERROR;
        }
        subr &l_subr = *(static_cast<subr *>(a_data));
        l_subr.m_lookup_job = NULL;
        // TODO DEBUG???
        //l_t_srvr->m_stat.m_subr_pending_resolv_map.erase(l_subr->get_label());
        if(!a_host_info)
        {
                //NDBG_PRINT("a_host_info == NULL --HOST: %s\n", l_subr->get_host().c_str());
                //NDBG_PRINT("l_host_info null\n");
                //++(l_t_srvr->m_stat.m_upsv_errors);
                if(l_subr.m_error_cb)
                {
                        l_subr.m_error_cb(l_subr, NULL, HTTP_STATUS_BAD_GATEWAY, "address lookup failure");
                        // TODO check status
                        return STATUS_OK;
                }
                return STATUS_OK;
        }
        //NDBG_PRINT("l_subr: %p -HOST: %s\n", l_subr, l_subr->get_host().c_str());
        l_subr.m_host_info = *a_host_info;
        //++(l_t_srvr->m_stat.m_dns_resolved);
        ups_session::subr_enqueue(l_subr);
        return STATUS_OK;
}
#endif
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
ups_session::ups_session(subr &a_subr):
        m_subr(a_subr),
        m_resp(NULL),
        m_nconn(NULL),
        m_in_q(NULL),
        m_in_q_detached(false),
        m_out_q(NULL),
#if 0
        m_body_q(NULL),
        m_keepalive(false),
#endif
        m_detach_resp(false),
        m_again(false),
        m_timeout_ms(10000),
        m_last_active_ms(0),
        m_evr_timeout(NULL),
        m_evr_readable(NULL),
        m_evr_writeable(NULL)
{
        //NDBG_PRINT("%sCONSTRUCT%s\n", ANSI_COLOR_BG_GREEN, ANSI_COLOR_OFF);
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
ups_session::~ups_session(void)
{
        //NDBG_PRINT("%sDESTROY%s: nconn: %p ups: %p \n", ANSI_COLOR_FG_RED, ANSI_COLOR_OFF, m_nconn, this);
        int32_t l_s;
        l_s = cancel_evr_timer();
        // TODO check status
        UNUSED(l_s);
        l_s = cancel_evr_readable();
        // TODO check status
        UNUSED(l_s);
        l_s = cancel_evr_writeable();
        // TODO check status
        UNUSED(l_s);
        if(m_resp)
        {
                delete m_resp;
                m_resp = NULL;
        }
        if(m_in_q)
        {
                if(!m_in_q_detached)
                {
                        delete m_in_q;
                }
                m_in_q = NULL;
        }
        if(m_out_q)
        {
                delete m_out_q;
                m_out_q = NULL;
        }
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
int32_t ups_session::teardown(ups_session *a_ups,
                              t_srvr &a_t_srvr,
                              nconn &a_nconn,
                              http_status_t a_status)
{
        //NDBG_PRINT("%sTEARDOWN%s: a_nconn: %p a_status: %8d a_ups: %p\n",
        //           ANSI_COLOR_FG_RED, ANSI_COLOR_OFF,
        //           &a_nconn, a_status, a_ups);
        if(!a_ups)
        {
                //++m_stat.m_upsv_conn_completed;
                if(a_t_srvr.m_nconn_proxy_pool.release(&a_nconn) != STATUS_OK)
                {
                        TRC_ERROR("performing m_nconn_proxy_pool.release: a_nconn: %p\n", &a_nconn);
                }
                //m_stat.m_pool_proxy_conn_active = m_nconn_proxy_pool.get_active_size();
                //m_stat.m_pool_proxy_conn_idle = m_nconn_proxy_pool.get_idle_size();
                return STATUS_OK;
        }
        ups_session &l_ups = *a_ups;
        l_ups.m_subr.m_state = subr::SUBR_STATE_NONE;;
        // TODO FIX!!!
#if 0
        l_ups.set_shutdown();
        l_ups.set_ups_done();
#endif
        if(a_status != HTTP_STATUS_OK)
        {
                if(l_ups.m_resp)
                {
                        l_ups.m_resp->set_status(a_status);
                }
                // -------------------------------------------------
                // TODO FIX!!!
                // -------------------------------------------------
#if 0
                subr_log_status(a_status);
                if(m_subr)
                {
                        m_subr->set_end_time_ms(get_time_ms());
                        bool l_detach_resp = m_subr->m_detach_resp;
                        subr::error_cb_t l_error_cb = m_subr->get_error_cb();
                        if(l_error_cb)
                        {
                                const char *l_err_str = NULL;
                                if(m_nconn)
                                {
                                        l_err_str = m_nconn->get_last_error().c_str();
                                }
                                l_error_cb(*(m_subr), m_nconn, a_status, l_err_str);
                                // TODO Check status...
                        }
                        if(l_detach_resp)
                        {
                                m_subr = NULL;
                        }
                }
#endif
                // -------------------------------------------------
                // TODO FIX!!!
                // -------------------------------------------------
#if 0
                m_subr->set_end_time_ms(get_time_ms());
                if(m_nconn && m_nconn->get_collect_stats_flag())
                {
                        m_nconn->set_stat_tt_completion_us(get_delta_time_us(m_nconn->get_connect_start_time_us()));
                        m_nconn->reset_stats();
                }
#endif
        }
        //++m_stat.m_upsv_conn_completed;
        if(a_t_srvr.m_nconn_proxy_pool.release(&a_nconn) != STATUS_OK)
        {
                TRC_ERROR("performing m_nconn_proxy_pool.release: a_nconn: %p\n", &a_nconn);
        }
        //m_stat.m_pool_proxy_conn_active = m_nconn_proxy_pool.get_active_size();
        //m_stat.m_pool_proxy_conn_idle = m_nconn_proxy_pool.get_idle_size();
        return STATUS_OK;
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
int32_t ups_session::queue_input(void)
{
        if(m_evr_readable)
        {
                // one's already queue'd up
                return STATUS_OK;
        }
        int32_t l_s;
        l_s = m_subr.m_session.m_t_srvr.queue_event(&m_evr_readable,
                                                    ups_session::evr_event_readable_cb,
                                                    m_nconn);
        if(l_s != STATUS_OK)
        {
                TRC_ERROR("performing queue_input\n");
                return STATUS_ERROR;
        }
        return STATUS_OK;
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
static int32_t run_state_machine(void *a_data, evr_mode_t a_conn_mode)
{
        //NDBG_PRINT("RUN a_conn_mode: %d a_data: %p\n", a_conn_mode, a_data);
        //CHECK_FOR_NULL_ERROR(a_data);
        // TODO -return OK for a_data == NULL
        if(!a_data)
        {
                return STATUS_OK;
        }
        nconn &l_nconn = *(static_cast<nconn*>(a_data));
        if(!l_nconn.get_ctx())
        {
                TRC_ERROR("no t_srvr associated with connection -cleaning up: nconn: label: %s\n",
                          l_nconn.get_label().c_str());
                l_nconn.nc_cleanup();
                return STATUS_ERROR;
        }
        // -------------------------------------------------
        // check for cancelled
        // -------------------------------------------------
        if(l_nconn.get_status() == CONN_STATUS_CANCELLED)
        {
                return STATUS_DONE;
        }
        t_srvr &l_t_srvr = *(static_cast<t_srvr *>(l_nconn.get_ctx()));
        ups_session *l_ups = static_cast<ups_session *>(l_nconn.get_data());
        // -------------------------------------------------
        // mode switch
        // -------------------------------------------------
        switch(a_conn_mode)
        {
        // -------------------------------------------------
        // ERROR
        // -------------------------------------------------
        case EVR_MODE_ERROR:
        {
                TRC_ERROR("connection error: label: %s\n", l_nconn.get_label().c_str());
                int32_t l_s;
                l_s = ups_session::teardown(l_ups, l_t_srvr, l_nconn, HTTP_STATUS_BAD_GATEWAY);
                // TODO -check status...
                UNUSED(l_s);
                return STATUS_DONE;
        }
        // -------------------------------------------------
        // TIMEOUT
        // -------------------------------------------------
        case EVR_MODE_TIMEOUT:
        {
                // ignore timeout for free connections
                if(l_nconn.is_free())
                {
                        TRC_ERROR("call back for free connection\n");
                        return STATUS_OK;
                }
                // calc time since last active
                if(!l_ups)
                {
                        TRC_ERROR("a_conn_mode[%d] ups_session[%p] || t_srvr[%p] == NULL\n",
                                        a_conn_mode,
                                        l_ups,
                                        &l_t_srvr);
                        return STATUS_ERROR;
                }
                // -----------------------------------------
                // timeout
                // -----------------------------------------
                uint64_t l_ct_ms = get_time_ms();
                if(((uint32_t)(l_ct_ms - l_ups->m_last_active_ms)) >= l_ups->m_timeout_ms)
                {
                        //++(l_t_srvr->m_stat.m_upsv_errors);
                        //++(l_t_srvr->m_stat.m_upsv_idle_killed);
                        //TRC_VERBOSE("teardown status: %d\n", HTTP_STATUS_GATEWAY_TIMEOUT);
                        TRC_ERROR("connection error: label: %s\n", l_nconn.get_label().c_str());
                        int32_t l_s;
                        l_s = ups_session::teardown(l_ups, l_t_srvr, l_nconn, HTTP_STATUS_BAD_GATEWAY);
                        // TODO -check status...
                        UNUSED(l_s);
                        return STATUS_DONE;
                }
                // -----------------------------------------
                // active -create new timer with
                // delta time
                // -----------------------------------------
                uint64_t l_d_time = (uint32_t)(l_ups->m_timeout_ms - (l_ct_ms - l_ups->m_last_active_ms));
                int32_t l_s;
                l_s = l_t_srvr.add_timer(l_d_time,
                                         ups_session::evr_event_timeout_cb,
                                         &l_nconn,
                                         (void **)(&(l_ups->m_evr_timeout)));
                // TODO check status
                UNUSED(l_s);
                // TODO check status
                return STATUS_OK;
        }
        // -------------------------------------------------
        // EVR_MODE_READ
        // -------------------------------------------------
        case EVR_MODE_READ:
        {
                // ignore readable for free connections
                if(l_nconn.is_free())
                {
                        TRC_ERROR("call back for free connection\n");
                        return STATUS_OK;
                }
                // -----------------------------------------
                // skip reads for sessions stuck in again
                // state
                // -----------------------------------------
                if(l_ups &&
                   l_ups->m_again)
                {
                        return STATUS_OK;
                }
                break;
        }
        // -------------------------------------------------
        // EVR_MODE_WRITE
        // -------------------------------------------------
        case EVR_MODE_WRITE:
        {
                break;
        }
        // -------------------------------------------------
        // default
        // -------------------------------------------------
        default:
        {
                TRC_ERROR("unknown a_conn_mode: %d\n", a_conn_mode);
                return STATUS_OK;
        }
        }
        // -------------------------------------------------
        // set last active
        // -------------------------------------------------
        if(l_ups)
        {
                l_ups->m_last_active_ms = get_time_ms();
        }
        bool l_idle = false;
        // --------------------------------------------------
        // **************************************************
        // state machine
        // **************************************************
        // --------------------------------------------------
        //NDBG_PRINT("%sRUN_STATE_MACHINE%s: CONN[%p] STATE[%d] MODE: %d --START\n",
        //                ANSI_COLOR_BG_YELLOW, ANSI_COLOR_OFF, &l_nconn, l_nconn.get_state(), a_conn_mode);
state_top:
        //NDBG_PRINT("%sRUN_STATE_MACHINE%s: CONN[%p] STATE[%d] MODE: %d\n",
        //                ANSI_COLOR_FG_YELLOW, ANSI_COLOR_OFF, &l_nconn, l_nconn.get_state(), a_conn_mode);
        switch(l_nconn.get_state())
        {
        // -------------------------------------------------
        // STATE: FREE
        // -------------------------------------------------
        case nconn::NC_STATE_FREE:
        {
                int32_t l_s;
                l_s = l_nconn.ncsetup();
                if(l_s != nconn::NC_STATUS_OK)
                {
                        TRC_ERROR("performing ncsetup\n");
                        return STATUS_ERROR;
                }
                l_nconn.set_state(nconn::NC_STATE_CONNECTING);
                // stats
#if 0
                if(m_collect_stats_flag)
                {
                        m_connect_start_time_us = get_time_us();
                }
#endif
                goto state_top;
        }
        // -------------------------------------------------
        // STATE: CONNECTING
        // -------------------------------------------------
        case nconn::NC_STATE_CONNECTING:
        {
                //NDBG_PRINT("%sConnecting%s: host: %s\n",
                //              ANSI_COLOR_FG_RED, ANSI_COLOR_OFF,
                //              l_nconn.m_label.c_str());
                int32_t l_s;
                l_s = l_nconn.ncconnect();
                if(l_s == nconn::NC_STATUS_ERROR)
                {
                        TRC_ERROR("performing ncconnect for host: %s.\n", l_nconn.m_label.c_str());
                        return STATUS_ERROR;
                }
                if(l_nconn.is_connecting())
                {
                        //NDBG_PRINT("Still connecting...\n");
                        return STATUS_OK;
                }
                //NDBG_PRINT("%sConnected%s: label: %s\n", ANSI_COLOR_FG_RED, ANSI_COLOR_OFF, l_nconn.m_label.c_str());
                TRC_DEBUG("Connected: label: %s\n", l_nconn.m_label.c_str());
                // Returning client fd
                // If OK -change state to connected???
                l_nconn.set_state(nconn::NC_STATE_CONNECTED);
                // TODO for h2 -need connection callback to switch protocols
                // TODO -REMOVE!
                //l_s = show_tls_info(&l_nconn);
                //if(l_s != STATUS_OK)
                //{
                //        TRC_ERROR("performing show_tls_info\n");
                //        return STATUS_ERROR;
                //}
                //NDBG_PRINT("alpn: %d\n", l_nconn.get_alpn());;
#if 0
                if(l_nconn.m_connected_cb)
                {
                        l_s = l_nconn.m_connected_cb(this, m_data);
                        if(l_s != STATUS_OK)
                        {
                                //NDBG_PRINT("LABEL[%s]: Error performing m_read_cb\n", m_label.c_str());
                                return STATUS_ERROR;
                        }
                }
#endif
                // stats
#if 0
                if(m_collect_stats_flag)
                {
                        m_stat.m_tt_connect_us = get_delta_time_us(m_connect_start_time_us);
                }
#endif
                goto state_top;
        }
        // -------------------------------------------------
        // STATE: DONE
        // -------------------------------------------------
        case nconn::NC_STATE_DONE:
        {
                int32_t l_s;
                l_s = ups_session::teardown(l_ups, l_t_srvr, l_nconn, HTTP_STATUS_OK);
                // TODO -check status...
                UNUSED(l_s);
                return STATUS_DONE;
        }
        // -------------------------------------------------
        // STATE: CONNECTED
        // -------------------------------------------------
        case nconn::NC_STATE_CONNECTED:
        {
                switch(a_conn_mode)
                {
                // -----------------------------------------
                // read...
                // -----------------------------------------
                case EVR_MODE_READ:
                {
                        nbq *l_in_q = NULL;
                        if(l_ups)
                        {
                                l_in_q = l_ups->m_in_q;
                        }
                        else
                        {
                                // for reading junk disassociated from upstream session
                                l_in_q = l_t_srvr.m_orphan_in_q;
                                l_in_q->reset_write();
                        }
                        if(!l_in_q)
                        {
                                TRC_ERROR("l_in_q == NULL\n");
                                return STATUS_ERROR;
                        }
                        uint32_t l_read = 0;
                        int32_t l_s = nconn::NC_STATUS_OK;
                        char *l_buf = NULL;
                        uint64_t l_off = l_in_q->get_cur_write_offset();
                        l_s = l_nconn.nc_read(l_in_q, &l_buf, l_read);
                        // ---------------------------------
                        // handle error
                        // ---------------------------------
                        if(l_s != nconn::NC_STATUS_OK)
                        {
                        switch(l_s)
                        {
                        // ---------------------------------
                        // NC_STATUS_EOF
                        // ---------------------------------
                        case nconn::NC_STATUS_EOF:
                        {
                                if(l_ups)
                                {
                                // TODO FIX!!!
#if 0
                                        l_ups->subr_complete();
#endif
                                }
                                int32_t l_s;
                                l_s = ups_session::teardown(l_ups, l_t_srvr, l_nconn, HTTP_STATUS_OK);
                                // TODO -check status...
                                UNUSED(l_s);
                                return STATUS_DONE;
                        }
                        // ---------------------------------
                        // NC_STATUS_ERROR
                        // ---------------------------------
                        case nconn::NC_STATUS_ERROR:
                        {
                                int32_t l_s;
                                l_s = ups_session::teardown(l_ups, l_t_srvr, l_nconn, HTTP_STATUS_BAD_GATEWAY);
                                // TODO -check status...
                                UNUSED(l_s);
                                return STATUS_DONE;
                        }
                        // ---------------------------------
                        // READ_UNAVAILABLE
                        // ---------------------------------
                        case nconn::NC_STATUS_READ_UNAVAILABLE:
                        {
                                // -------------------------
                                // proxy back pressure
                                // -------------------------
                                if(l_ups)
                                {
                                        //TRC_DEBUG("set_again(true): l_ups: %p l_nconn: %p m_subr: %p path: %s\n",
                                        //                l_ups,
                                        //                l_nconn,
                                        //                l_ups->m_subr,
                                        //                l_ups->m_subr->get_path().c_str());
                                        l_ups->m_again = true;
                                        return STATUS_OK;
                                }
                                // TODO ???
                                break;
                        }
                        // ---------------------------------
                        // NC_STATUS_BREAK
                        // ---------------------------------
                        case nconn::NC_STATUS_BREAK:
                        {
                                return STATUS_OK;
                        }
                        // ---------------------------------
                        // NC_STATUS_AGAIN
                        // ---------------------------------
                        case nconn::NC_STATUS_AGAIN:
                        {
                                if(!l_idle)
                                {
                                        return STATUS_OK;
                                }
                                //NDBG_PRINT("add_idle\n");
                                l_nconn.set_data(NULL);
                                int32_t l_status;
                                l_status = l_t_srvr.m_nconn_proxy_pool.add_idle(&l_nconn);
                                if(l_status != STATUS_OK)
                                {
                                        TRC_ERROR("performing m_nconn_proxy_pool.add_idle(%p)\n", &l_nconn);
                                        return STATUS_ERROR;
                                }
                                return STATUS_OK;
                        }
                        // ---------------------------------
                        // default...
                        // ---------------------------------
                        default:
                        {
                                TRC_ERROR("unhandled connection state: %d\n", l_s);
                                int32_t l_s;
                                l_s = ups_session::teardown(l_ups, l_t_srvr, l_nconn, HTTP_STATUS_BAD_GATEWAY);
                                // TODO -check status...
                                UNUSED(l_s);
                                return STATUS_DONE;
                        }
                        }
                        }
                        // ---------------------------------
                        // stats
                        // ---------------------------------
#if 0
                        if(m_collect_stats_flag && (ao_read > 0))
                        {
                                m_stat.m_total_bytes += ao_read;
                                if(m_stat.m_tt_first_read_us == 0)
                                {
                                        m_stat.m_tt_first_read_us = get_delta_time_us(m_request_start_time_us);
                                }
                        }
#endif
                        // ---------------------------------
                        // parse
                        // ---------------------------------
                        if((l_read > 0) &&
                           l_ups &&
                           l_ups->m_resp &&
                           l_ups->m_resp->m_http_parser)
                        {
                                hmsg *l_hmsg = static_cast<hmsg *>(l_ups->m_resp);
                                size_t l_parse_status = 0;
                                //NDBG_PRINT("%sHTTP_PARSER%s: m_read_buf: %p, m_read_buf_idx: %d, l_bytes_read: %d\n",
                                //              ANSI_COLOR_BG_WHITE, ANSI_COLOR_OFF,
                                //              l_buf,
                                //              (int)l_off,
                                //              (int)l_read);
                                l_hmsg->m_cur_buf = l_in_q->b_write_data_ptr();
                                l_parse_status = http_parser_execute(l_hmsg->m_http_parser,
                                                                     l_hmsg->m_http_parser_settings,
                                                                     reinterpret_cast<const char *>(l_buf),
                                                                     l_read);
                                l_hmsg->m_cur_off = l_off;
                                //NDBG_PRINT("STATUS: %lu\n", l_parse_status);
                                if(l_parse_status < (size_t)l_read)
                                {
                                        TRC_ERROR("Parse error.  Reason: %s: %s\n",
                                                   http_errno_name((enum http_errno)l_hmsg->m_http_parser->http_errno),
                                                   http_errno_description((enum http_errno)l_hmsg->m_http_parser->http_errno));
                                        //TRC_VERBOSE("teardown status: %d\n", HTTP_STATUS_BAD_GATEWAY);
                                        TRC_ERROR("unhandled connection state: %d\n", l_s);
                                        int32_t l_s;
                                        l_s = ups_session::teardown(l_ups, l_t_srvr, l_nconn, HTTP_STATUS_BAD_GATEWAY);
                                        // TODO -check status...
                                        UNUSED(l_s);
                                        return STATUS_DONE;
                                }
                                //NDBG_PRINT("complete: %d\n", l_ups->m_resp->m_complete);
                        }
                        // ---------------------------------
                        // upstream read
                        // ---------------------------------
                        if(l_ups &&
                           l_ups->m_subr.m_u &&
                           (l_read > 0))
                        {
                                ssize_t l_size;
                                l_size = l_ups->m_subr.m_u->ups_read((size_t)l_read);
                                if(l_size < 0)
                                {
                                        TRC_ERROR("performing ups_read -a_conn_status: %d l_size: %zd\n", l_read, l_size);
                                        if(l_size == STATUS_ERROR)
                                        {
                                                //TRC_VERBOSE("teardown status: %d\n", HTTP_STATUS_BAD_GATEWAY);
                                                TRC_ERROR("unhandled connection state: %d\n", l_s);
                                                int32_t l_s;
                                                l_s = ups_session::teardown(l_ups, l_t_srvr, l_nconn, HTTP_STATUS_BAD_GATEWAY);
                                                // TODO -check status...
                                                UNUSED(l_s);
                                                return STATUS_DONE;
                                        }
                                }
                        }
                        // ---------------------------------
                        // handle completion
                        // ---------------------------------
                        if(l_ups &&
                           l_ups->m_resp &&
                           l_ups->m_resp->m_complete)
                        {
                                // -------------------------
                                // Cancel timer
                                // -------------------------
                                l_ups->cancel_timer(l_ups->m_evr_timeout);
                                // TODO Check status
                                l_ups->m_evr_timeout = NULL;
                                // -------------------------
                                // stats
                                // -------------------------
#if 0
                                // Get request time
                                if(l_nconn.get_collect_stats_flag())
                                {
                                        l_nconn.set_stat_tt_completion_us(get_delta_time_us(l_nconn.get_connect_start_time_us()));
                                }
                                if(l_ups->m_resp && l_t_srvr)
                                {
                                        l_t_srvr.add_stat_to_agg(l_nconn.get_stats(), l_ups->m_resp->get_status());
                                }
#endif
                                // -------------------------
                                // check can reuse
                                // -------------------------
                                bool l_hmsg_keep_alive = false;
                                if(l_ups->m_resp)
                                {
                                        l_hmsg_keep_alive = l_ups->m_resp->m_supports_keep_alives;
                                }
                                bool l_nconn_can_reuse = l_nconn.can_reuse();
                                // TODO FIX!!!
                                //bool l_keepalive = l_ups->m_keepalive;
                                bool l_keepalive = true;
                                bool l_detach_resp = l_ups->m_detach_resp;
                                // -------------------------
                                // complete request
                                // -------------------------
                                //subr_log_status(0);
                                //m_subr->set_end_time_ms(get_time_ms());
                                // Get vars -completion -can delete subr object
                                if(l_ups->m_subr.m_completion_cb)
                                {
                                        l_ups->m_subr.m_completion_cb(l_ups->m_subr, l_nconn, *(l_ups->m_resp));
                                }
                                if(l_detach_resp)
                                {
                                        l_ups->m_resp = NULL;
                                        l_ups->m_in_q = NULL;
                                }
                                // set state to done
                                l_ups->m_subr.m_state = subr::SUBR_STATE_NONE;;
                                if(!l_nconn_can_reuse ||
                                   !l_keepalive ||
                                   !l_hmsg_keep_alive)
                                {
                                        TRC_VERBOSE("marking conn EOF: l_nconn_can_reuse: %d, l_keepalive: %d, l_hmsg_keep_alive: %d\n",
                                                    l_nconn_can_reuse,
                                                    l_keepalive,
                                                    l_hmsg_keep_alive);
                                        l_nconn.set_state_done();
                                        goto state_top;
                                }
                                // Give back rqst + in q
                                if(l_ups->m_out_q)
                                {
                                        delete l_ups->m_out_q;
                                        l_ups->m_out_q = NULL;
                                }
                                if(!l_detach_resp)
                                {
                                        if(l_ups->m_resp)
                                        {
                                                delete l_ups->m_resp;
                                                l_ups->m_resp = NULL;
                                        }
                                        if(!l_ups->m_in_q_detached)
                                        {
                                                delete l_ups->m_in_q;
                                                l_ups->m_in_q = NULL;
                                        }
                                }
                                // -------------------------
                                // set idle
                                // -------------------------
                                l_idle = true;
                                goto state_top;
                        }
                        goto state_top;
                }
                // -----------------------------------------
                // write...
                // -----------------------------------------
                case EVR_MODE_WRITE:
                {
                        nbq *l_out_q = NULL;
                        if(l_ups)
                        {
                                l_out_q = l_ups->m_out_q;
                        }
                        else
                        {
                                // for reading junk disassociated from upstream session
                                TRC_WARN("l_out_q == t_srvr orphan out q\n");
                                l_out_q = l_t_srvr.m_orphan_out_q;
                                l_out_q->reset_write();
                        }
                        if(!l_out_q)
                        {
                                TRC_ERROR("l_out_q == NULL\n");
                                return STATUS_ERROR;
                        }
                        // ---------------------------------
                        // write
                        // ---------------------------------
                        uint32_t l_written = 0;
                        int32_t l_s = nconn::NC_STATUS_OK;
                        l_s = l_nconn.nc_write(l_out_q, l_written);
                        // ---------------------------------
                        // handle error
                        // ---------------------------------
                        if(l_s != nconn::NC_STATUS_OK)
                        {
                        switch(l_s)
                        {
                        // ---------------------------------
                        // NC_STATUS_EOF
                        // ---------------------------------
                        case nconn::NC_STATUS_EOF:
                        {
                                l_nconn.set_state_done();
                                goto state_top;
                        }
                        // ---------------------------------
                        // NC_STATUS_ERROR
                        // ---------------------------------
                        case nconn::NC_STATUS_ERROR:
                        {
                                //++(l_t_srvr.m_stat.m_clnt_errors);
                                l_nconn.set_state_done();
                                goto state_top;
                        }
                        // ---------------------------------
                        // NC_STATUS_BREAK
                        // ---------------------------------
                        case nconn::NC_STATUS_BREAK:
                        {
                                return STATUS_OK;
                        }
                        // ---------------------------------
                        // NC_STATUS_AGAIN
                        // ---------------------------------
                        case nconn::NC_STATUS_AGAIN:
                        {
                                if(!l_idle)
                                {
                                        return STATUS_OK;
                                }
                                l_nconn.set_data(NULL);
                                int32_t l_status;
                                l_status = l_t_srvr.m_nconn_proxy_pool.add_idle(&l_nconn);
                                if(l_status != STATUS_OK)
                                {
                                        TRC_ERROR("performing m_nconn_proxy_pool.add_idle(%p)\n", &l_nconn);
                                        return STATUS_ERROR;
                                }
                                return STATUS_OK;
                        }
                        // ---------------------------------
                        // default...
                        // ---------------------------------
                        default:
                        {
                                TRC_ERROR("unhandled connection state: %d\n", l_s);
                                return STATUS_ERROR;
                        }
                        }
                        }
                        // stats
                        //l_t_srvr.m_stat.m_upsv_bytes_read += l_read;
                        //l_t_srvr.m_stat.m_upsv_bytes_written += l_written;
                        if(l_out_q->read_avail())
                        {
                                goto state_top;
                        }
                        return STATUS_OK;
                }
                // -----------------------------------------
                // TODO
                // -----------------------------------------
                default:
                {
                        return STATUS_ERROR;
                }
                }
                return STATUS_ERROR;
        }
        // -------------------------------------------------
        // default
        // -------------------------------------------------
        default:
        {
                //NDBG_PRINT("default\n");
                TRC_ERROR("unexpected conn state %d\n", l_nconn.get_state());
                return STATUS_ERROR;
        }
        }
        return STATUS_OK;
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
int32_t ups_session::evr_fd_readable_cb(void *a_data)
{
        return run_state_machine(a_data, EVR_MODE_READ);
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
int32_t ups_session::evr_fd_writeable_cb(void *a_data)
{
        return run_state_machine(a_data, EVR_MODE_WRITE);
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
int32_t ups_session::evr_fd_error_cb(void *a_data)
{
        return run_state_machine(a_data, EVR_MODE_ERROR);
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
int32_t ups_session::evr_event_timeout_cb(void *a_data)
{
        // -------------------------------------------------
        // clear event
        // -------------------------------------------------
        CHECK_FOR_NULL_ERROR(a_data);
        nconn* l_nconn = static_cast<nconn*>(a_data);
        ups_session *l_ups = static_cast<ups_session *>(l_nconn->get_data());
        if(l_ups &&
           l_ups->m_evr_timeout)
        {
                l_ups->m_evr_timeout = NULL;
        }
        return run_state_machine(a_data, EVR_MODE_TIMEOUT);
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
int32_t ups_session::evr_event_readable_cb(void *a_data)
{
        // -------------------------------------------------
        // clear event
        // -------------------------------------------------
        CHECK_FOR_NULL_ERROR(a_data);
        nconn* l_nconn = static_cast<nconn*>(a_data);
        ups_session *l_ups = static_cast<ups_session *>(l_nconn->get_data());
        if(l_ups &&
           l_ups->m_evr_readable)
        {
                l_ups->m_evr_readable = NULL;
        }
        return run_state_machine(a_data, EVR_MODE_READ);
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
#if 0
int32_t ups_session::evr_event_writeable_cb(void *a_data)
{
        // -------------------------------------------------
        // clear event
        // -------------------------------------------------
        CHECK_FOR_NULL_ERROR(a_data);
        nconn* l_nconn = static_cast<nconn*>(a_data);
        ups_session *l_ups = static_cast<ups_session *>(l_nconn->get_data());
        if(l_ups &&
           l_ups->m_evr_writeable)
        {
                l_ups->m_evr_writeable = NULL;
        }
        return run_state_machine(a_data, EVR_MODE_WRITE);
}
#endif
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
void ups_session::subr_enqueue(subr &a_subr)
{
        // attach to upstream object if missing
        if(!a_subr.m_session.m_u)
        {
                subr_u *l_subr_u = new subr_u(a_subr.m_session);
                l_subr_u->queue(a_subr);
                a_subr.m_session.m_u = l_subr_u;
        }
        a_subr.m_state = subr::SUBR_STATE_QUEUED;
        t_srvr &l_t_srvr = a_subr.m_session.m_t_srvr;
        l_t_srvr.m_subr_list.push_back(&a_subr);
        a_subr.m_i_q = --(l_t_srvr.m_subr_list.end());
        ++l_t_srvr.m_subr_list_size;
        evr_event *l_event;
        l_t_srvr.queue_event(&l_event, subr_dequeue, &(l_t_srvr));
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
int32_t ups_session::subr_dequeue(void *a_data)
{
        // TODO FIX!!!
        if(!a_data)
        {
                // TODO -log error???
                return STATUS_ERROR;
        }
        t_srvr &l_t_srvr = *(static_cast <t_srvr *>(a_data));
        //NDBG_PRINT("%sSTART%s\n", ANSI_COLOR_BG_GREEN, ANSI_COLOR_OFF);
        while(l_t_srvr.m_subr_list_size &&
              !l_t_srvr.get_stopped())
        {
                // -----------------------------------------
                // dequeue
                // -----------------------------------------
                if(!l_t_srvr.m_subr_list.front())
                {
                        l_t_srvr.m_subr_list.pop_front();
                        --l_t_srvr.m_subr_list_size;
                        continue;
                }
                subr &l_subr = *(l_t_srvr.m_subr_list.front());
                l_subr.m_state = subr::SUBR_STATE_NONE;
                l_subr.m_i_q = l_t_srvr.m_subr_list.end();
                l_t_srvr.m_subr_list.pop_front();
                --l_t_srvr.m_subr_list_size;
                int32_t l_s = STATUS_OK;
                l_s = subr_start(l_subr);
                if(l_s == STATUS_OK)
                {
                        l_subr.m_state = subr::SUBR_STATE_ACTIVE;
                }
                else if(l_s == STATUS_AGAIN)
                {
                        // break since ran out of available connections
                        subr_enqueue(l_subr);
                        break;
                }
#ifdef ASYNC_DNS_SUPPORT
                else if(l_s == STATUS_QUEUED_ASYNC_DNS)
                {
                        l_subr.m_state = subr::SUBR_STATE_DNS_LOOKUP;
                }
#endif
        }
        return STATUS_OK;
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
int32_t ups_session::subr_start(subr &a_subr)
{
        int32_t l_s;
        std::string l_error;
        // -------------------------------------------------
        // try get idle from proxy pool
        // -------------------------------------------------
        t_srvr &l_t_srvr = a_subr.m_session.m_t_srvr;
        const t_conf &l_t_conf = *(l_t_srvr.m_t_conf);
        nconn_pool &l_proxy_pool = l_t_srvr.m_nconn_proxy_pool;
        nconn *l_nconn = NULL;
        l_nconn = l_proxy_pool.get_idle(a_subr.get_label());
        //NDBG_PRINT("l_nconn: %p\n", l_nconn);
        if(!l_nconn)
        {
                //NDBG_PRINT("l_nconn: %p\n", l_nconn);
                // Check for available active connections
                // If we maxed out -try again later...
                if(!l_proxy_pool.get_active_available())
                {
                        return STATUS_AGAIN;
                }
                nresolver *l_nresolver = l_t_conf.m_nresolver;
                if(!l_nresolver)
                {
                        //NDBG_PRINT("Error no resolver\n");
                        return STATUS_ERROR;
                }
                // Try fast
                host_info l_host_info;
                l_s = l_nresolver->lookup_tryfast(a_subr.m_host,
                                                  a_subr.m_port,
                                                  l_host_info);
                if(l_s != STATUS_OK)
                {
#ifdef ASYNC_DNS_SUPPORT
                        if(!l_t_conf.m_dns_use_sync)
                        {
                                //m_stat.m_subr_pending_resolv_map[a_subr.get_label()] = &a_subr;
                                //++(m_stat.m_dns_resolve_req);
                                //NDBG_PRINT("%sl_subr label%s: %s --HOST: %s\n", ANSI_COLOR_BG_RED, ANSI_COLOR_OFF, a_subr.get_label().c_str(), a_subr.get_host().c_str());
                                void *l_job_handle = NULL;
                                l_s = l_nresolver->lookup_async(l_t_srvr.m_adns_ctx,
                                                                a_subr.m_host,
                                                                a_subr.m_port,
                                                                &a_subr,
                                                                &l_job_handle);
                                if(l_s != STATUS_OK)
                                {
                                        return STATUS_ERROR;
                                }
                                if(l_job_handle)
                                {
                                        a_subr.m_lookup_job = l_job_handle;
                                }
                                return STATUS_QUEUED_ASYNC_DNS;
                        }
                        else
                        {
#endif
                        // sync dns
                        l_s = l_nresolver->lookup_sync(a_subr.m_host, a_subr.m_port, l_host_info);
                        if(l_s != STATUS_OK)
                        {
                                //NDBG_PRINT("Error: performing lookup_sync\n");
                                //++m_stat.m_upsv_errors;
                                if(a_subr.m_error_cb)
                                {
                                        a_subr.m_error_cb(a_subr, NULL, HTTP_STATUS_BAD_GATEWAY, "address lookup failure");
                                        // TODO check status
                                }
                                return STATUS_ERROR;
                        }
                        else
                        {
                                //++(m_stat.m_dns_resolved);
                        }
#ifdef ASYNC_DNS_SUPPORT
                        }
#endif
                }
                // -----------------------------------------
                // connection setup
                // -----------------------------------------
                l_nconn = l_proxy_pool.get_new_active(a_subr.get_label(), a_subr.m_scheme);
                if(!l_nconn)
                {
                        //NDBG_PRINT("Returning NULL\n");
                        return STATUS_AGAIN;
                }
                l_nconn->set_ctx(&l_t_srvr);
                l_nconn->set_num_reqs_per_conn(l_t_conf.m_num_reqs_per_conn);
                //l_nconn->set_collect_stats(l_t_conf.m_collect_stats);
                l_nconn->setup_evr_fd(ups_session::evr_fd_readable_cb,
                                      ups_session::evr_fd_writeable_cb,
                                      ups_session::evr_fd_error_cb);
                if(l_nconn->get_scheme() == SCHEME_TLS)
                {
                        _SET_NCONN_OPT((*l_nconn),nconn_tls::OPT_TLS_CIPHER_STR,l_t_conf.m_tls_client_ctx_cipher_list.c_str(),l_t_conf.m_tls_client_ctx_cipher_list.length());
                        _SET_NCONN_OPT((*l_nconn),nconn_tls::OPT_TLS_CTX,l_t_conf.m_tls_client_ctx,sizeof(l_t_conf.m_tls_client_ctx));
                        _SET_NCONN_OPT((*l_nconn), nconn_tls::OPT_TLS_VERIFY, &(a_subr.m_tls_verify), sizeof(bool));
                        _SET_NCONN_OPT((*l_nconn), nconn_tls::OPT_TLS_VERIFY_ALLOW_SELF_SIGNED, &(a_subr.m_tls_self_ok), sizeof(bool));
                        _SET_NCONN_OPT((*l_nconn), nconn_tls::OPT_TLS_VERIFY_NO_HOST_CHECK, &(a_subr.m_tls_no_host_check), sizeof(bool));
                        _SET_NCONN_OPT((*l_nconn), nconn_tls::OPT_TLS_SNI, &(a_subr.m_tls_sni), sizeof(bool));
                        if(!a_subr.m_hostname.empty())
                        {
                                _SET_NCONN_OPT((*l_nconn), nconn_tls::OPT_TLS_HOSTNAME, a_subr.m_hostname.c_str(), a_subr.m_hostname.length());
                        }
                        else
                        {
                                _SET_NCONN_OPT((*l_nconn), nconn_tls::OPT_TLS_HOSTNAME, a_subr.m_host.c_str(), a_subr.m_host.length());
                        }
                }
                l_nconn->set_host_info(l_host_info);
                a_subr.m_host_info = l_host_info;
                // Reset stats
                //l_nconn->reset_stats();
                // stats
                //++m_stat.m_upsv_conn_started;
                //m_stat.m_pool_proxy_conn_active = m_nconn_proxy_pool.get_active_size();
        }
        // -------------------------------------------------
        // If we grabbed an idle connection spoof connect
        // time for stats
        // -------------------------------------------------
        else
        {
                // Reset stats
                //l_nconn->reset_stats();
                //if(l_nconn->get_collect_stats_flag())
                //{
                //        l_nconn->set_connect_start_time_us(get_time_us());
                //}
        }
        // -------------------------------------------------
        // proxy u
        // -------------------------------------------------
        ups_session *l_ups = new ups_session(a_subr);
        l_ups->m_evr_timeout = NULL;
        l_ups->m_nconn = l_nconn;
        a_subr.m_ups_session = l_ups;
        l_nconn->set_data(l_ups);
        l_nconn->set_evr_loop(l_t_srvr.get_evr_loop());
        // -------------------------------------------------
        // resp
        // -------------------------------------------------
        l_ups->m_resp = new resp();
        l_ups->m_resp->init(a_subr.m_save);
        l_ups->m_resp->m_http_parser->data = l_ups->m_resp;
        l_ups->m_resp = l_ups->m_resp;
        l_ups->m_resp->m_expect_resp_body_flag = a_subr.get_expect_resp_body_flag();
        // ---------------------------------------
        // in q
        // ---------------------------------------
        if(a_subr.m_u &&
           (a_subr.m_u->ups_get_type() == proxy_u::S_UPS_TYPE_PROXY) &&
           (a_subr.m_session.m_out_q))
        {
                l_ups->m_in_q = a_subr.m_session.m_out_q;
                l_ups->m_in_q_detached = true;
        }
        else
        {
                l_ups->m_in_q = a_subr.m_session.m_t_srvr.get_nbq(NULL);
        }
        l_ups->m_resp->set_q(l_ups->m_in_q);
        // ---------------------------------------
        // out q
        // ---------------------------------------
        if(!l_ups->m_out_q)
        {
                l_ups->m_out_q = l_t_srvr.get_nbq(NULL);
        }
        else
        {
                l_ups->m_out_q->reset_read();
        }
        // ---------------------------------------
        // create request
        // ---------------------------------------
        l_s = a_subr.create_request(*(l_ups->m_out_q));
        if(STATUS_OK != l_s)
        {
                return ups_session::evr_fd_error_cb(l_nconn);
        }
        // ---------------------------------------
        // stats
        // ---------------------------------------
        //++m_stat.m_upsv_reqs;
        //a_subr.set_start_time_ms(get_time_ms());
        //if(l_nconn->get_collect_stats_flag())
        //{
        //        l_nconn->set_request_start_time_us(get_time_us());
        //}
        //l_ups->set_last_active_ms(get_time_ms());
        //l_ups->set_timeout_ms(a_subr.get_timeout_ms());
        // ---------------------------------------
        // idle timer
        // ---------------------------------------
        l_s = l_t_srvr.add_timer(l_ups->m_timeout_ms,
                                 ups_session::evr_event_timeout_cb,
                                 l_nconn,
                                 (void **)&(l_ups->m_evr_timeout));
        if(l_s != STATUS_OK)
        {
                return ups_session::evr_fd_error_cb(l_nconn);
        }
        // ---------------------------------------
        // start writing request
        // ---------------------------------------
        return ups_session::evr_fd_writeable_cb(l_nconn);
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
#if 0
void ups_session::subr_log_status(uint16_t a_status)
{
        if(!m_t_srvr)
        {
                return;
        }
        ++(m_t_srvr->m_stat.m_upsv_resp);
        uint16_t l_status;
        if(m_resp)
        {
                l_status = m_resp->get_status();
        }
        else if(a_status)
        {
                l_status = a_status;
        }
        else
        {
                l_status = HTTP_STATUS_INTERNAL_SERVER_ERROR;
        }
        if((l_status >= 100) && (l_status < 200)) {/* TODO log 1xx's? */}
        else if((l_status >= 200) && (l_status < 300)){++m_t_srvr->m_stat.m_upsv_resp_status_2xx;}
        else if((l_status >= 300) && (l_status < 400)){++m_t_srvr->m_stat.m_upsv_resp_status_3xx;}
        else if((l_status >= 400) && (l_status < 500)){++m_t_srvr->m_stat.m_upsv_resp_status_4xx;}
        else if((l_status >= 500) && (l_status < 600)){++m_t_srvr->m_stat.m_upsv_resp_status_5xx;}
}
#endif
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
int32_t ups_session::cancel_evr_timer(void)
{
        if(!m_evr_timeout)
        {
                return STATUS_OK;
        }
        int32_t l_status;
        l_status = m_subr.m_session.m_t_srvr.cancel_event(m_evr_timeout);
        if(l_status != STATUS_OK)
        {
                return STATUS_ERROR;
        }
        m_evr_timeout = NULL;
        return STATUS_OK;
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
int32_t ups_session::cancel_evr_readable(void)
{
        if(!m_evr_readable)
        {
                return STATUS_OK;
        }
        int32_t l_status;
        l_status = m_subr.m_session.m_t_srvr.cancel_event(m_evr_readable);
        if(l_status != STATUS_OK)
        {
                return STATUS_ERROR;
        }
        m_evr_readable = NULL;
        return STATUS_OK;
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
int32_t ups_session::cancel_evr_writeable(void)
{
        if(!m_evr_writeable)
        {
                return STATUS_OK;
        }
        int32_t l_status;
        l_status = m_subr.m_session.m_t_srvr.cancel_event(m_evr_writeable);
        if(l_status != STATUS_OK)
        {
                return STATUS_ERROR;
        }
        m_evr_writeable = NULL;
        return STATUS_OK;
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
int32_t ups_session::cancel_timer(void *a_timer)
{
        int32_t l_s;
        l_s = m_subr.m_session.m_t_srvr.cancel_event(a_timer);
        if(l_s != STATUS_OK)
        {
                return STATUS_ERROR;
        }
        return STATUS_OK;
}
} //namespace ns_is2 {
