//: ----------------------------------------------------------------------------
//: Copyright (C) 2018 Verizon.  All Rights Reserved.
//: All Rights Reserved
//:
//: \file:    wb_get_info.cc
//: \details: TODO
//: \author:  Reed P. Morrison
//: \date:    02/17/2016
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
#include "is2/support/os.h"
#include "is2/status.h"
#include "catch/catch.hpp"
#include <string>
//: ----------------------------------------------------------------------------
//:
//: ----------------------------------------------------------------------------
#define OK STATUS_OK
#define ERROR STATUS_ERROR
//: ----------------------------------------------------------------------------
//: Tests
//: ----------------------------------------------------------------------------
TEST_CASE( "os", "[os]" ) {
        SECTION("path tests") {
                //: ----------------------------------------
                //:
                //: ----------------------------------------
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
