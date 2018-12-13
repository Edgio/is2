//: ----------------------------------------------------------------------------
//: Copyright (C) 2018 Verizon.  All Rights Reserved.
//: All Rights Reserved
//:
//: \file:    files.cc
//: \details: file example:
//:           compile with:
//:           g++ ./files.cc -lis2 -lssl -lcrypto -lpthread
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
#include <is2/handler/file_h.h>
#include <string.h>
#include <stdio.h>
//#include <google/profiler.h>
//: ----------------------------------------------------------------------------
//: main
//: ----------------------------------------------------------------------------
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
