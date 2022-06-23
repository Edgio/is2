//! ----------------------------------------------------------------------------
//! Copyright Edgio Inc.
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
#include <is2/srvr/resp.h>
#include <is2/handler/proxy_h.h>
#include <is2/support/trace.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
//#include <google/profiler.h>
//! ----------------------------------------------------------------------------
//! \details: sighandler
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
ns_is2::srvr *g_srvr = NULL;
void sig_handler(int signo)
{
        if(signo == SIGINT) { g_srvr->stop();}
}
//! ----------------------------------------------------------------------------
//! main
//! ----------------------------------------------------------------------------
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
        ns_is2::proxy_h *l_proxy_h = new ns_is2::proxy_h("https://www.google.com", "");
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
