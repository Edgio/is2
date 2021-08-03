//! ----------------------------------------------------------------------------
//! Copyright Verizon.
//!
//! \file:    TODO
//! \details: TODO
//!
//! Licensed under the terms of the Apache 2.0 open source license.
//! Please refer to the LICENSE file in the project root for the terms.
//! ----------------------------------------------------------------------------
#include <is2/srvr/srvr.h>
#include <is2/srvr/lsnr.h>
#include <is2/srvr/default_rqst_h.h>
#include <is2/srvr/session.h>
#include <is2/srvr/api_resp.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
// for displaying addresses
#include <arpa/inet.h>
//! ----------------------------------------------------------------------------
//! globals
//! ----------------------------------------------------------------------------
std::string g_access_log_file = "";
FILE *g_accesslog_file_ptr = stdout;
//! ----------------------------------------------------------------------------
//! done callback
//! ----------------------------------------------------------------------------
static int32_t s_resp_done_cb(ns_is2::session &a_session)
{
        //NDBG_PRINT("RESP DONE\n");
        if(!g_accesslog_file_ptr)
        {
                g_accesslog_file_ptr = ::fopen(g_access_log_file.c_str(), "a");
                if(!g_accesslog_file_ptr)
                {
                        printf("Error performing fopen.\n");
                        return -1;
                }
        }
        // -------------------------------------------------
        // nginx default log format...
        // -------------------------------------------------
        // $remote_addr [$time_local] $request $status $body_bytes_sent $http_referer $http_user_agent
        // 127.0.0.1 - - [30/Mar/2016:18:13:04 -0700] "GET / HTTP/1.1" 200 612 "-" "curl/7.35.0"
        // -------------------------------------------------
        // Get access_info
        const ns_is2::access_info &l_ai = ns_is2::get_access_info(a_session);
        char l_cln_addr_str[INET6_ADDRSTRLEN];
        l_cln_addr_str[0] = '\0';
        if(l_ai.m_conn_clnt_sa_len == sizeof(sockaddr_in))
        {
                // a thousand apologies for this monstrosity :(
                errno = 0;
                const char *l_s;
                l_s = inet_ntop(AF_INET,
                                &(((sockaddr_in *)(&l_ai.m_conn_clnt_sa))->sin_addr),
                                l_cln_addr_str,
                                INET_ADDRSTRLEN);
                if(!l_s)
                {
                        printf("Error performing inet_ntop.\n");
                }
        }
        else if(l_ai.m_conn_clnt_sa_len == sizeof(sockaddr_in6))
        {
                // a thousand apologies for this monstrosity :(
                errno = 0;
                const char *l_s;
                l_s = inet_ntop(AF_INET6,
                                &(((sockaddr_in6 *)(&l_ai.m_conn_clnt_sa))->sin6_addr),
                                l_cln_addr_str,
                                INET6_ADDRSTRLEN);
                if(!l_s)
                {
                        printf("Error performing inet_ntop.\n");
                }
        }
        fprintf(g_accesslog_file_ptr, "%s [%s] \"%s %s HTTP/%u.%u\" %u %lu%lu  %lu %s %s\n",
                l_cln_addr_str,                     // remote addr
                "DATE STRING",                      // local time
                l_ai.m_rqst_request.c_str(),        // request line
                l_ai.m_rqst_method,                 // method
                l_ai.m_rqst_http_major,             // HTTP ver major
                l_ai.m_rqst_http_minor,             // HTTP ver minor
                l_ai.m_resp_status,                 // status
                l_ai.m_bytes_in,                    // TODO body bytes sent
                l_ai.m_bytes_out,                   // TODO body bytes sent
                l_ai.m_total_time_ms,               // TODO body bytes sent
                l_ai.m_rqst_http_referer.c_str(),   // referer
                l_ai.m_rqst_http_user_agent.c_str() // user agent
                );
        fflush(g_accesslog_file_ptr);
        return 0;
}
//! ----------------------------------------------------------------------------
//! define handler for get
//! ----------------------------------------------------------------------------
class base_handler: public ns_is2::default_rqst_h
{
public:
        // GET
        ns_is2::h_resp_t do_get(ns_is2::session &a_session,
                                ns_is2::rqst &a_rqst,
                                const ns_is2::url_pmap_t &a_url_pmap)
        {
                char l_len_str[64];
                uint32_t l_body_len = strlen("Hello World\n");
                sprintf(l_len_str, "%u", l_body_len);
                ns_is2::api_resp &l_api_resp = create_api_resp(a_session);
                l_api_resp.set_status(ns_is2::HTTP_STATUS_OK);
                l_api_resp.set_header("Content-Length", l_len_str);
                l_api_resp.set_body_data("Hello World\n", l_body_len);
                ns_is2::queue_api_resp(a_session, l_api_resp);
                return ns_is2::H_RESP_DONE;
        }
};
//! ----------------------------------------------------------------------------
//! main
//! ----------------------------------------------------------------------------
int main(void)
{
        ns_is2::srvr *l_srvr = new ns_is2::srvr();
        ns_is2::lsnr *l_lsnr = new ns_is2::lsnr(12345, ns_is2::SCHEME_TCP);
        ns_is2::rqst_h *l_rqst_h = new base_handler();
        l_lsnr->add_route("/hello", l_rqst_h);
        l_srvr->register_lsnr(l_lsnr);
        l_srvr->set_resp_done_cb(s_resp_done_cb);
        // Run in foreground w/ threads == 0
        l_srvr->set_num_threads(0);
        //ProfilerStart("tmp.prof");
        l_srvr->run();
        //l_srvr->wait_till_stopped();
        //ProfilerStop();
        if(l_srvr) {delete l_srvr; l_srvr = NULL;}
        if(l_rqst_h) {delete l_rqst_h; l_rqst_h = NULL;}
        return 0;
}

