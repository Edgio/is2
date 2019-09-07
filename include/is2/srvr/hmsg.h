//: ----------------------------------------------------------------------------
//: Copyright (C) 2018 Verizon.  All Rights Reserved.
//: All Rights Reserved
//:
//: \file:    hmsg.h
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
#ifndef _HMSG_H
#define _HMSG_H
//: ----------------------------------------------------------------------------
//: includes
//: ----------------------------------------------------------------------------
#include <stdint.h>
#include "is2/srvr/cr.h"
#include "is2/support/data.h"
//: ----------------------------------------------------------------------------
//: fwd decl's
//: ----------------------------------------------------------------------------
struct http_parser_settings;
struct http_parser;
namespace ns_is2 {
//: ----------------------------------------------------------------------------
//: fwd decl's
//: ----------------------------------------------------------------------------
class nbq;
//: ----------------------------------------------------------------------------
//: \details: http message obj -abstraction of http reqeust / response
//: ----------------------------------------------------------------------------
class hmsg
{
public:
        // -------------------------------------------------
        // public types
        // -------------------------------------------------
        // hobj type
        typedef enum type_enum {
                TYPE_NONE = 0,
                TYPE_RQST,
                TYPE_RESP
        } type_t;
        // -------------------------------------------------
        // public methods
        // -------------------------------------------------
        hmsg();
        virtual ~hmsg();
        // Getters
        type_t get_type(void) const;
        nbq *get_q(void) const;
        nbq *get_body_q(void);
        uint64_t get_body_len(void) const;
        const mutable_arg_list_t& get_header_list();
        const mutable_data_map_list_t& get_header_map();
        uint64_t get_idx(void) const;
        void set_idx(uint64_t a_idx);
        // Setters
        void set_q(nbq *a_q);
        void reset_body_q(void);
        virtual void init(void);
        // Debug
        virtual void show() = 0;
        // -------------------------------------------------
        // public members
        // -------------------------------------------------
        // Parser settings
        http_parser_settings *m_http_parser_settings;
        http_parser *m_http_parser;
        bool m_expect_resp_body_flag;
        uint64_t m_cur_off;
        char * m_cur_buf;
        // -------------------------------------------------
        // raw http request offsets
        // -------------------------------------------------
        cr_list_t m_p_h_list_key;
        cr_list_t m_p_h_list_val;
        cr_t m_p_body;
        int m_http_major;
        int m_http_minor;
        // -------------------------------------------------
        // ...
        // -------------------------------------------------
        //uint16_t m_status;
        bool m_complete;
        bool m_supports_keep_alives;
protected:
        // -------------------------------------------------
        // Protected members
        // -------------------------------------------------
        type_t m_type;
        nbq *m_q;
        nbq *m_body_q;
        uint64_t m_idx;
        mutable_arg_list_t *m_header_list;
        mutable_data_map_list_t *m_header_map;
private:
        // -------------------------------------------------
        // private methods
        // -------------------------------------------------
        // Disallow copy/assign
        hmsg(const hmsg &);
        hmsg& operator=(const hmsg &);
};
}
#endif

