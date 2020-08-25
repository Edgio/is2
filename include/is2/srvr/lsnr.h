//: ----------------------------------------------------------------------------
//: Copyright (C) 2018 Verizon.  All Rights Reserved.
//: All Rights Reserved
//:
//: \file:    lsnr.h
//: \details: TODO
//: \author:  Reed P. Morrison
//: \date:    03/11/2015
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
#ifndef _LSNR_H
#define _LSNR_H
//: ----------------------------------------------------------------------------
//: Includes
//: ----------------------------------------------------------------------------
#include "is2/nconn/scheme.h"
// For fixed size types
#include <stdint.h>
#include <string>
#include <sys/socket.h>
//: ----------------------------------------------------------------------------
//: Fwd Decl's
//: ----------------------------------------------------------------------------
namespace ns_is2 {
class nconn;
}
namespace ns_is2 {
//: ----------------------------------------------------------------------------
//: Fwd Decl's
//: ----------------------------------------------------------------------------
class rqst_h;
class url_router;
class t_srvr;
//: ----------------------------------------------------------------------------
//: lsnr
//: ----------------------------------------------------------------------------
class lsnr
{
public:
        // -------------------------------------------------
        // public methods
        // -------------------------------------------------
        lsnr(uint16_t a_port=12345,
             scheme_t a_scheme = SCHEME_TCP,
             rqst_h* a_default_handler = NULL,
             url_router* a_url_router = NULL,
             uint32_t a_local_addr_v4 = 0);
        ~lsnr();
        // Getters
        int32_t get_fd(void) const { return m_fd;}
        scheme_t get_scheme(void) const { return m_scheme;}
        url_router *get_url_router(void) const { return m_url_router;}
        rqst_h * get_default_route(void) { return m_default_handler;}
        void get_sa(sockaddr_storage &ao_sa, socklen_t &ao_sa_len)
        {
                ao_sa = m_sa;
                ao_sa_len = m_sa_len;
        }
        uint16_t get_port(void) { return m_port;}
        uint32_t get_local_addr_v4(void) { return m_local_addr_v4;}
        // Setters
        int32_t set_local_addr_v4(const char *a_addr_str);
        int32_t set_default_route(rqst_h *a_handler);
        int32_t add_route(const std::string &a_endpoint, const rqst_h *a_handler);
        void set_t_srvr(t_srvr *a_t_srvr) { m_t_srvr = a_t_srvr;}
        int32_t init(void);
        // -------------------------------------------------
        // Class methods
        // -------------------------------------------------
        static int32_t evr_fd_readable_cb(void* a_data);
private:
        // -------------------------------------------------
        // private methods
        // -------------------------------------------------
        // Disallow copy/assign
        lsnr& operator=(const lsnr&);
        lsnr(const lsnr&);
        // -------------------------------------------------
        // private members
        // -------------------------------------------------
        scheme_t m_scheme;
        uint32_t m_local_addr_v4;
        uint16_t m_port;
        sockaddr_storage m_sa;
        socklen_t m_sa_len;
        rqst_h* m_default_handler;
        url_router* m_url_router;
        bool m_url_router_ref;
        t_srvr* m_t_srvr;
        int32_t m_fd;
        bool m_is_initd;
};
}
#endif
