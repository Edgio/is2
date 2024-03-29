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
//! Includes
//! ----------------------------------------------------------------------------
#include "is2/srvr/api_resp.h"
#include "is2/srvr/http_status.h"
#include "srvr/http_resp.h"
#include "is2/support/nbq.h"
#include "is2/support/string_util.h"
#include "is2/support/time_util.h"
#include "is2/srvr/rqst.h"
#include <string.h>
#include <stdlib.h>
#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>
#include <stdarg.h>
namespace ns_is2 {
//! ----------------------------------------------------------------------------
//! response helpers
//! ----------------------------------------------------------------------------
const char *get_resp_status_str(http_status_t a_status)
{
        const char *l_err_str = NULL;
        http_resp_strs::code_resp_map_t::const_iterator i_r = http_resp_strs::S_CODE_RESP_MAP.find(a_status);
        if(i_r != http_resp_strs::S_CODE_RESP_MAP.end())
        {
                l_err_str = i_r->second.c_str();
        }
        return l_err_str;
}
//! ----------------------------------------------------------------------------
//! nbq utilities
//! ----------------------------------------------------------------------------
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t nbq_write_request_line(nbq &ao_q, const char *a_buf, uint32_t a_len)
{
        ao_q.write(a_buf, a_len);
        ao_q.write("\r\n", strlen("\r\n"));
        return 0;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t nbq_write_status(nbq &ao_q, http_status_t a_status)
{
        http_resp_strs::code_resp_map_t::const_iterator i_r = http_resp_strs::S_CODE_RESP_MAP.find(a_status);
        if(i_r != http_resp_strs::S_CODE_RESP_MAP.end())
        {
                ao_q.write("HTTP/1.1 ", strlen("HTTP/1.1 "));
                char l_status_code_str[10];
                sprintf(l_status_code_str, "%u ", a_status);
                ao_q.write(l_status_code_str, strnlen(l_status_code_str, 10));
                ao_q.write(i_r->second.c_str(), i_r->second.length());
                ao_q.write("\r\n", strlen("\r\n"));
        }
        else
        {
                ao_q.write("HTTP/1.1 900 Missing\r\n", strlen("HTTP/1.1 900 Missing"));
        }
        return 0;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t nbq_write_header(nbq &ao_q,
                         const char *a_key_buf, uint32_t a_key_len,
                         const char *a_val_buf, uint32_t a_val_len)
{
        ao_q.write(a_key_buf, a_key_len);
        ao_q.write(": ", 2);
        ao_q.write(a_val_buf, a_val_len);
        ao_q.write("\r\n", 2);
        return 0;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t nbq_write_header(nbq &ao_q,
                         const char *a_key_buf,
                         const char *a_val_buf)
{
        nbq_write_header(ao_q, a_key_buf, strlen(a_key_buf), a_val_buf, strlen(a_val_buf));
        return 0;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t nbq_write_body(nbq &ao_q, const char *a_buf, uint32_t a_len)
{
        ao_q.write("\r\n", strlen("\r\n"));
        ao_q.write(a_buf, a_len);
        return 0;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
void create_json_resp_str(http_status_t a_status, std::string &ao_resp_str)
{
        ao_resp_str += "{ \"errors\": [";
        ao_resp_str += "{\"code\": ";
        char l_status_code_str[8];
        sprintf(l_status_code_str, "%u", a_status);
        ao_resp_str += l_status_code_str;
        ao_resp_str += ", \"message\": \"";
        http_resp_strs::code_resp_map_t::const_iterator i_r = http_resp_strs::S_CODE_RESP_MAP.find(a_status);
        if(i_r != http_resp_strs::S_CODE_RESP_MAP.end())
        {
                ao_resp_str += i_r->second;
        }
        else
        {
                ao_resp_str += "Missing";
        }
        ao_resp_str += "\"}],";
        ao_resp_str += " \"success\": false}\r\n";
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
api_resp::api_resp():
                m_status(HTTP_STATUS_OK),
                m_headers(),
                m_body_data(NULL),
                m_body_data_len(0)
{

}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
api_resp::~api_resp()
{
        if(m_body_data)
        {
                m_body_data = NULL;
                m_body_data_len = 0;
        }
}
//! ----------------------------------------------------------------------------
//!                                  Getters
//! ----------------------------------------------------------------------------
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
const kv_map_list_t &api_resp::get_headers(void)
{
        return m_headers;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
http_status_t api_resp::get_status(void)
{
        return m_status;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
void api_resp::add_std_headers(http_status_t a_status,
                               const char *a_content_type,
                               uint64_t a_len,
                               bool a_keep_alive,
                               const std::string &a_server_name)
{
        set_status(a_status);
        set_header("Server", a_server_name);
        set_header("Date", get_date_str());
        set_header("Content-type", a_content_type);
        set_header("Connection", a_keep_alive ? "keep-alive" : "close");
        char l_length_str[64];
        sprintf(l_length_str, "%" PRIu64 "", a_len);
        set_header("Content-Length", l_length_str);
}
//! ----------------------------------------------------------------------------
//!                            Set Response
//! ----------------------------------------------------------------------------
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
void api_resp::set_status(http_status_t a_status)
{
        m_status = a_status;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int api_resp::set_header(const std::string &a_header)
{
        int32_t l_s;
        std::string l_header_key;
        std::string l_header_val;
        l_s = break_header_string(a_header, l_header_key, l_header_val);
        if(l_s != 0)
        {
                // If verbose???
                //printf("Error header string[%s] is malformed\n", a_header.c_str());
                return -1;
        }
        l_s = set_header(l_header_key, l_header_val);
        if(l_s != 0)
        {
                // If verbose???
                //printf("Error header string[%s] is malformed\n", a_header.c_str());
                return -1;
        }
        return 0;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int api_resp::set_header(const std::string &a_key, const std::string &a_val)
{
        kv_map_list_t::iterator i_obj = m_headers.find(a_key);
        if(i_obj != m_headers.end())
        {
                i_obj->second.push_back(a_val);
        }
        else
        {
                str_list_t l_list;
                l_list.push_back(a_val);
                m_headers[a_key] = l_list;
        }
        return 0;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
void api_resp::set_headers(const kv_map_list_t &a_headers_list)
{
        m_headers = a_headers_list;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int api_resp::set_headerf(const std::string &a_key, const char* a_fmt, ...)
{
        // -------------------------------------------------
        // maximum size is going to be 8K for header for now
        // -------------------------------------------------
        const int l_max_header_size = 8192;
        // -------------------------------------------------
        // allocate space for header
        // -------------------------------------------------
        char *l_val = (char*)malloc(l_max_header_size);
        // -------------------------------------------------
        // ...
        // -------------------------------------------------
        va_list l_ap;
        va_start(l_ap, a_fmt);
        int l_res = vsnprintf(l_val, l_max_header_size, a_fmt, l_ap);
        va_end(l_ap);
        if(l_res < 0)
        {
                // failed
                if(l_val) { free(l_val); l_val = NULL; }
                return -1;
        }
        // -------------------------------------------------
        // create str...
        // -------------------------------------------------
        std::string l_str = std::string(l_val, l_res);
        if(l_val) { free(l_val); l_val = NULL; }
        // -------------------------------------------------
        // set header
        // -------------------------------------------------
        int32_t l_s;
        l_s = set_header(a_key, l_str);
        return l_s;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
void api_resp::set_body_data(const char *a_ptr, uint32_t a_len)
{
        m_body_data = a_ptr;
        m_body_data_len = a_len;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
void api_resp::clear_headers(void)
{
        m_headers.clear();
}
//! ----------------------------------------------------------------------------
//! \details: Serialize to q for sending.
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t api_resp::serialize(nbq &ao_q)
{
        // -------------------------------------------
        // Reset write ptr.
        // -------------------------------------------
        ao_q.reset_write();
        // -------------------------------------------
        // Write status
        // -------------------------------------------
        nbq_write_status(ao_q, m_status);
        // -------------------------------------------
        // Write headers
        // -------------------------------------------
        // Loop over reqlet map
        for(kv_map_list_t::const_iterator i_hl = m_headers.begin();
            i_hl != m_headers.end();
            ++i_hl)
        {
                if(i_hl->first.empty() || i_hl->second.empty())
                {
                        continue;
                }
                for(str_list_t::const_iterator i_v = i_hl->second.begin();
                    i_v != i_hl->second.end();
                    ++i_v)
                {
                        nbq_write_header(ao_q, i_hl->first.c_str(), i_hl->first.length(), i_v->c_str(), i_v->length());
                }
        }
        // -------------------------------------------
        // Write body
        // -------------------------------------------
        if(m_body_data && m_body_data_len)
        {
                //NDBG_PRINT("Write: buf: %p len: %d\n", l_buf, l_len);
                nbq_write_body(ao_q, m_body_data, m_body_data_len);
        }
        else
        {
                nbq_write_body(ao_q, NULL, 0);
        }
        return 0;
}
} //namespace ns_is2 {
