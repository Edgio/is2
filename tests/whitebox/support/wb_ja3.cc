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
#include "is2/status.h"
#include "is2/support/ja3.h"
#include "is2/support/os.h"
#include "catch/catch.hpp"
#include <string>
//! ----------------------------------------------------------------------------
//! Tests
//! ----------------------------------------------------------------------------
TEST_CASE( "ja3", "[ja3]" ) {
        // -----------------------------------------
        //
        // -----------------------------------------
        SECTION("basic tests") {
                ns_is2::ja3 l_ja3;
                std::string l_clnt_hello_bin_file;
                int32_t l_s;
                char* l_buf = nullptr;
                size_t l_len = 0;
                // -----------------------------------------
                // test curl
                // -----------------------------------------
                l_clnt_hello_bin_file = TEST_RESOURCE_DIR;
                l_clnt_hello_bin_file += "tls_client_hello.curl.bin";
                l_s = ns_is2::read_file(l_clnt_hello_bin_file.c_str(), &l_buf, &l_len);
                REQUIRE((l_s == STATUS_OK));
                REQUIRE((l_len > 0));
                REQUIRE((l_buf != nullptr));
                l_ja3.reset();
                l_s = l_ja3.extract_bytes(l_buf, (uint16_t)l_len);
                REQUIRE((l_s == STATUS_OK));
                REQUIRE((l_ja3.get_md5() == "00bcd759cb8ad485fdbf1e7a0c5b94b4"));
                if (l_buf) { free(l_buf); l_buf = nullptr; l_len = 0; }
                // -----------------------------------------
                // test chrome
                // -----------------------------------------
                l_clnt_hello_bin_file = TEST_RESOURCE_DIR;
                l_clnt_hello_bin_file += "tls_client_hello.chrome.bin";
                l_s = ns_is2::read_file(l_clnt_hello_bin_file.c_str(), &l_buf, &l_len);
                REQUIRE((l_s == STATUS_OK));
                REQUIRE((l_len > 0));
                REQUIRE((l_buf != nullptr));
                l_ja3.reset();
                l_s = l_ja3.extract_bytes(l_buf, (uint16_t)l_len);
                REQUIRE((l_s == STATUS_OK));
                REQUIRE((l_ja3.get_md5() == "598872011444709307b861ae817a4b60"));
                if (l_buf) { free(l_buf); l_buf = nullptr; l_len = 0; }
        }
}
