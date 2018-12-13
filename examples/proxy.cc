//: ----------------------------------------------------------------------------
//: Copyright (C) 2018 Verizon.  All Rights Reserved.
//: All Rights Reserved
//:
//: \file:    proxy.cc
//: \details: proxy example:
//:           compile with:
//:           g++ ./proxy.cc -lis2 -lssl -lcrypto -lpthread
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
#include <is2/srvr/resp.h>
#include <is2/handler/proxy_h.h>
#include <is2/support/trace.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
//#include <google/profiler.h>
//: ----------------------------------------------------------------------------
//: \details: sighandler
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
ns_is2::srvr *g_srvr = NULL;
void sig_handler(int signo)
{
        if(signo == SIGINT) { g_srvr->stop();}
}
//: ----------------------------------------------------------------------------
//: main
//: ----------------------------------------------------------------------------
int main(void)
{
        // -------------------------------------------
        // Sigint handler
        // -------------------------------------------
        if(signal(SIGINT, sig_handler) == SIG_ERR)
        {
                printf("error: can't catch SIGINT\n");
                return -1;
        }
        ns_is2::trc_log_level_set(ns_is2::TRC_LOG_LEVEL_ERROR);
        ns_is2::trc_log_file_open("/dev/stdout");
        ns_is2::lsnr *l_lsnr = new ns_is2::lsnr(12345, ns_is2::SCHEME_TCP);
        ns_is2::proxy_h *l_proxy_h = new ns_is2::proxy_h("http://127.0.0.1:8089", "");
        l_proxy_h->set_timeout_ms(1000);
        l_lsnr->set_default_route(l_proxy_h);
        g_srvr = new ns_is2::srvr();
        g_srvr->register_lsnr(l_lsnr);
        // Run in foreground w/ threads == 0
        g_srvr->set_num_threads(0);
        //ProfilerStart("tmp.prof");
        g_srvr->run();
        //l_srvr->wait_till_stopped();
        //ProfilerStop();
        if(g_srvr) {delete g_srvr; g_srvr = NULL;}
        if(l_proxy_h) {delete l_proxy_h; l_proxy_h = NULL;}
        ns_is2::trc_log_file_close();
        return 0;
}
