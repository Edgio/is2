//: ----------------------------------------------------------------------------
//: Copyright (C) 2018 Verizon.  All Rights Reserved.
//: All Rights Reserved
//:
//: \file:    rqst.cc
//: \details: TODO
//: \author:  Reed P. Morrison
//: \date:    07/20/2015
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
//: Includes
//: ----------------------------------------------------------------------------
#include "support/ndebug.h"
#include "srvr/cb.h"
#include "support/uri.h"
#include "is2/support/nbq.h"
#include "is2/support/string_util.h"
#include "is2/srvr/rqst.h"
#include "is2/support/trace.h"
#include "is2/status.h"
#include "http_parser/http_parser.h"
#include <string.h>
#include <stdlib.h>
namespace ns_is2 {
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
rqst::rqst(void):
        hmsg(),
        m_p_url(),
        m_method(HTTP_GET),
        m_expect(false),
        m_url_parsed(false),
        m_url_buf(NULL),
        m_url_buf_len(0),
        m_url(),
        m_url_uri(),
        m_url_host(),
        m_url_path(),
        m_url_query(),
        m_url_fragment(),
        m_query_list(NULL),
        m_query_map(NULL)
{
        init(true);
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
rqst::~rqst(void)
{
        if(m_url_buf) { free(m_url_buf); m_url_buf = NULL; m_url_buf_len = 0;}
        // -------------------------------------------------
        // delete query args
        // -------------------------------------------------
        if(m_query_list)
        {
                for(mutable_arg_list_t::iterator i_q = m_query_list->begin();
                    i_q != m_query_list->end();
                    ++i_q)
                {
                        if(i_q->m_key) { free(i_q->m_key); i_q->m_key = NULL; }
                        if(i_q->m_val) { free(i_q->m_val); i_q->m_val = NULL; }
                }
                delete m_query_list;
                m_query_list = NULL;
        }
        if(m_query_map) { delete m_query_map; m_query_map = NULL; }
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
void rqst::init(bool a_save)
{
        hmsg::init(a_save);
        m_type = hmsg::TYPE_RQST;
        m_save = a_save;
        m_p_url.clear();
        m_method = HTTP_GET;
        m_expect = false;
        m_url.clear();
        m_url_uri.clear();
        m_url_path.clear();
        m_url_query.clear();
        m_url_fragment.clear();
        m_url_parsed = false;
        if(m_http_parser_settings)
        {
                m_http_parser_settings->on_status = hp_on_status;
                m_http_parser_settings->on_message_complete = hp_on_message_complete;
                m_http_parser_settings->on_message_begin = hp_on_message_begin;
                m_http_parser_settings->on_url = hp_on_url;
                m_http_parser_settings->on_header_field = hp_on_header_field;
                m_http_parser_settings->on_header_value = hp_on_header_value;
                m_http_parser_settings->on_headers_complete = hp_on_headers_complete;
                m_http_parser_settings->on_body = hp_on_body;
        }
        if(m_http_parser)
        {
                http_parser_init(m_http_parser, HTTP_REQUEST);
                m_http_parser->data = this;
        }
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
const data_t &rqst::get_url()
{
        if(!m_url_parsed)
        {
                int32_t l_s = parse_uri();
                if(l_s != STATUS_OK)
                {
                        // return empty string
                        return m_url;
                }
        }
        return m_url;
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
const data_t &rqst::get_url_host()
{
        if(!m_url_parsed)
        {
                int32_t l_s = parse_uri();
                if(l_s != STATUS_OK)
                {
                        // do nothing...
                }
        }
        return m_url_host;
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
const data_t &rqst::get_url_path()
{
        if(!m_url_parsed)
        {
                int32_t l_s = parse_uri();
                if(l_s != STATUS_OK)
                {
                        // do nothing...
                }
        }
        return m_url_path;
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
const data_t &rqst::get_url_query()
{
        if(!m_url_parsed)
        {
                int32_t l_s = parse_uri();
                if(l_s != STATUS_OK)
                {
                        // do nothing...
                }
        }
        return m_url_query;
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
const data_t &rqst::get_url_fragment()
{
        if(!m_url_parsed)
        {
                int32_t l_s = parse_uri();
                if(l_s != STATUS_OK)
                {
                        // do nothing...
                }
        }
        return m_url_fragment;
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
const char *rqst::get_method_str()
{
        // -------------------------------------------------
        // TODO enum cast here is of course uncool -but
        // m_method value is populated by http_parser so
        // "ought" to be safe.
        // Will fix later
        // -------------------------------------------------
        return http_method_str((enum http_method)m_method);
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
const mutable_arg_list_t& rqst::get_query_list()
{
        // -------------------------------------------------
        // create query list
        // -------------------------------------------------
        if(!m_query_list)
        {
                m_query_list = new mutable_arg_list_t();
                // parse args
                uint32_t l_invalid_cnt = 0;
                int32_t l_s;
                l_s = parse_args(*m_query_list,
                                 l_invalid_cnt,
                                 m_url_query.m_data,
                                 m_url_query.m_len,
                                 '&');
                if(l_s != STATUS_OK)
                {
                        // TODO log reason???
                        return *m_query_list;
                }
        }
        return *m_query_list;
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
const mutable_data_map_list_t& rqst::get_query_map()
{
        // -------------------------------------------------
        // create header map
        // -------------------------------------------------
        if(!m_query_map)
        {
                const mutable_arg_list_t &l_list = get_query_list();
                m_header_map = new mutable_data_map_list_t();
                for(mutable_arg_list_t::const_iterator i_q = l_list.begin();
                    i_q != l_list.end();
                    ++i_q)
                {
                        mutable_data_t l_k;
                        l_k.m_data = i_q->m_key;
                        l_k.m_len = i_q->m_key_len;
                        mutable_data_t l_v;
                        l_v.m_data = i_q->m_val;
                        l_v.m_len = i_q->m_val_len;
                        mutable_data_map_list_t::iterator i_obj = m_query_map->find(l_k);
                        if(i_obj != m_query_map->end())
                        {
                                i_obj->second.push_back(l_v);
                        }
                        else
                        {
                                mutable_data_list_t l_list;
                                l_list.push_back(l_v);
                                (*m_query_map)[l_k] = l_list;
                        }
                }
        }
        return *m_query_map;
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
int32_t rqst::parse_uri()
{
        if(m_url_parsed)
        {
                return STATUS_OK;
        }
        // Copy out the url...
        // TODO zero copy???
        if(!m_q ||
           !m_p_url.m_len)
        {
                NDBG_PRINT("ERROR!\n");
                return STATUS_ERROR;
        }
        // -------------------------------------------------
        // copy in url for parsing...
        // -------------------------------------------------
        if(m_url_buf) { free(m_url_buf); m_url_buf = NULL; m_url_buf_len = 0;}
        m_url_buf = copy_part(*m_q, m_p_url.m_off, m_p_url.m_len);
        m_url_buf_len = m_p_url.m_len;
        http_parser_url l_url;
        http_parser_url_init(&l_url);
        // silence bleating memory sanitizers...
        //memset(&l_url, 0, sizeof(l_url));
        // -------------------------------------------------
        // parse
        // -------------------------------------------------
        int l_s;
        l_s = http_parser_parse_url(m_url_buf, m_url_buf_len, 0, &l_url);
        if(l_s != 0)
        {
                TRC_ERROR("parsing url: %.*s\n", m_url_buf_len, m_url_buf);
                // TODO get error msg from http_parser
                if(m_url_buf) { free(m_url_buf); m_url_buf = NULL; m_url_buf_len = 0;}
                return STATUS_ERROR;
        }
        // -------------------------------------------------
        // set bits...
        // -------------------------------------------------
        for(uint32_t i_part = 0; i_part < UF_MAX; ++i_part)
        {
                //NDBG_PRINT("i_part: %d offset: %d len: %d\n", i_part, l_url.field_data[i_part].off, l_url.field_data[i_part].len);
                //NDBG_PRINT("len+off: %d\n",       l_url.field_data[i_part].len + l_url.field_data[i_part].off);
                if(l_url.field_data[i_part].len &&
                  // TODO Some bug with parser -parsing urls like "http://127.0.0.1" sans paths
                  ((l_url.field_data[i_part].len + l_url.field_data[i_part].off) <= m_url_buf_len))
                {
                        switch(i_part)
                        {
                        case UF_PATH:
                        {
                                m_url_path.m_data = m_url_buf + l_url.field_data[i_part].off;
                                m_url_path.m_len = l_url.field_data[i_part].len;
                                //NDBG_PRINT("l_part[UF_PATH]: %.*s\n", m_url_path.m_len, m_url_path.m_data);
                                break;
                        }
                        case UF_QUERY:
                        {
                                m_url_query.m_data = m_url_buf + l_url.field_data[i_part].off;
                                m_url_query.m_len = l_url.field_data[i_part].len;
                                //NDBG_PRINT("l_part[UF_QUERY]: %.*s\n", m_url_query.m_len, m_url_query.m_data);
                                break;
                        }
                        case UF_FRAGMENT:
                        {
                                m_url_fragment.m_data = m_url_buf + l_url.field_data[i_part].off;
                                m_url_fragment.m_len = l_url.field_data[i_part].len;
                                //NDBG_PRINT("l_part[UF_FRAGMENT]: %.*s\n", m_url_fragment.m_len, m_url_fragment.m_data);
                                break;
                        }
                        case UF_HOST:
                        {
                                m_url_host.m_data = m_url_buf + l_url.field_data[i_part].off;
                                m_url_host.m_len = l_url.field_data[i_part].len;
                                //NDBG_PRINT("l_part[UF_PATH]: %.*s\n", m_url_host.m_len, m_url_host.m_data);
                                break;
                        }
                        case UF_USERINFO:
                        case UF_SCHEMA:
                        case UF_PORT:
                        default:
                        {
                                break;
                        }
                        }
                }
        }
        // -------------------------------------------------
        // url
        // -------------------------------------------------
        m_url.m_data = m_url_buf;
        m_url.m_len = m_url_buf_len;
        // -------------------------------------------------
        // uri
        // -------------------------------------------------
        m_url_uri.m_data = m_url_path.m_data;
        m_url_uri.m_len = m_url_buf_len - (m_url_path.m_data - m_url_buf);
        // -------------------------------------------------
        // set parsed...
        // -------------------------------------------------
        m_url_parsed = true;
        return STATUS_OK;
}
//: ----------------------------------------------------------------------------
//:                               Debug
//: ----------------------------------------------------------------------------
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
void rqst::show(void)
{
        m_q->reset_read();
        cr_list_t::const_iterator i_k = m_p_h_list_key.begin();
        cr_list_t::const_iterator i_v = m_p_h_list_val.begin();
        print_part(*m_q, m_p_url.m_off, m_p_url.m_len);
        TRC_OUTPUT("\r\n");
        for(;i_k != m_p_h_list_key.end() && i_v != m_p_h_list_val.end(); ++i_k, ++i_v)
        {
                print_part(*m_q, i_k->m_off, i_k->m_len);
                TRC_OUTPUT(": ");
                print_part(*m_q, i_v->m_off, i_v->m_len);
                TRC_OUTPUT("\r\n");
        }
        TRC_OUTPUT("\r\n");
        print_part(*m_q, m_p_body.m_off, m_p_body.m_len);
        TRC_OUTPUT("\r\n");
}
} //namespace ns_is2 {
