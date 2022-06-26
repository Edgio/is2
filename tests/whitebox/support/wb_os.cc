//! ----------------------------------------------------------------------------
//! Copyright Edgio Inc.
//!
//! \file:    TODO
//! \details: TODO
//!
//! Licensed under the terms of the Apache 2.0 open source license.
//! Please refer to the LICENSE file in the project root for the terms.
//! ----------------------------------------------------------------------------
//! ----------------------------------------------------------------------------
//! includes
//! ----------------------------------------------------------------------------
#include "is2/support/os.h"
#include "is2/status.h"
#include "catch/catch.hpp"
#include <string>
//! ----------------------------------------------------------------------------
//:
//! ----------------------------------------------------------------------------
#define OK STATUS_OK
#define ERROR STATUS_ERROR
//! ----------------------------------------------------------------------------
//! Tests
//! ----------------------------------------------------------------------------
TEST_CASE( "os", "[os]" ) {
        SECTION("path tests") {
                //! ----------------------------------------
                //:
                //! ----------------------------------------
                struct exp {
                       int32_t m_result;
                       const char *m_route;
                       const char *m_url_path;
                       const char *m_path;
                };
                struct exp g_path_vec[] = {
                        {ERROR, "/is2_www/*", "/is2_www/../../getsrvinfo.py", ""},
                        {ERROR, "/is2_www/*", "/is2_www/./getsrvinfo.py",     ""},
                        {ERROR, "/is2_www/*", "/./wang.py",                  ""},
                        {ERROR, "/is2_www/*", "/bananas",                          ""},
                        {OK,    "/is2_www/*", "/is2_www/wang.py",       "/wang.py"}
                };
                size_t l_n;
                l_n = sizeof(g_path_vec)/sizeof(g_path_vec[0]);
                for(size_t i_n = 0; i_n < l_n; ++i_n)
                {
                        int32_t l_s;
                        std::string l_path;
                        l_s = ns_is2::get_path(l_path, g_path_vec[i_n].m_route, g_path_vec[i_n].m_url_path);
                        REQUIRE((l_s == g_path_vec[i_n].m_result));
                        if(l_s == OK)
                        {
                                REQUIRE((l_path == g_path_vec[i_n].m_path));
                        }
                }
        }
}
