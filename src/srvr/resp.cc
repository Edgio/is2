//! ----------------------------------------------------------------------------
//! Copyright Verizon.
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
#include "is2/support/ndebug.h"
#include "srvr/cb.h"
#include "is2/support/nbq.h"
#include "is2/srvr/resp.h"
#include "is2/support/trace.h"
#include "is2/status.h"
#include "http_parser/http_parser.h"
namespace ns_is2 {
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
resp::resp(void):
        hmsg(),
        m_p_status(),
        m_tls_info_protocol_str(NULL),
        m_tls_info_cipher_str(NULL),
        m_status()
{
        init();
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
resp::~resp(void)
{
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
void resp::clear(void)
{
        init();
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
void resp::init(void)
{
        hmsg::init();
        m_type = hmsg::TYPE_RESP;
        m_p_status.clear();
        m_tls_info_protocol_str = NULL;
        m_tls_info_cipher_str =  NULL;
        m_status = HTTP_STATUS_NONE;
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
        if(m_http_parser_settings)
        {
                m_http_parser->data = this;
                http_parser_init(m_http_parser, HTTP_RESPONSE);
        }
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
uint16_t resp::get_status(void)
{
        return m_status;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
void resp::set_status(http_status_t a_code)
{
        m_status = a_code;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
void resp::show(void)
{
        m_q->reset_read();
        TRC_OUTPUT("HTTP/%d.%d %u ", m_http_major, m_http_minor, m_status);
        print_part(*m_q, m_p_status.m_off, m_p_status.m_len);
        TRC_OUTPUT("\r\n");
        cr_list_t::const_iterator i_k = m_p_h_list_key.begin();
        cr_list_t::const_iterator i_v = m_p_h_list_val.begin();
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
