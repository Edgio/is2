//: ----------------------------------------------------------------------------
//: Copyright (C) 2018 Verizon.  All Rights Reserved.
//: All Rights Reserved
//:
//: \file:    wb_nconn_pool.cc
//: \details: TODO
//: \author:  Reed P. Morrison
//: \date:    02/09/2017
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
//: ----------------------------------------------------------------------------
//: includes
//: ----------------------------------------------------------------------------
#include "catch/catch.hpp"
#include "srvr/t_srvr.h"
#include "is2/srvr/lsnr.h"
#include "is2/status.h"
#include "hurl/support/trace.h"
#include "hurl/support/ndebug.h"
#include "ups_srvr.h"
#include <unistd.h>
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
static int32_t timer_cb(void *a_data)
{
        NDBG_PRINT(0,"timer fired\n");
        if(a_data)
        {
                int32_t *l_count = ((int32_t *)a_data);
                ++(*l_count);
        }
        return STATUS_OK;
}
//: ----------------------------------------------------------------------------
//: Tests
//: ----------------------------------------------------------------------------
TEST_CASE( "t_srvr test", "[t_srvr]" )
{
        SECTION("basic t_srvr test")
        {
                INFO("init");
                // -------------------------------
                // start upstream server
                // -------------------------------
                ns_wb_ups_srvr::run();
                // -------------------------------
                // logging
                // -------------------------------
                ns_is2::trc_log_file_open("/dev/stdout");
                ns_is2::trc_log_level_set(ns_is2::TRC_LOG_LEVEL_ALL);
                // -------------------------------
                // resolver
                // -------------------------------
                int32_t l_s = STATUS_OK;
                ns_is2::nresolver *l_nresolver = new ns_is2::nresolver();
                std::string l_cache_file;
                l_s = l_nresolver->init(false, l_cache_file);
                REQUIRE((l_s == STATUS_OK));
                // -------------------------------
                // t_srvr
                // -------------------------------
                ns_is2::t_conf l_conf;
                l_conf.m_nresolver = l_nresolver;
                ns_is2::t_srvr *l_t_srvr = new ns_is2::t_srvr(&l_conf);
                l_t_srvr->set_srvr_instance(NULL);
                l_s = l_t_srvr->init();
                REQUIRE((l_s == STATUS_OK));
                // -------------------------------
                // lsnr
                // -------------------------------
                ns_is2::lsnr *l_lsnr = NULL;
                l_lsnr = new ns_is2::lsnr(12345, ns_is2::SCHEME_TCP);
                l_s = l_lsnr->init();
                REQUIRE((l_s == STATUS_OK));
                l_s = l_t_srvr->add_lsnr(*l_lsnr);
                REQUIRE((l_s == STATUS_OK));
                // -------------------------------
                // add timer
                // -------------------------------
                void *l_timer = NULL;
                int32_t l_data = 0;
                l_t_srvr->add_timer(100,
                                    timer_cb,
                                    (void*)(&l_data),
                                    (&l_timer));
                REQUIRE((l_timer != NULL));
                // -------------------------------
                // verify timer fires
                // -------------------------------
                NDBG_PRINT("before run\n");
                l_s = l_t_srvr->run_loop();
                NDBG_PRINT("after run\n");
                REQUIRE((l_s == STATUS_OK));
                REQUIRE((l_data == 1));
                // -------------------------------
                // shutdown
                // -------------------------------
                l_t_srvr->stop();
                // -------------------------------
                // teardown/cleanup
                // -------------------------------
                if(l_t_srvr)
                {
                        delete l_t_srvr;
                        l_t_srvr = NULL;
                }
                if(l_nresolver)
                {
                        delete l_nresolver;
                        l_nresolver = NULL;
                }
                // -------------------------------
                // stop upstream server
                // -------------------------------
                ns_wb_ups_srvr::stop();
        }
}
