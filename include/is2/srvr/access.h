//! ----------------------------------------------------------------------------
//! Copyright Edgio Inc.
//!
//! \file:    TODO
//! \details: TODO
//!
//! Licensed under the terms of the Apache 2.0 open source license.
//! Please refer to the LICENSE file in the project root for the terms.
//! ----------------------------------------------------------------------------
#ifndef _ACCESS_H
#define _ACCESS_H
//! ----------------------------------------------------------------------------
//! includes
//! ----------------------------------------------------------------------------
#include "is2/nconn/scheme.h"
#include "is2/srvr/http_status.h"
#include <cstdint>
#include <sys/socket.h>
#include <string>
namespace ns_is2 {
//! ----------------------------------------------------------------------------
//! Info
//! ----------------------------------------------------------------------------
class access_info {
public:
        // -------------------------------------------------
        // public members
        // -------------------------------------------------
        // -----------------------------------
        // Connection info
        // -----------------------------------
        struct sockaddr_storage m_conn_clnt_sa;
        socklen_t m_conn_clnt_sa_len;
        struct sockaddr_storage m_conn_upsv_sa;
        socklen_t m_conn_upsv_sa_len;
        // -----------------------------------
        // Request info
        // -----------------------------------
        std::string m_rqst_host;
        scheme_t m_rqst_scheme;
        const char *m_rqst_method;
        uint8_t m_rqst_http_major;
        uint8_t m_rqst_http_minor;
        std::string m_rqst_request;
        std::string m_rqst_query_string;
        std::string m_rqst_http_user_agent;
        std::string m_rqst_http_referer;
        uint64_t m_rqst_request_length;
        // TODO reuse/expose method enumeration from http-parser
        //uint16_t m_rqst_request_method;
        // -----------------------------------
        // Response info
        // -----------------------------------
        // local time in the "Common Log Format"
        std::string m_resp_time_local;
        http_status_t m_resp_status;
        uint64_t m_bytes_in;
        uint64_t m_bytes_out;
        uint64_t m_start_time_ms;
        uint64_t m_total_time_ms;
        // -------------------------------------------------
        // public methods
        // -------------------------------------------------
        access_info(void);
        ~access_info(void);
        void clear(void);
private:
        // -------------------------------------------------
        // private methods
        // -------------------------------------------------
        // Disallow copy/assign
        access_info& operator=(const access_info &);
        access_info(const access_info &);
};
}
#endif
