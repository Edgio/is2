//! ----------------------------------------------------------------------------
//! Copyright Edgio Inc.
//!
//! \file:    TODO
//! \details: TODO
//!
//! Licensed under the terms of the Apache 2.0 open source license.
//! Please refer to the LICENSE file in the project root for the terms.
//! ----------------------------------------------------------------------------
//: ----------------------------------------------------------------------------
//: includes
//: ----------------------------------------------------------------------------
#include "srvr/t_srvr.h"
#include "is2/srvr/lsnr.h"
#include "is2/srvr/default_rqst_h.h"
#include "is2/status.h"
#include "hurl/http/api_resp.h"
#include <string.h>
#include <stdio.h>
//#include <google/profiler.h>
namespace ns_wb_ups_srvr {
//: ----------------------------------------------------------------------------
//: globals
//: ----------------------------------------------------------------------------
ns_is2::srvr *g_srvr = NULL;
//: ----------------------------------------------------------------------------
//: handler
//: ----------------------------------------------------------------------------
class bananas_getter: public ns_is2::default_rqst_h
{
public:
        // GET
        ns_is2::h_resp_t do_get(ns_is2::clnt_session &a_clnt_session,
                                ns_is2::rqst &a_rqst,
                                const ns_is2::url_pmap_t &a_url_pmap)
        {
                char l_len_str[64];
                uint32_t l_body_len = strlen("Hello World\n");
                sprintf(l_len_str, "%u", l_body_len);
                ns_is2::api_resp &l_api_resp = create_api_resp(a_clnt_session);
                l_api_resp.set_status(ns_is2::HTTP_STATUS_OK);
                l_api_resp.set_header("Content-Length", l_len_str);
                l_api_resp.set_body_data("Hello World\n", l_body_len);
                ns_is2::queue_api_resp(a_clnt_session, l_api_resp);
                return ns_is2::H_RESP_DONE;
        }
};
//: ----------------------------------------------------------------------------
//: handler
//: ----------------------------------------------------------------------------
class quitter: public ns_is2::default_rqst_h
{
public:
        // GET
        ns_is2::h_resp_t do_get(ns_is2::clnt_session &a_clnt_session,
                                ns_is2::rqst &a_rqst,
                                const ns_is2::url_pmap_t &a_url_pmap)
        {
                if(g_srvr)
                {
                        g_srvr->stop();
                }
                return ns_is2::H_RESP_DONE;
        }
};
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
int32_t run(void)
{
        return STATUS_OK;
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
int32_t stop(void)
{
        return STATUS_OK;
}

} // namespace ns_wb_ups_srvr {
#if 0
//: ----------------------------------------------------------------------------
//: main
//: ----------------------------------------------------------------------------
int main(void)
{
        ns_hlx::lsnr *l_lsnr = new ns_hlx::lsnr(12345, ns_hlx::SCHEME_TCP);
        ns_hlx::rqst_h *l_rqst_h = new bananas_getter();
        ns_hlx::rqst_h *l_rqst_h_quit = new quitter();
        l_lsnr->add_route("/bananas", l_rqst_h);
        l_lsnr->add_route("/quit", l_rqst_h_quit);
        g_srvr = new ns_hlx::srvr();
        g_srvr->register_lsnr(l_lsnr);
        // Run in foreground w/ threads == 0
        g_srvr->set_num_threads(0);
        //ProfilerStart("tmp.prof");
        g_srvr->run();
        //l_hlx->wait_till_stopped();
        //ProfilerStop();
        if(g_srvr) {delete g_srvr; g_srvr = NULL;}
        if(l_rqst_h) {delete l_rqst_h; l_rqst_h = NULL;}
        if(l_rqst_h_quit) {delete l_rqst_h_quit; l_rqst_h_quit = NULL;}
        return 0;
}
#endif
