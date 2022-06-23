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
#include <is2/handler/file_h.h>
#include <string.h>
#include <stdio.h>
//#include <google/profiler.h>
//! ----------------------------------------------------------------------------
//! main
//! ----------------------------------------------------------------------------
int main(void)
{
        ns_is2::lsnr *l_lsnr = new ns_is2::lsnr(12345, ns_is2::SCHEME_TCP);
        ns_is2::file_h *l_file_h = new ns_is2::file_h();
        l_lsnr->add_route("/*", l_file_h);
        ns_is2::srvr *l_srvr = new ns_is2::srvr();
        l_srvr->register_lsnr(l_lsnr);
        // Run in foreground w/ threads == 0
        l_srvr->set_num_threads(0);
        //ProfilerStart("tmp.prof");
        l_srvr->run();
        //l_srvr->wait_till_stopped();
        //ProfilerStop();
        if(l_srvr) {delete l_srvr; l_srvr = NULL;}
        if(l_file_h) {delete l_file_h; l_file_h = NULL;}
        return 0;
}
