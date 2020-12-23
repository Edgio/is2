//! ----------------------------------------------------------------------------
//! Copyright Verizon.
//!
//! \file:    TODO
//! \details: TODO
//!
//! Licensed under the terms of the Apache 2.0 open source license.
//! Please refer to the LICENSE file in the project root for the terms.
//! ----------------------------------------------------------------------------
//! ----------------------------------------------------------------------------
//! includes
//! ----------------------------------------------------------------------------
#include "srvr/t_srvr.h"
#include "srvr/ups_session.h"
#include "is2/support/ndebug.h"
#include "http_parser/http_parser.h"
#include "is2/nconn/nconn.h"
#include "is2/support/nbq.h"
#include "is2/support/trace.h"
#include "is2/status.h"
#include "is2/evr/evr.h"
#include "is2/srvr/rqst.h"
#include "is2/srvr/base_u.h"
#include "is2/srvr/lsnr.h"
#include "is2/srvr/subr.h"
#include "is2/srvr/session.h"
#include "is2/srvr/api_resp.h"
//! ----------------------------------------------------------------------------
//! macros
//! ----------------------------------------------------------------------------
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

namespace ns_is2 {
//! ----------------------------------------------------------------------------
//! Static
//! ----------------------------------------------------------------------------
default_rqst_h session::s_default_rqst_h;
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
session::session(t_srvr &a_t_srvr):
        m_nconn(NULL),
        m_t_srvr(a_t_srvr),
        m_evr_timeout(NULL),
        m_evr_readable(NULL),
        m_evr_writeable(NULL),
        m_rqst(NULL),
        m_lsnr(NULL),
        m_in_q(NULL),
        m_out_q(NULL),
        m_idx(0),
        m_u(NULL),
        m_access_info(),
        m_resp_done_cb(NULL),
        m_last_active_ms(0),
        m_timeout_ms(10000)
{}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
session::~session(void)
{
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
        if(m_rqst)
        {
                delete m_rqst;
                m_rqst = NULL;
        }
        if(m_in_q)
        {
                delete m_in_q;
                m_in_q = NULL;
        }
        if(m_out_q)
        {
                delete m_out_q;
                m_out_q = NULL;
        }
        if(m_u)
        {
                if(!m_u->ups_done())
                {
                        m_u->ups_cancel();
                }
                delete m_u;
                m_u = NULL;
        }
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
nbq *session::get_nbq(void)
{
        return m_t_srvr.get_nbq(NULL);
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t session::add_evr_timer(uint32_t a_ms)
{
        int32_t l_s;
        l_s = m_t_srvr.add_timer(a_ms,
                                  session::evr_event_timeout_cb,
                                  m_nconn,
                                  (void **)(&(m_evr_timeout)));
        return l_s;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t session::cancel_evr_timer(void)
{
        if(!m_evr_timeout)
        {
                return STATUS_OK;
        }
        int32_t l_status;
        l_status = m_t_srvr.cancel_event(m_evr_timeout);
        if(l_status != STATUS_OK)
        {
                return STATUS_ERROR;
        }
        m_evr_timeout = NULL;
        return STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t session::cancel_evr_readable(void)
{
        if(!m_evr_readable)
        {
                return STATUS_OK;
        }
        int32_t l_status;
        l_status = m_t_srvr.cancel_event(m_evr_readable);
        if(l_status != STATUS_OK)
        {
                return STATUS_ERROR;
        }
        m_evr_readable = NULL;
        return STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t session::cancel_evr_writeable(void)
{
        if(!m_evr_writeable)
        {
                return STATUS_OK;
        }
        int32_t l_status;
        l_status = m_t_srvr.cancel_event(m_evr_writeable);
        if(l_status != STATUS_OK)
        {
                return STATUS_ERROR;
        }
        m_evr_writeable = NULL;
        return STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
uint32_t session::get_timeout_ms(void)
{
        return m_timeout_ms;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
host_info session::get_host_info(void)
{
        if(m_nconn)
        {
                return m_nconn->get_host_info();
        }
        host_info l_host_info_tmp;
        return l_host_info_tmp;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
scheme_t session::get_scheme(void)
{
        if(m_nconn)
        {
                return m_nconn->get_scheme();
        }
        return SCHEME_TCP;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
void session::set_timeout_ms(uint32_t a_t_ms)
{
        m_timeout_ms = a_t_ms;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
uint64_t session::get_last_active_ms(void)
{
        return m_last_active_ms;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
void session::set_last_active_ms(uint64_t a_time_ms)
{
        m_last_active_ms = a_time_ms;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t session::teardown(void)
{
        //NDBG_PRINT("%sTEARDOWN%s: this: %p m_nconn: %p m_rqst: %p\n",
        //           ANSI_COLOR_BG_RED, ANSI_COLOR_OFF,
        //           this, m_nconn, m_rqst);
        // -------------------------------------------------
        // cancel timer
        // -------------------------------------------------
        int32_t l_s;
        l_s = cancel_evr_timer();
        // TODO Check status
        UNUSED(l_s);
        // if upstream object associated w/ clnt request...
        if(m_u)
        {
                if(!m_u->ups_done())
                {
                        l_s = m_u->ups_cancel();
                        if(l_s != STATUS_OK)
                        {
                                TRC_ERROR("performing ups_cancel\n");
                        }
                }
                delete m_u;
                m_u = NULL;
        }
        // -------------------------------------------------
        // disassociate connection
        // -------------------------------------------------
        if(m_nconn)
        {
                m_nconn->set_ctx(NULL);
                m_nconn->set_data(NULL);
                m_nconn->nccleanup();
                delete m_nconn;
                m_nconn = NULL;
        }
        return STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
static int32_t run_state_machine(void *a_data, evr_mode_t a_conn_mode)
{
        //NDBG_PRINT("RUN a_conn_mode: %d a_data: %p\n", a_conn_mode, a_data);
        CHECK_FOR_NULL_ERROR(a_data);
        nconn* l_nconn = static_cast<nconn*>(a_data);
        CHECK_FOR_NULL_ERROR(l_nconn->get_ctx());
        t_srvr *l_t_srvr = static_cast<t_srvr *>(l_nconn->get_ctx());
        session *l_cs = static_cast<session *>(l_nconn->get_data());
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
                // ignore callbacks for free connections
                if(l_nconn->is_free())
                {
                        TRC_ERROR("call back for free connection\n");
                        return STATUS_DONE;
                }
                //if(l_t_srvr) { ++(l_t_srvr->m_stat.m_clnt_errors); }
                if(l_cs)
                {
                        int32_t l_s;
                        l_s = l_cs->teardown();
                        // TODO check status
                        UNUSED(l_s);
                        delete l_cs;
                        l_cs = NULL;
                        return STATUS_DONE;
                }
                TRC_ERROR("a_conn_mode[%d] session == NULL\n", a_conn_mode);
                return STATUS_OK;
        }
        // -------------------------------------------------
        // TIMEOUT
        // -------------------------------------------------
        case EVR_MODE_TIMEOUT:
        {
                // ignore callbacks for free connections
                if(l_nconn->is_free())
                {
                        TRC_ERROR("call back for free connection\n");
                        return STATUS_OK;
                }
                // calc time since last active
                if(!l_cs)
                {
                        TRC_ERROR("a_conn_mode[%d] session[%p] == NULL\n",
                                        a_conn_mode,
                                        l_cs);
                        return STATUS_OK;
                }
                // -----------------------------------------
                // timeout
                // -----------------------------------------
                uint64_t l_ct_ms = get_time_ms();
                if(((uint32_t)(l_ct_ms - l_cs->get_last_active_ms())) >= l_cs->get_timeout_ms())
                {
                        //++(l_t_srvr->m_stat.m_clnt_idle_killed);
                        //++(l_t_srvr->m_stat.m_clnt_errors);
                        int32_t l_s;
                        l_s = l_cs->teardown();
                        // TODO check status
                        UNUSED(l_s);
                        delete l_cs;
                        l_cs = NULL;
                        return STATUS_DONE;
                }
                // -----------------------------------------
                // active -create new timer with
                // delta time
                // -----------------------------------------
                else if(l_cs)
                {
                        uint64_t l_d_time = (uint32_t)(l_cs->get_timeout_ms() - (l_ct_ms - l_cs->get_last_active_ms()));
                        int32_t l_s;
                        l_s = l_cs->add_evr_timer(l_d_time);
                        // TODO check status
                        UNUSED(l_s);
                        return STATUS_OK;
                }
        }
        // -------------------------------------------------
        // EVR_MODE_READ
        // -------------------------------------------------
        case EVR_MODE_READ:
        {
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
        if(l_cs)
        {
                l_cs->set_last_active_ms(get_time_ms());
        }
        // --------------------------------------------------
        // **************************************************
        // state machine
        // **************************************************
        // --------------------------------------------------
        //NDBG_PRINT("%sRUN_STATE_MACHINE%s: CONN[%p] STATE[%d] MODE: %d --START\n",
        //            ANSI_COLOR_BG_YELLOW, ANSI_COLOR_OFF,
        //            l_nconn, l_nconn->get_state(), a_conn_mode);
state_top:
        // -------------------------------------------------
        // check srvr state
        // -------------------------------------------------
        if(l_t_srvr &&
           !l_t_srvr->is_running())
        {
                return STATUS_OK;
        }
        //NDBG_PRINT("%sRUN_STATE_MACHINE%s: CONN[%p] STATE[%d] MODE: %d\n",
        //            ANSI_COLOR_FG_YELLOW, ANSI_COLOR_OFF,
        //            l_nconn, l_nconn->get_state(), a_conn_mode);
        switch(l_nconn->get_state())
        {
        // -------------------------------------------------
        // STATE: FREE
        // -------------------------------------------------
        case nconn::NC_STATE_DONE:
        {
                if(!l_cs)
                {
                        return STATUS_DONE;
                }
                int32_t l_s;
                l_s = l_cs->teardown();
                // TODO check status
                UNUSED(l_s);
                delete l_cs;
                l_cs = NULL;
                return STATUS_DONE;
        }
        // -------------------------------------------------
        // STATE: FREE
        // -------------------------------------------------
        case nconn::NC_STATE_FREE:
        {
                // ignore callbacks for free connections
                TRC_ERROR("call back for free connection\n");
                return STATUS_OK;
        }
        // -------------------------------------------------
        // STATE: ACCEPTING
        // -------------------------------------------------
        case nconn::NC_STATE_ACCEPTING:
        {
                // -----------------------------------------
                // accept
                // -----------------------------------------
                int32_t l_s;
                l_s = l_nconn->ncaccept();
                if(l_s == nconn::NC_STATUS_ERROR)
                {
                        TRC_ERROR("performing ncaccept\n");
                        return STATUS_ERROR;
                }
                // -----------------------------------------
                // check state
                // -----------------------------------------
                if(l_nconn->is_accepting())
                {
                        //NDBG_PRINT("Still connecting...\n");
                        return STATUS_OK;
                }
                l_nconn->set_state(nconn::NC_STATE_CONNECTED);
                goto state_top;
        }
        // -------------------------------------------------
        // STATE: CONNECTED
        // -------------------------------------------------
        case nconn::NC_STATE_CONNECTED:
        {
                // TODO  use sessions for h2
                //session *l_ses = static_cast<session *>(l_nconn->get_data());
                switch(a_conn_mode)
                {
                // -----------------------------------------
                // read...
                // -----------------------------------------
                case EVR_MODE_READ:
                {
                        // TODO get from session
                        nbq *l_in_q = NULL;
                        if(l_cs)
                        {
                                l_in_q = l_cs->m_in_q;
                        }
                        else if(l_t_srvr)
                        {
                                l_in_q = l_t_srvr->m_orphan_in_q;
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
                        l_s = l_nconn->nc_read(l_in_q, &l_buf, l_read);
                        if(l_t_srvr) { l_t_srvr->m_stat.m_bytes_read += l_read; }
                        if(l_cs) {l_cs->m_access_info.m_bytes_in += l_read; }
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
                                l_nconn->set_state_done();
                                goto state_top;
                        }
                        // ---------------------------------
                        // NC_STATUS_ERROR
                        // ---------------------------------
                        case nconn::NC_STATUS_ERROR:
                        {
                                //if(l_t_srvr) { ++(l_t_srvr->m_stat.m_clnt_errors);}
                                l_nconn->set_state_done();
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
                                return STATUS_OK;
                        }
                        // ---------------------------------
                        // NC_STATUS_READ_UNAVAILABLE
                        // ---------------------------------
                        case nconn::NC_STATUS_READ_UNAVAILABLE:
                        {
                                // TODO TRACE
                                TRC_ERROR("performing nc_read\n");
                                return STATUS_ERROR;
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
                        // read data...
                        // ---------------------------------
                        if((l_read > 0) &&
                           l_cs &&
                           l_cs->m_rqst &&
                           l_cs->m_rqst->m_http_parser)
                        {
                                hmsg *l_hmsg = static_cast<hmsg *>(l_cs->m_rqst);
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
                                if((l_parse_status < (size_t)l_read))
                                {
                                        TRC_ERROR("http parse error[%d]: %s: %s\n",
                                                   l_hmsg->m_http_parser->http_errno,
                                                   http_errno_name((enum http_errno)l_hmsg->m_http_parser->http_errno),
                                                   http_errno_description((enum http_errno)l_hmsg->m_http_parser->http_errno));
                                        // -----------------
                                        // send error resp
                                        // -----------------
                                        h_resp_t l_hdlr_status = H_RESP_NONE;
                                        l_hdlr_status = l_cs->s_default_rqst_h.send_bad_request(*l_cs, l_hmsg->m_supports_keep_alives);
                                        UNUSED(l_hdlr_status);
                                        return STATUS_OK;
                                }
                        }
                        // ---------------------------------
                        // send expect response -if signal
                        // ---------------------------------
                        if(l_cs &&
                           l_cs->m_rqst &&
                           l_cs->m_rqst->m_expect)
                        {
                                nbq l_nbq(64);
                                const char l_exp_reply[] = "HTTP/1.1 100 Continue\r\n\r\n";
                                l_nbq.write(l_exp_reply, strlen(l_exp_reply));
                                uint32_t l_w;
                                l_nconn->nc_write(&l_nbq, l_w);
                                if(l_t_srvr) { l_t_srvr->m_stat.m_bytes_written += strlen(l_exp_reply);}
                                l_cs->m_access_info.m_bytes_out += l_w;
                                l_cs->m_rqst->m_expect = false;
                        }
                        // ---------------------------------
                        // rqst complete
                        // ---------------------------------
                        if(l_cs &&
                           l_cs->m_rqst &&
                           l_cs->m_rqst->m_complete)
                        {
                                if(l_t_srvr) { ++l_t_srvr->m_stat.m_reqs; }
                                int32_t l_rs = STATUS_OK;
                                l_rs = l_cs->handle_req();
                                if(l_rs != STATUS_OK)
                                {
                                        TRC_ERROR("performing handle_req\n");
                                        l_nconn->set_state_done();
                                        goto state_top;
                                }
                                bool l_ka = l_cs->m_rqst->m_supports_keep_alives;
                                l_cs->m_rqst->init();
                                l_cs->m_rqst->m_supports_keep_alives = l_ka;
                                if(l_cs->m_in_q)
                                {
                                        l_cs->m_in_q->reset_write();
                                }
                        }
                        goto state_top;
                }
                // -----------------------------------------
                // write...
                // -----------------------------------------
                case EVR_MODE_WRITE:
                {
                        // ---------------------------------
                        // out q
                        // ---------------------------------
                        nbq *l_out_q = NULL;
                        if(l_cs)
                        {
                                if(!l_cs->m_out_q &&
                                   l_t_srvr)
                                {
                                        l_cs->m_out_q = l_t_srvr->m_orphan_out_q;
                                        l_t_srvr->m_orphan_out_q = l_t_srvr->get_nbq(NULL);
                                        l_cs->m_out_q->reset_write();
                                }
                                l_out_q = l_cs->m_out_q;
                        }
                        else if(l_t_srvr)
                        {
                                TRC_WARN("l_out_q == t_srvr orphan out q\n");
                                l_out_q = l_t_srvr->m_orphan_out_q;
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
                        l_s = l_nconn->nc_write(l_out_q, l_written);
                        if(l_t_srvr) { l_t_srvr->m_stat.m_bytes_written += l_written; }
                        if(l_cs) {l_cs->m_access_info.m_bytes_out += l_written; }
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
                                l_nconn->set_state_done();
                                goto state_top;
                        }
                        // ---------------------------------
                        // NC_STATUS_ERROR
                        // ---------------------------------
                        case nconn::NC_STATUS_ERROR:
                        {
                                l_nconn->set_state_done();
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
                        // ---------------------------------
                        // proxy backpressure
                        // ---------------------------------
                        // TODO
#if 0
                        if(l_cs->m_u &&
                          (l_cs->m_u->get_type() == proxy_u::S_UPS_TYPE_PROXY))
                        {
                                proxy_u *l_proxy_u = static_cast<proxy_u *>(l_cs->m_u);
                                if(l_proxy_u->get_subr() &&
                                   l_proxy_u->get_subr()->get_ups_srvr_session() &&
                                   l_proxy_u->get_again() &&
                                   !l_cs->m_out_q->read_avail_is_max_limit())
                                {
                                        //TRC_DEBUG("set_again(false): l_cs: %p l_nconn: %p path: %s\n",
                                        //          l_cs,
                                        //          l_nconn,
                                        //          l_proxy_u->get_subr()->get_path().c_str());
                                        //NDBG_PRINT("dequeueing\n");
                                        l_proxy_u->set_again(false);
                                        l_s = l_proxy_u->get_subr()->get_ups_srvr_session()->queue_input();
                                }
                        }
#endif
                        // ---------------------------------
                        // check is done
                        // ---------------------------------
                        if(l_cs &&
                           l_cs->m_out_q &&
                           !l_cs->m_out_q->read_avail())
                        {
                                bool l_done = false;
                                bool l_shutdown = false;
                                if(l_cs->m_u)
                                {
                                        // -----------------
                                        // done
                                        // -----------------
                                        if(l_cs->m_u->ups_done())
                                        {
                                                l_shutdown = l_cs->m_u->get_shutdown();
                                                delete l_cs->m_u;
                                                l_cs->m_u = NULL;
                                                l_done = true;
                                        }
                                        // -----------------
                                        // read ahead
                                        // -----------------
                                        else
                                        {
                                                ssize_t l_ra_s = 0;
                                                l_ra_s = l_cs->m_u->ups_read_ahead(64*1024);
                                                if(l_ra_s > 0)
                                                {
                                                        goto state_top;
                                                }
                                        }
                                }
                                else if(!l_nconn->is_accepting())
                                {
                                        l_done = true;
                                }
                                if(l_done)
                                {
                                        // -----------------
                                        // stat
                                        // -----------------
                                        //l_cs->m_access_info.m_total_time_ms = get_time_ms() - l_cs->m_access_info.m_start_time_ms;
                                        // -----------------
                                        // done
                                        // -----------------
                                        if(l_cs->m_resp_done_cb)
                                        {
                                                int32_t l_status;
                                                l_status = l_cs->m_resp_done_cb(*l_cs);
                                                if(l_status != 0)
                                                {
                                                        // TODO Do nothing???
                                                }
                                                l_cs->m_access_info.clear();
                                        }
                                        l_cs->m_out_q->reset_write();
                                        if(!l_shutdown &&
                                           (l_cs->m_rqst != NULL) &&
                                           (l_cs->m_rqst->m_supports_keep_alives))
                                        {
                                                l_cs->m_nconn->nc_set_connected();
                                                l_cs->m_rqst->init();
                                                l_cs->m_rqst->m_supports_keep_alives = true;
                                                // TODO -check status
                                                return STATUS_OK;
                                        }
                                        else
                                        {
                                                l_nconn->set_state_done();
                                                goto state_top;
                                        }
                                }
                                else
                                {
                                        return STATUS_OK;
                                }
                        }
                        goto state_top;
                }
                // -----------------------------------------
                // TODO
                // -----------------------------------------
                default:
                {
                        TRC_ERROR("unexpected conn mode %d\n", a_conn_mode);
                        return STATUS_ERROR;
                }
                }
        }
        // -------------------------------------------------
        // default
        // -------------------------------------------------
        default:
        {
                //NDBG_PRINT("default\n");
                TRC_ERROR("unexpected conn state %d\n", l_nconn->get_state());
                return STATUS_ERROR;
        }
        }
        return STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t session::evr_fd_readable_cb(void *a_data)
{
        return run_state_machine(a_data, EVR_MODE_READ);
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t session::evr_fd_writeable_cb(void *a_data)
{
        return run_state_machine(a_data, EVR_MODE_WRITE);
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t session::evr_fd_error_cb(void *a_data)
{
        return run_state_machine(a_data, EVR_MODE_ERROR);
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t session::evr_event_timeout_cb(void *a_data)
{
        // -------------------------------------------------
        // clear event
        // -------------------------------------------------
        CHECK_FOR_NULL_ERROR(a_data);
        nconn* l_nconn = static_cast<nconn*>(a_data);
        session *l_cs = static_cast<session *>(l_nconn->get_data());
        if(l_cs &&
           l_cs->m_evr_timeout)
        {
                l_cs->m_evr_timeout = NULL;
        }
        return run_state_machine(a_data, EVR_MODE_TIMEOUT);
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t session::evr_event_readable_cb(void *a_data)
{
        // -------------------------------------------------
        // clear event
        // -------------------------------------------------
        CHECK_FOR_NULL_ERROR(a_data);
        nconn* l_nconn = static_cast<nconn*>(a_data);
        session *l_cs = static_cast<session *>(l_nconn->get_data());
        if(l_cs &&
           l_cs->m_evr_readable)
        {
                l_cs->m_evr_readable = NULL;
        }
        return run_state_machine(a_data, EVR_MODE_READ);
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t session::evr_event_writeable_cb(void *a_data)
{
        // -------------------------------------------------
        // clear event
        // -------------------------------------------------
        CHECK_FOR_NULL_ERROR(a_data);
        nconn* l_nconn = static_cast<nconn*>(a_data);
        session *l_cs = static_cast<session *>(l_nconn->get_data());
        if(l_cs &&
           l_cs->m_evr_writeable)
        {
                l_cs->m_evr_writeable = NULL;
        }
        return run_state_machine(a_data, EVR_MODE_WRITE);
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t session::handle_req(void)
{
        if(!m_lsnr)
        {
                TRC_ERROR("m_lsnr == NULL\n");
                return STATUS_ERROR;
        }
        url_router *l_router = m_lsnr->get_url_router();
        if(!l_router)
        {
                TRC_ERROR("m_lsnr->get_url_router() == NULL\n");
                return STATUS_ERROR;
        }
        const data_t &l_url = m_rqst->get_url();
        if(!l_url.m_data ||
           !l_url.m_len)
        {
                TRC_ERROR("rqst url empty\n");
                return STATUS_ERROR;
        }
        // -------------------------------------------------
        // *************************************************
        //             A C C E S S   I N F O
        // *************************************************
        // TODO -this is a lil clunky...
        // -------------------------------------------------
        m_access_info.m_start_time_ms = get_time_ms();
        m_access_info.m_rqst_request.assign(l_url.m_data, l_url.m_len);
        const mutable_data_map_list_t &l_headers = m_rqst->get_header_map();
        mutable_data_t l_v;
#define SET_ACCESS_INFO(_str, _field) do { \
        if(find_first(l_v, l_headers, _str, sizeof(_str))) { \
                m_access_info.m_rqst_##_field.assign(l_v.m_data, l_v.m_len); \
        }} while(0)
        SET_ACCESS_INFO("User-Agent", http_user_agent);
        SET_ACCESS_INFO("Host", host);
        SET_ACCESS_INFO("Referer", http_referer);
        m_access_info.m_rqst_method = m_rqst->get_method_str();
        m_access_info.m_rqst_http_major = m_rqst->m_http_major;
        m_access_info.m_rqst_http_minor = m_rqst->m_http_minor;
        TRC_DEBUG("RQST: %s %.*s --client_session: %p\n",
                  m_rqst->get_method_str(),
                  l_url.m_len, l_url.m_data,
                  this);
        // -------------------------------------------------
        // find route
        // -------------------------------------------------
        url_pmap_t l_pmap;
        h_resp_t l_hdlr_status = H_RESP_NONE;
        const data_t &l_url_path = m_rqst->get_url_path();
        rqst_h *l_rqst_h = NULL;
        l_rqst_h = (rqst_h *)l_router->find_route(l_url_path.m_data,
                                                  l_url_path.m_len,
                                                  l_pmap);
        TRC_VERBOSE("l_rqst_h: %p\n", l_rqst_h);
        if(l_rqst_h)
        {
                if(l_rqst_h->get_do_default())
                {
                        l_hdlr_status = l_rqst_h->do_default(*this, *m_rqst, l_pmap);
                }
                else
                {
                        // Method switch
                        switch(m_rqst->m_method)
                        {
                        case HTTP_GET:
                        {
                                l_hdlr_status = l_rqst_h->do_get(*this, *m_rqst, l_pmap);
                                break;
                        }
                        case HTTP_POST:
                        {
                                l_hdlr_status = l_rqst_h->do_post(*this, *m_rqst, l_pmap);
                                break;
                        }
                        case HTTP_PUT:
                        {
                                l_hdlr_status = l_rqst_h->do_put(*this, *m_rqst, l_pmap);
                                break;
                        }
                        case HTTP_DELETE:
                        {
                                l_hdlr_status = l_rqst_h->do_delete(*this, *m_rqst, l_pmap);
                                break;
                        }
                        default:
                        {
                                l_hdlr_status = l_rqst_h->do_default(*this, *m_rqst, l_pmap);
                                break;
                        }
                        }
                }
        }
        else
        {
                TRC_VERBOSE("no handler\n");
                rqst_h *l_default_rqst_h = m_lsnr->get_default_route();
                if(l_default_rqst_h)
                {
                        TRC_VERBOSE("default handler\n");
                        l_hdlr_status = l_default_rqst_h->do_default(*this, *m_rqst, l_pmap);
                }
                else
                {
                        TRC_VERBOSE("no default handler\n");
                        l_hdlr_status = s_default_rqst_h.do_get(*this, *m_rqst, l_pmap);
                }
        }
        switch(l_hdlr_status)
        {
        case H_RESP_DONE:
        {
                break;
        }
        case H_RESP_DEFERRED:
        {
                break;
        }
        case H_RESP_CLIENT_ERROR:
        {
                l_hdlr_status = s_default_rqst_h.send_bad_request(*this, m_rqst->m_supports_keep_alives);
                break;
        }
        case H_RESP_SERVER_ERROR:
        {
                l_hdlr_status = s_default_rqst_h.send_internal_server_error(*this, m_rqst->m_supports_keep_alives);
                break;
        }
        default:
        {
                break;
        }
        }
        // TODO Handler errors?
        if(l_hdlr_status == H_RESP_SERVER_ERROR)
        {
                return STATUS_ERROR;
        }
        return STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
void session::log_status(void)
{
        t_stat_cntr_t& l_stat = m_t_srvr.m_stat;
        //++l_stat.m_resp;
        uint16_t l_status = m_access_info.m_resp_status;
        if((l_status >= 100) && (l_status < 200)) {/* TODO log 1xx's? */}
        else if((l_status >= 200) && (l_status < 300)){++l_stat.m_resp_status_2xx;}
        else if((l_status >= 300) && (l_status < 400)){++l_stat.m_resp_status_3xx;}
        else if((l_status >= 400) && (l_status < 500)){++l_stat.m_resp_status_4xx;}
        else if((l_status >= 500) && (l_status < 600)){++l_stat.m_resp_status_5xx;}
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t session::queue_output(void)
{
        if(m_evr_writeable)
        {
                // one's already queue'd up
                return STATUS_OK;
        }
        int32_t l_s;
        l_s = m_t_srvr.queue_event(&m_evr_writeable,
                                     session::evr_event_writeable_cb,
                                     m_nconn);
        if(l_s != STATUS_OK)
        {
                TRC_ERROR("performing queue_output\n");
                return STATUS_ERROR;
        }
        return STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t session::subr_enqueue(subr &a_subr)
{
        ups_session::subr_enqueue(a_subr);
        return STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
const std::string &session::get_server_name(void)
{
        return m_t_srvr.get_server_name();
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t session::add_timer(uint32_t a_time_ms, evr_event_cb_t a_cb, void *a_data, void **ao_event)
{
        return m_t_srvr.add_timer(a_time_ms, a_cb, a_data, ao_event);
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
evr_loop *session::get_evr_loop(void)
{
        return m_t_srvr.get_evr_loop();
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
srvr& session::get_srvr(void)
{
        return *(m_t_srvr.get_srvr_instance());
}
//! ----------------------------------------------------------------------------
//! API Responses
//! ----------------------------------------------------------------------------
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
api_resp &create_api_resp(session &a_session)
{
        api_resp *l_api_resp = new api_resp();
        return *l_api_resp;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t queue_api_resp(session &a_session, api_resp &a_api_resp)
{
        if(!a_session.m_out_q)
        {
                a_session.m_out_q = a_session.m_t_srvr.get_nbq(NULL);
        }
        // access info
        a_session.m_access_info.m_resp_status = a_api_resp.get_status();
        a_session.log_status();
        int32_t l_s;
        l_s = a_api_resp.serialize(*(a_session.m_out_q));
        if(l_s != STATUS_OK)
        {
                return STATUS_ERROR;
        }
        l_s = a_session.queue_output();
        if(l_s != STATUS_OK)
        {
                return STATUS_ERROR;
        }
        delete &a_api_resp;
        return STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t queue_resp(session &a_session)
{
        int32_t l_s;
        l_s = a_session.queue_output();
        if(l_s != STATUS_OK)
        {
                return STATUS_ERROR;
        }
        // TODO check status
        UNUSED(l_s);
        return STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t add_timer(void *a_t_srvr,
                  uint32_t a_ms,
                  evr_event_cb_t a_cb,
                  void *a_data,
                  void **ao_timer)
{
        if(!a_t_srvr)
        {
                return STATUS_ERROR;
        }
        t_srvr *l_t_svr = static_cast<t_srvr *>(a_t_srvr);
        int32_t l_status;
        l_status = l_t_svr->add_timer(a_ms, a_cb, a_data, ao_timer);
        if(l_status != STATUS_OK)
        {
                return STATUS_ERROR;
        }
        return STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t cancel_timer(void *a_t_srvr, void *a_timer)
{
        if(!a_t_srvr)
        {
            return STATUS_ERROR;
        }
        t_srvr *l_t_srvr = static_cast<t_srvr *>(a_t_srvr);
        int32_t l_status;
        l_status = l_t_srvr->cancel_event(a_timer);
        if(l_status != STATUS_OK)
        {
                return STATUS_ERROR;
        }
        return STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
nconn *get_nconn(session &a_session)
{
        return a_session.m_nconn;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
const access_info &get_access_info(session &a_session)
{
        return a_session.m_access_info;
}
} //namespace ns_is2 {
