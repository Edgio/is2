//: ----------------------------------------------------------------------------
//: Copyright (C) 2018 Verizon.  All Rights Reserved.
//: All Rights Reserved
//:
//: \file:    file_h.h
//: \details: TODO
//: \author:  Reed P. Morrison
//: \date:    12/12/2015
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
#ifndef _FILE_H_H
#define _FILE_H_H
//: ----------------------------------------------------------------------------
//: includes
//: ----------------------------------------------------------------------------
#include "is2/srvr/default_rqst_h.h"
#include "is2/srvr/base_u.h"
#include <string>
//: ----------------------------------------------------------------------------
//: Fwd Decl's
//: ----------------------------------------------------------------------------
namespace ns_is2 {
class nbq;
}
namespace ns_is2 {
//: ----------------------------------------------------------------------------
//: Fwd decl's
//: ----------------------------------------------------------------------------
class nbq;
//: ----------------------------------------------------------------------------
//: Handler
//: ----------------------------------------------------------------------------
class file_h: public default_rqst_h
{
public:
        // -------------------------------------------------
        // public methods
        // -------------------------------------------------
        file_h(void);
        ~file_h();
        h_resp_t do_get(session &a_session, rqst &a_rqst, const url_pmap_t &a_url_pmap);
        int32_t set_root(const std::string &a_root);
        int32_t set_index(const std::string &a_index);
        int32_t set_route(const std::string &a_route);
        // -------------------------------------------------
        // public static methods
        // -------------------------------------------------
        static h_resp_t get_file(session &a_session, rqst &a_rqst, const std::string &a_path);
private:
        // -------------------------------------------------
        // private methods
        // -------------------------------------------------
        // Disallow copy/assign
        file_h& operator=(const file_h &);
        file_h(const file_h &);
        // -------------------------------------------------
        // private members
        // -------------------------------------------------
        std::string m_root;
        std::string m_index;
        std::string m_route;
};
//: ----------------------------------------------------------------------------
//: Upstream Object
//: Example based on Dan Kegel's "Introduction to non-blocking I/O"
//: ref: http://www.kegel.com/dkftpbench/nonblocking.html
//: ----------------------------------------------------------------------------
class file_u: public base_u
{
public:
        // -------------------------------------------------
        // const
        // -------------------------------------------------
        static const uint32_t S_UPS_TYPE_FILE = 0xFFFF000A;
        // -------------------------------------------------
        // public methods
        // -------------------------------------------------
        file_u(session &a_session);
        ~file_u();
        int fsinit(const char *a_filename);
        size_t fssize(void) { return m_size;}
        // -------------------------------------------------
        // upstream methods
        // -------------------------------------------------
        ssize_t ups_read(size_t a_len);
        ssize_t ups_read_ahead(size_t a_len);
        int32_t ups_cancel(void);
        uint32_t ups_get_type(void) { return S_UPS_TYPE_FILE;}
private:
        // -------------------------------------------------
        // private methods
        // -------------------------------------------------
        // Disallow copy/assign
        file_u& operator=(const file_u &);
        file_u(const file_u &);
        // -------------------------------------------------
        // private members
        // -------------------------------------------------
        int m_fd;
        size_t m_size;
        size_t m_read;
};
} //namespace ns_is2 {
#endif
