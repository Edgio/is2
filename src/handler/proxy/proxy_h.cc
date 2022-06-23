//! ----------------------------------------------------------------------------
//! Copyright Edgio Inc.
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
#include "is2/status.h"
#ifdef BUILD_TLS_WITH_OPENSSL
#include "nconn/nconn_tls.h"
#endif
#include "is2/support/nbq.h"
#include "is2/support/trace.h"
#include "is2/srvr/resp.h"
#include "is2/srvr/rqst.h"
#include "is2/srvr/session.h"
#include "is2/srvr/subr.h"
#include "is2/srvr/api_resp.h"
#include "is2/handler/proxy_h.h"
namespace ns_is2 {
//! ----------------------------------------------------------------------------
//! handler
//! ----------------------------------------------------------------------------
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
proxy_h::proxy_h(void):
        default_rqst_h(),
        m_ups_host(),
        m_route(),
        // 10 min
        m_timeout_ms(600000),
        // no backpressure -default
        m_max_in_q_size(-1)
{
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
proxy_h::proxy_h(const std::string &a_ups_host, const std::string &a_route):
        default_rqst_h(),
        m_ups_host(a_ups_host),
        m_route(a_route),
        // 10 min
        m_timeout_ms(600000),
        // no backpressure -default
        m_max_in_q_size(-1)
{
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
proxy_h::~proxy_h(void)
{
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
h_resp_t proxy_h::do_default(session &a_session,
                             rqst &a_rqst,
                             const url_pmap_t &a_url_pmap)
{
        // -------------------------------------------------
        // get url
        // -------------------------------------------------
        const data_t &l_url_data = a_rqst.get_url();
        std::string l_route;
        l_route.assign(l_url_data.m_data, l_url_data.m_len);
        if(l_route.empty())
        {
                return H_RESP_SERVER_ERROR;
        }
        if(m_route != "/")
        {
                std::string::size_type i_s = l_route.find(m_route);
                if (i_s != std::string::npos)
                {
                        l_route.erase(i_s, m_route.length());
                }
        }
        std::string l_url = m_ups_host + l_route;
        // -------------------------------------------------
        // create subrequest
        // -------------------------------------------------
        subr *l_subr = new subr(a_session);
        l_subr->init_with_url(l_url);
        l_subr->m_completion_cb = proxy_u::s_completion_cb;
        l_subr->m_error_cb = proxy_u::s_error_cb;
#if 0
        l_subr->set_headers(a_rqst.get_headers());
#endif
        l_subr->set_keepalive(true);
        l_subr->m_timeout_ms = m_timeout_ms;
        l_subr->m_verb = a_rqst.get_method_str();
        nbq *l_body_q = a_rqst.get_body_q();
        l_subr->m_body_q = l_body_q;
        a_rqst.reset_body_q();
        proxy_u *l_pu = new proxy_u(a_session, l_subr);
        l_subr->m_u = l_pu;
        a_session.m_u = l_pu;
        if(!a_session.m_out_q)
        {
                a_session.m_out_q = a_session.m_t_srvr.get_nbq(NULL);
        }
        a_session.m_out_q->set_max_read_queue(m_max_in_q_size);
        int32_t l_s;
        l_s = a_session.subr_enqueue(*l_subr);
        if(l_s != STATUS_OK)
        {
                return H_RESP_SERVER_ERROR;
        }
        return H_RESP_DONE;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
bool proxy_h::get_do_default(void)
{
        return true;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
void proxy_h::set_timeout_ms(uint32_t a_val)
{
        m_timeout_ms = a_val;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
void proxy_h::set_max_in_q_size(int64_t a_val)
{
        m_max_in_q_size = a_val;
}
//! ----------------------------------------------------------------------------
//! upstream object
//! ----------------------------------------------------------------------------
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
proxy_u::proxy_u(session &a_session, subr* a_subr):
        base_u(a_session),
        m_subr(a_subr)
{
        //NDBG_PRINT("%sCONSTRUCT%s\n", ANSI_COLOR_BG_GREEN, ANSI_COLOR_OFF);
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
proxy_u::~proxy_u(void)
{
        if(m_subr)
        {
                delete m_subr;
                m_subr = NULL;
        }
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
ssize_t proxy_u::ups_read(size_t a_len)
{
        if(!m_subr)
        {
                TRC_ERROR("m_subr == NULL\n");
                return STATUS_ERROR;
        }
        ups_session *l_ups = m_subr->m_ups_session;
        if(!l_ups ||
           !l_ups->m_in_q)
        {
                TRC_ERROR("requester ups_session or m_in_q == NULL\n");
                return STATUS_ERROR;
        }
        m_state = UPS_STATE_SENDING;
        if(!m_session.m_out_q)
        {
                TRC_ERROR("m_session.m_out_q == NULL\n");
                return STATUS_ERROR;
        }
        if(!l_ups->m_resp)
        {
                TRC_ERROR("l_ups_srvr_session->m_resp == NULL\n");
                return STATUS_ERROR;
        }
        if(l_ups->m_resp->m_complete)
        {
                m_state = UPS_STATE_DONE;
        }
        int32_t l_s;
        l_s = m_session.queue_output();
        if(l_s != STATUS_OK)
        {
                TRC_ERROR("performing queue_output\n");
                return STATUS_ERROR;

        }
        return a_len;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
ssize_t proxy_u::ups_read_ahead(size_t a_len)
{
        return 0;
}
//! ----------------------------------------------------------------------------
//! \details: Cancel and cleanup
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t proxy_u::ups_cancel(void)
{
        if(ups_done())
        {
                return STATUS_OK;
        }
        m_state = UPS_STATE_DONE;
        if(m_subr)
        {
                m_subr->cancel();
        }
        //NDBG_PRINT("%sUPS_CANCEL%s\n", ANSI_COLOR_BG_RED, ANSI_COLOR_OFF);
        return STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t proxy_u::s_completion_cb(subr &a_subr,
                                 nconn &a_nconn,
                                 resp &a_resp)
{
        //NDBG_PRINT("%ss_completion_cb%s\n", ANSI_COLOR_BG_YELLOW, ANSI_COLOR_OFF);
        a_subr.m_session->m_access_info.m_resp_status = (http_status_t)a_resp.get_status();
        return STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t proxy_u::s_error_cb(subr &a_subr,
                            nconn *a_nconn,
                            http_status_t a_status,
                            const char *a_error_str)
{
        //NDBG_PRINT("%ss_error_cb%s\n", ANSI_COLOR_BG_MAGENTA, ANSI_COLOR_OFF);
        rqst_h::send_json_resp_err(*a_subr.m_session,
                                   false,
                                   // TODO use supports keep-alives from client request
                                   a_status);
        return STATUS_OK;
}
} //namespace ns_is2 {
