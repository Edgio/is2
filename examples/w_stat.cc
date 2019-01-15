//: ----------------------------------------------------------------------------
//: Copyright (C) 2018 Verizon.  All Rights Reserved.
//: All Rights Reserved
//:
//: \file:    base.cc
//: \details: basic example:
//:           compile with:
//:           g++ ./basic.cc -lis2 -lssl -lcrypto -lpthread
//: \author:  Reed P. Morrison
//: \date:    01/06/2018
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
#include <is2/srvr/srvr.h>
#include <is2/srvr/lsnr.h>
#include <is2/srvr/default_rqst_h.h>
#include <is2/srvr/api_resp.h>
#include <is2/handler/stat_h.h>
#include <string.h>
#include <stdio.h>
//: ----------------------------------------------------------------------------
//: define handler for get
//: ----------------------------------------------------------------------------
class base_handler: public ns_is2::default_rqst_h
{
public:
        // GET
        ns_is2::h_resp_t do_get(ns_is2::session &a_session,
                                ns_is2::rqst &a_rqst,
                                const ns_is2::url_pmap_t &a_url_pmap)
        {
                uint32_t l_body_len = strlen("Hello World\n");
                char l_len_str[64];
                sprintf(l_len_str, "%u", l_body_len);
                ns_is2::api_resp &l_api_resp = create_api_resp(a_session);
                l_api_resp.set_status(ns_is2::HTTP_STATUS_OK);
                l_api_resp.set_header("Content-Length", l_len_str);
                l_api_resp.set_body_data("Hello World\n", l_body_len);
                ns_is2::queue_api_resp(a_session, l_api_resp);
                return ns_is2::H_RESP_DONE;
        }
};
//: ----------------------------------------------------------------------------
//: main
//: ----------------------------------------------------------------------------
int main(void)
{
        ns_is2::srvr *l_srvr = new ns_is2::srvr();
        ns_is2::lsnr *l_lsnr = new ns_is2::lsnr(12345, ns_is2::SCHEME_TCP);
        ns_is2::rqst_h *l_stat_h = new ns_is2::stat_h();
        l_lsnr->add_route("/stat.json", l_stat_h);
        ns_is2::rqst_h *l_rqst_h = new base_handler();
        l_lsnr->set_default_route(l_rqst_h);
        l_srvr->register_lsnr(l_lsnr);
        l_srvr->set_num_threads(0);
        l_srvr->set_stat_update_ms(1000);
        l_srvr->run();
        if(l_srvr) {delete l_srvr; l_srvr = NULL;}
        if(l_rqst_h) {delete l_rqst_h; l_rqst_h = NULL;}
        if(l_stat_h) {delete l_stat_h; l_stat_h = NULL;}
        return 0;
}
