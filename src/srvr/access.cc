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
#include "is2/srvr/access.h"
#include <strings.h>
namespace ns_is2 {
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
access_info::access_info(void):
        m_conn_clnt_sa(),
        m_conn_clnt_sa_len(0),
        m_conn_upsv_sa(),
        m_conn_upsv_sa_len(0),
        m_rqst_host(),
        m_rqst_scheme(),
        m_rqst_method(""),
        m_rqst_http_major(0),
        m_rqst_http_minor(0),
        m_rqst_request(),
        m_rqst_query_string(),
        m_rqst_http_user_agent(),
        m_rqst_http_referer(),
        m_rqst_request_length(),
        m_resp_time_local(),
        m_resp_status(),
        m_bytes_in(0),
        m_bytes_out(0),
        m_start_time_ms(0),
        m_total_time_ms(0)
{
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
access_info::~access_info(void)
{
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
void access_info::clear(void)
{
        bzero((char *) &m_conn_upsv_sa, sizeof(m_conn_upsv_sa));
        m_conn_upsv_sa_len = 0;
        m_rqst_host.clear();
        m_rqst_scheme = SCHEME_NONE;
        m_rqst_method = NULL;
        m_rqst_http_major = 0;
        m_rqst_http_minor = 0;
        m_rqst_request.clear();
        m_rqst_query_string.clear();
        m_rqst_http_user_agent.clear();
        m_rqst_http_referer.clear();
        m_rqst_request_length = 0;
        m_resp_time_local.clear();
        m_resp_status = HTTP_STATUS_NONE;
        m_bytes_in = 0;
        m_bytes_out = 0;
        m_start_time_ms = 0;
        m_total_time_ms = 0;
}
}
