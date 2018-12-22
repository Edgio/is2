//: ----------------------------------------------------------------------------
//: Copyright (C) 2018 Verizon.  All Rights Reserved.
//: All Rights Reserved
//:
//: \file:    resp.h
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
#ifndef _RESP_H
#define _RESP_H
//: ----------------------------------------------------------------------------
//: Includes
//: ----------------------------------------------------------------------------
#include "is2/srvr/hmsg.h"
#include "is2/srvr/http_status.h"
namespace ns_is2 {
//: ----------------------------------------------------------------------------
//: \details: TODO
//: ----------------------------------------------------------------------------
class resp : public hmsg
{
public:
        // -------------------------------------------------
        // public methods
        // -------------------------------------------------
        resp();
        ~resp();
        // Getters
        uint16_t get_status(void);
        // Setters
        void set_status(http_status_t a_code);
        void clear(void);
        void init(bool a_save);
        // Debug
        void show();
        // -------------------------------------------------
        // public members
        // -------------------------------------------------
        // -------------------------------------------------
        // raw http request offsets
        // -------------------------------------------------
        cr_t m_p_status;
        // TODO REMOVE
        const char *m_tls_info_protocol_str;
        const char *m_tls_info_cipher_str;
private:
        // -------------------------------------------------
        // private methods
        // -------------------------------------------------
        // Disallow copy/assign
        resp& operator=(const resp &);
        resp(const resp &);
        // -------------------------------------------------
        // private members
        // -------------------------------------------------
        http_status_t m_status;
};
}
#endif
