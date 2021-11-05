//! ----------------------------------------------------------------------------
//! Copyright Edgecast Inc.
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
