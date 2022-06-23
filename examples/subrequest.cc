//! ----------------------------------------------------------------------------
//! Copyright Edgio Inc.
//!
//! \file:    TODO
//! \details: TODO
//!
//! Licensed under the terms of the Apache 2.0 open source license.
//! Please refer to the LICENSE file in the project root for the terms.
//! ----------------------------------------------------------------------------
#include <is2/status.h>
#include <is2/srvr/srvr.h>
#include <is2/srvr/lsnr.h>
#include <is2/srvr/rqst.h>
#include <is2/srvr/api_resp.h>
#include <is2/srvr/subr.h>
#include <is2/srvr/session.h>
#include <is2/srvr/default_rqst_h.h>
#include <is2/handler/stat_h.h>
#include <is2/support/trace.h>
#include <is2/support/time_util.h>
#include <string.h>
#include <stdio.h>
#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>
//#include <gperftools/profiler.h>
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
static int32_t create_response(ns_is2::session &a_session,
                               const char *a_resp_buf,
                               uint64_t a_resp_len)
{
        ns_is2::api_resp &l_api_resp = ns_is2::create_api_resp(a_session);
        const std::string &l_date_str = ns_is2::get_date_str();
        l_api_resp.set_status(ns_is2::HTTP_STATUS_OK);
        l_api_resp.set_header("Server",a_session.get_server_name());
        l_api_resp.set_header("Date", l_date_str);
        l_api_resp.set_header("Content-type", "application/json");
        char l_length_str[64];
        sprintf(l_length_str, "%" PRIu64 "", a_resp_len);
        l_api_resp.set_header("Content-Length", l_length_str);
        if(a_session.m_rqst->m_supports_keep_alives)
        {
                l_api_resp.set_header("Connection", "keep-alive");
        }
        else
        {
                l_api_resp.set_header("Connection", "close");
        }
        l_api_resp.set_body_data(a_resp_buf, a_resp_len);
        ns_is2::queue_api_resp(a_session, l_api_resp);
        return STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! define handler for get
//! ----------------------------------------------------------------------------
class subrequest_handler: public ns_is2::default_rqst_h
{
public:
        // DEFAULT
        ns_is2::h_resp_t do_default(ns_is2::session &a_session,
                                    ns_is2::rqst &a_rqst,
                                    const ns_is2::url_pmap_t &a_url_pmap);
        static int32_t s_completion_cb(ns_is2::subr &a_subr,
                                       ns_is2::nconn &a_nconn,
                                       ns_is2::resp &a_resp);
        static int32_t s_error_cb(ns_is2::subr &a_subr,
                                  ns_is2::nconn *a_nconn,
                                  ns_is2::http_status_t a_status,
                                  const char *a_error_str);
};
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
ns_is2::h_resp_t subrequest_handler::do_default(ns_is2::session &a_session,
                                                ns_is2::rqst &a_rqst,
                                                const ns_is2::url_pmap_t &a_url_pmap)
{
        ns_is2::subr *l_subr = new ns_is2::subr(a_session);
        l_subr->init_with_url("http://google.com");
        l_subr->m_verb = a_rqst.get_method_str();
        l_subr->m_completion_cb = s_completion_cb;
        l_subr->m_error_cb = s_error_cb;
        l_subr->set_header("User-Agent", "is2");
        l_subr->set_header("Accept", "*/*");
        l_subr->set_keepalive(true);
        int32_t l_s;
        l_s = a_session.subr_enqueue(*l_subr);
        if(l_s != STATUS_OK)
        {
                // TODO --cancel pending...
                return ns_is2::H_RESP_SERVER_ERROR;
        }
        return ns_is2::H_RESP_DONE;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t subrequest_handler::s_completion_cb(ns_is2::subr &a_subr,
                                            ns_is2::nconn &a_nconn,
                                            ns_is2::resp &a_resp)
{
        //printf("COMPLETE: a_nconn: %p\n", &a_nconn);
        const char l_resp[] = "{\"msg\": \"hello\"}";
        int32_t l_s;
        l_s = create_response(*a_subr.m_session, l_resp, strlen(l_resp));
        (void)l_s;
        return STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t subrequest_handler::s_error_cb(ns_is2::subr &a_subr,
                                       ns_is2::nconn *a_nconn,
                                       ns_is2::http_status_t a_status,
                                       const char *a_error_str)
{
        printf("ERROR: a_nconn: %p\n", &a_nconn);
        const char l_resp[] = "{\"msg\": \"error\"}";
        int32_t l_s;
        l_s = create_response(*a_subr.m_session, l_resp, strlen(l_resp));
        (void)l_s;
        return STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! main
//! ----------------------------------------------------------------------------
int main(void)
{
        //ns_is2::trc_log_level_set(ns_is2::TRC_LOG_LEVEL_ERROR);
        //ns_is2::trc_log_file_open("/dev/stdout");
        ns_is2::srvr *l_srvr = new ns_is2::srvr();
        ns_is2::lsnr *l_lsnr = new ns_is2::lsnr(12345, ns_is2::SCHEME_TCP);
        ns_is2::rqst_h *l_stat_h = new ns_is2::stat_h();
        l_lsnr->add_route("/stat.json", l_stat_h);
        l_srvr->set_stat_update_ms(1000);
        ns_is2::rqst_h *l_subr_h = new subrequest_handler();
        l_lsnr->set_default_route(l_subr_h);
        l_srvr->register_lsnr(l_lsnr);
        // Run in foreground w/ threads == 0
        l_srvr->set_num_threads(0);
        //ProfilerStart("tmp.prof");
        l_srvr->run();
        //l_srvr->wait_till_stopped();
        //ProfilerStop();
        if(l_srvr) {delete l_srvr; l_srvr = NULL;}
        if(l_subr_h) {delete l_subr_h; l_subr_h = NULL;}
        if(l_stat_h) {delete l_stat_h; l_stat_h = NULL;}
        //ns_is2::trc_log_file_close();
        return 0;
}
