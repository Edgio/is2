//: ----------------------------------------------------------------------------
//: Copyright (C) 2018 Verizon.  All Rights Reserved.
//: All Rights Reserved
//:
//: \file:    subr.cc
//: \details: TODO
//: \author:  Reed P. Morrison
//: \date:    01/19/2016
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
#include "srvr/t_srvr.h"
#include "srvr/ups_session.h"
#include "http_parser/http_parser.h"
#include "is2/status.h"
#include "is2/support/ndebug.h"
#include "is2/support/nbq.h"
#include "is2/support/trace.h"
#include "is2/srvr/base_u.h"
#include "is2/srvr/subr.h"
#include "is2/srvr/resp.h"
#include "is2/srvr/api_resp.h"
#include "is2/srvr/session.h"
#include "string.h"
namespace ns_is2 {
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
subr::subr(session &a_session):
        m_state(SUBR_STATE_NONE),
        m_scheme(SCHEME_NONE),
        m_host(),
        m_port(0),
        m_server_label(),
        m_timeout_ms(10000),
        m_path(),
        m_query(),
        m_fragment(),
        m_userinfo(),
        m_hostname(),
        m_verb("GET"),
        m_keepalive(false),
        m_id(),
        m_where(),
        m_headers(),
        m_body_q(NULL),
        m_error_cb(NULL),
        m_completion_cb(NULL),
        m_data(NULL),
        m_detach_resp(false),
        m_uid(0),
        m_session(&a_session),
        m_host_info(),
        m_start_time_ms(0),
        m_end_time_ms(0),
        m_lookup_job(NULL),
        m_i_q(),
        m_tls_verify(false),
        m_tls_sni(false),
        m_tls_self_ok(false),
        m_tls_no_host_check(false),
        m_ups_session(NULL),
        m_u(NULL)
{
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
subr::subr(const subr &a_subr):
        m_state(a_subr.m_state),
        m_scheme(a_subr.m_scheme),
        m_host(a_subr.m_host),
        m_port(a_subr.m_port),
        m_server_label(a_subr.m_server_label),
        m_timeout_ms(a_subr.m_timeout_ms),
        m_path(a_subr.m_path),
        m_query(a_subr.m_query),
        m_fragment(a_subr.m_fragment),
        m_userinfo(a_subr.m_userinfo),
        m_hostname(a_subr.m_hostname),
        m_verb(a_subr.m_verb),
        m_keepalive(a_subr.m_keepalive),
        m_id(a_subr.m_id),
        m_where(a_subr.m_where),
        m_headers(a_subr.m_headers),
        m_body_q(a_subr.m_body_q),
        m_error_cb(a_subr.m_error_cb),
        m_completion_cb(a_subr.m_completion_cb),
        m_detach_resp(a_subr.m_detach_resp),
        m_uid(a_subr.m_uid),
        m_session(a_subr.m_session),
        m_host_info(a_subr.m_host_info),
        m_start_time_ms(a_subr.m_start_time_ms),
        m_end_time_ms(a_subr.m_end_time_ms),
        m_lookup_job(a_subr.m_lookup_job),
        m_i_q(a_subr.m_i_q),
        m_tls_verify(a_subr.m_tls_verify),
        m_tls_sni(a_subr.m_tls_sni),
        m_tls_self_ok(a_subr.m_tls_self_ok),
        m_tls_no_host_check(a_subr.m_tls_no_host_check),
        m_ups_session(a_subr.m_ups_session),
        m_u(a_subr.m_u)
{
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
subr::~subr(void)
{
        if(m_ups_session)
        {
                delete m_ups_session;
                m_ups_session = NULL;
        }
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
void subr::reset_label(void)
{
        switch(m_scheme)
        {
        case SCHEME_NONE:
        {
                m_server_label += "none://";
                break;
        }
        case SCHEME_TCP:
        {
                m_server_label += "http://";
                break;
        }
        case SCHEME_TLS:
        {
                m_server_label += "https://";
                break;
        }
        default:
        {
                m_server_label += "default://";
                break;
        }
        }
        m_server_label += m_host;
        char l_port_str[16];
        snprintf(l_port_str, 16, ":%u", m_port);
        m_server_label += l_port_str;
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
const std::string &subr::get_label(void)
{
        if(m_server_label.empty())
        {
                reset_label();
        }
        return m_server_label;
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
void subr::set_keepalive(bool a_val)
{
        m_keepalive = a_val;
        del_header("Connection");
        if(m_keepalive)
        {
                set_header("Connection", "keep-alive");
                //if(m_num_reqs_per_conn == 1)
                //{
                //        set_num_reqs_per_conn(-1);
                //}
        }
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
void subr::set_host(const std::string &a_val)
{
        m_host = a_val;
        del_header("Host");
        set_header("Host", a_val);
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
bool subr::get_expect_resp_body_flag(void)
{
        if(m_verb == "HEAD")
        {
                return false;
        }
        else
        {
                return true;
        }
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
void subr::set_headers(const kv_map_list_t &a_headers_list)
{
        m_headers = a_headers_list;
        kv_map_list_t::const_iterator i_vl = m_headers.find("Connection");
        if(i_vl != m_headers.end())
        {
                if(i_vl->second.size() &&
                   (strncasecmp(i_vl->second.begin()->c_str(), "keep-alive", sizeof("keep-alive")) == 0))
                {
                        m_keepalive = true;
                }
        }
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
int subr::set_header(const std::string &a_key, const std::string &a_val)
{
        kv_map_list_t::iterator i_obj = m_headers.find(a_key);
        if(i_obj != m_headers.end())
        {
                // Special handling for Host/User-agent/referer
                bool l_replace = false;
                bool l_remove = false;
                if(!strcasecmp(a_key.c_str(), "User-Agent") ||
                   !strcasecmp(a_key.c_str(), "Referer") ||
                   !strcasecmp(a_key.c_str(), "Accept") ||
                   !strcasecmp(a_key.c_str(), "Host"))
                {
                        l_replace = true;
                        if(a_val.empty())
                        {
                                l_remove = true;
                        }
                }
                if(l_replace)
                {
                        i_obj->second.pop_front();
                        if(!l_remove)
                        {
                                i_obj->second.push_back(a_val);
                        }
                }
                else
                {
                        i_obj->second.push_back(a_val);
                }
        }
        else
        {
                str_list_t l_list;
                l_list.push_back(a_val);
                m_headers[a_key] = l_list;
        }
        return STATUS_OK;
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
int subr::del_header(const std::string &a_key)
{
        kv_map_list_t::iterator i_obj = m_headers.find(a_key);
        if(i_obj != m_headers.end())
        {
                m_headers.erase(i_obj);
        }
        return STATUS_OK;
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
void subr::clear_headers(void)
{
        m_headers.clear();
}
//: ----------------------------------------------------------------------------
//:                              Initialize
//: ----------------------------------------------------------------------------
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
int32_t subr::init_with_url(const std::string &a_url)
{
        std::string l_url_fixed = a_url;
        // Find scheme prefix "://"
        if(a_url.find("://", 0) == std::string::npos)
        {
                l_url_fixed = "http://" + a_url;
        }
        //NDBG_PRINT("Parse url:           %s\n", a_url.c_str());
        http_parser_url l_url;
        http_parser_url_init(&l_url);
        // silence bleating memory sanitizers...
        //memset(&l_url, 0, sizeof(l_url));
        int l_s;
        l_s = http_parser_parse_url(l_url_fixed.c_str(), l_url_fixed.length(), 0, &l_url);
        if(l_s != 0)
        {
                TRC_ERROR("Error parsing url: %s\n", l_url_fixed.c_str());
                // TODO get error msg from http_parser
                return STATUS_ERROR;
        }
        // Set no port
        m_port = 0;
        for(uint32_t i_part = 0; i_part < UF_MAX; ++i_part)
        {
                //NDBG_PRINT("i_part: %d offset: %d len: %d\n", i_part, l_url.field_data[i_part].off, l_url.field_data[i_part].len);
                //NDBG_PRINT("len+off: %d\n",       l_url.field_data[i_part].len + l_url.field_data[i_part].off);
                //NDBG_PRINT("a_url.length(): %d\n", (int)a_url.length());
                if(l_url.field_data[i_part].len &&
                  // TODO Some bug with parser -parsing urls like "http://127.0.0.1" sans paths
                  ((l_url.field_data[i_part].len + l_url.field_data[i_part].off) <= l_url_fixed.length()))
                {
                        switch(i_part)
                        {
                        case UF_SCHEMA:
                        {
                                std::string l_part = l_url_fixed.substr(l_url.field_data[i_part].off, l_url.field_data[i_part].len);
                                //NDBG_PRINT("l_part: %s\n", l_part.c_str());
                                if(l_part == "http")
                                {
                                        m_scheme = SCHEME_TCP;
                                }
                                else if(l_part == "https")
                                {
                                        m_scheme = SCHEME_TLS;
                                }
                                else
                                {
                                        TRC_ERROR("Error schema[%s] is unsupported\n", l_part.c_str());
                                        return STATUS_ERROR;
                                }
                                break;
                        }
                        case UF_HOST:
                        {
                                std::string l_part = l_url_fixed.substr(l_url.field_data[i_part].off, l_url.field_data[i_part].len);
                                //NDBG_PRINT("l_part[UF_HOST]: %s\n", l_part.c_str());
                                m_host = l_part;
                                break;
                        }
                        case UF_PORT:
                        {
                                std::string l_part = l_url_fixed.substr(l_url.field_data[i_part].off, l_url.field_data[i_part].len);
                                //NDBG_PRINT("l_part[UF_PORT]: %s\n", l_part.c_str());
                                m_port = (uint16_t)strtoul(l_part.c_str(), NULL, 10);
                                break;
                        }
                        case UF_PATH:
                        {
                                std::string l_part = l_url_fixed.substr(l_url.field_data[i_part].off, l_url.field_data[i_part].len);
                                //NDBG_PRINT("l_part[UF_PATH]: %s\n", l_part.c_str());
                                m_path = l_part;
                                break;
                        }
                        case UF_QUERY:
                        {
                                std::string l_part = l_url_fixed.substr(l_url.field_data[i_part].off, l_url.field_data[i_part].len);
                                //NDBG_PRINT("l_part[UF_QUERY]: %s\n", l_part.c_str());
                                m_query = l_part;
                                break;
                        }
                        case UF_FRAGMENT:
                        {
                                std::string l_part = l_url_fixed.substr(l_url.field_data[i_part].off, l_url.field_data[i_part].len);
                                //NDBG_PRINT("l_part[UF_FRAGMENT]: %s\n", l_part.c_str());
                                m_fragment = l_part;
                                break;
                        }
                        case UF_USERINFO:
                        {
                                std::string l_part = l_url_fixed.substr(l_url.field_data[i_part].off, l_url.field_data[i_part].len);
                                //sNDBG_PRINT("l_part[UF_USERINFO]: %s\n", l_part.c_str());
                                m_userinfo = l_part;
                                break;
                        }
                        default:
                        {
                                break;
                        }
                        }
                }
        }
        // Default ports
        if(!m_port)
        {
                switch(m_scheme)
                {
                case SCHEME_TCP:
                {
                        m_port = 80;
                        break;
                }
                case SCHEME_TLS:
                {
                        m_port = 443;
                        break;
                }
                default:
                {
                        m_port = 80;
                        break;
                }
                }
        }
        //m_num_to_req = m_path_vector.size();
        //NDBG_PRINT("Showing parsed url.\n");
        //m_url.show();
        if (STATUS_OK != l_s)
        {
                // Failure
                TRC_ERROR("Error parsing url: %s.\n", l_url_fixed.c_str());
                return STATUS_ERROR;
        }
        //NDBG_PRINT("Parsed url: %s\n", l_url_fixed.c_str());
        return STATUS_OK;
}
//: ----------------------------------------------------------------------------
//: \details: Cancel active or remove pending subrequest
//: \return:  status OK on success ERROR on fail
//: \param:   TODO
//: ----------------------------------------------------------------------------
int32_t subr::cancel(void)
{
        // -------------------------------------------------
        // cancel subr based on state
        // -------------------------------------------------
        switch(m_state)
        {
        // -------------------------------------------------
        // queue'd
        // -------------------------------------------------
        case subr::SUBR_STATE_QUEUED:
        {
                *(m_i_q) = NULL;
                m_state = SUBR_STATE_NONE;
                if(m_error_cb)
                {
                        m_error_cb(*(this), NULL, HTTP_STATUS_GATEWAY_TIMEOUT, get_resp_status_str(HTTP_STATUS_GATEWAY_TIMEOUT));
                        // TODO Check status...
                }
                break;
        }
#ifdef ASYNC_DNS_SUPPORT
        // -------------------------------------------------
        // lookup
        // -------------------------------------------------
        case subr::SUBR_STATE_DNS_LOOKUP:
        {
                // Get lookup job
                nresolver::job *l_job = static_cast<nresolver::job *>(m_lookup_job);
                if(l_job)
                {
                        l_job->m_cb = NULL;
                }
                if(m_error_cb)
                {
                        m_error_cb(*(this), NULL, HTTP_STATUS_GATEWAY_TIMEOUT, get_resp_status_str(HTTP_STATUS_GATEWAY_TIMEOUT));
                        // TODO Check status...
                }
                m_state = SUBR_STATE_NONE;
                break;
        }
#endif
        // -------------------------------------------------
        // active
        // -------------------------------------------------
        case subr::SUBR_STATE_ACTIVE:
        {
                if(m_ups_session)
                {
                        ups_session::teardown(m_ups_session,
                                              m_session->m_t_srvr,
                                              *m_ups_session->m_nconn,
                                              HTTP_STATUS_GATEWAY_TIMEOUT);
                }
                else if(m_error_cb)
                {
                        m_error_cb(*(this), NULL, HTTP_STATUS_GATEWAY_TIMEOUT, get_resp_status_str(HTTP_STATUS_GATEWAY_TIMEOUT));
                        // TODO Check status...
                }
                m_state = SUBR_STATE_NONE;
                break;
        }
        default:
        {
                break;
        }
        }
        //NDBG_PRINT("%sUPS_CANCEL%s\n", ANSI_COLOR_BG_RED, ANSI_COLOR_OFF);
        return STATUS_OK;
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
int32_t subr::create_request(nbq &ao_q)
{
        std::string l_path_ref = m_path;
        char l_buf[2048];
        int32_t l_len = 0;
        if(l_path_ref.empty())
        {
                l_path_ref = "/";
        }
        if(!(m_query.empty()))
        {
                l_path_ref += "?";
                l_path_ref += m_query;
        }
        //NDBG_PRINT("HOST: %s PATH: %s\n", a_reqlet.m_url.m_host.c_str(), l_path_ref.c_str());
        l_len = snprintf(l_buf, sizeof(l_buf),
                        "%s %s HTTP/1.1",
                        m_verb.c_str(), l_path_ref.c_str());
        nbq_write_request_line(ao_q, l_buf, l_len);
        // -------------------------------------------------
        // Add repo headers
        // -------------------------------------------------
        bool l_specd_host = false;
        bool l_specd_ua = false;
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
                        if (strcasecmp(i_hl->first.c_str(), "host") == 0)
                        {
                                l_specd_host = true;
                        }
                        if (strcasecmp(i_hl->first.c_str(), "user-agent") == 0)
                        {
                                l_specd_ua = true;
                        }
                }
        }
        // -------------------------------------------------
        // Default Host if unspecified
        // -------------------------------------------------
        if (!l_specd_host)
        {
                nbq_write_header(ao_q, "Host", strlen("Host"),
                                 m_host.c_str(), m_host.length());
        }
        // -------------------------------------------------
        // Default server if unspecified
        // -------------------------------------------------
        if (!l_specd_ua)
        {
                const std::string &l_ua = m_session->m_t_srvr.get_server_name();
                nbq_write_header(ao_q, "User-Agent", strlen("User-Agent"),
                                l_ua.c_str(), l_ua.length());
        }
        // -------------------------------------------------
        // body
        // -------------------------------------------------
        if(m_body_q)
        {
                nbq_write_body(ao_q, NULL, 0);
                //NDBG_PRINT("Write: buf: %p len: %d\n", l_buf, l_len);
                int32_t l_s;
                l_s = ao_q.join_ref(*m_body_q);
                if(l_s != STATUS_OK)
                {
                        return STATUS_ERROR;
                }
        }
        else
        {
                nbq_write_body(ao_q, NULL, 0);
        }
        return STATUS_OK;
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
#if 0
void subr::set_uri(const std::string &a_uri)
{
        m_uri = a_uri;
        // -------------------------------------------------
        // TODO Zero copy with something like substring...
        // This is pretty awful performance wise
        // -------------------------------------------------
        // Read uri up to first '?'
        size_t l_query_pos = 0;
        if((l_query_pos = m_uri.find('?', 0)) == std::string::npos)
        {
                // No query string -path == uri
                m_path = m_uri;
                return;
        }
        m_path = m_uri.substr(0, l_query_pos);
        // TODO Url decode???
        std::string l_query = m_uri.substr(l_query_pos + 1, m_uri.length() - l_query_pos + 1);
        // Split the query by '&'
        if(!l_query.empty())
        {
                //NDBG_PRINT("%s__QUERY__%s: l_query: %s\n", ANSI_COLOR_BG_WHITE, ANSI_COLOR_OFF, l_query.c_str());
                size_t l_qi_begin = 0;
                size_t l_qi_end = 0;
                bool l_last = false;
                while (!l_last)
                {
                        l_qi_end = l_query.find('&', l_qi_begin);
                        if(l_qi_end == std::string::npos)
                        {
                                l_last = true;
                                l_qi_end = l_query.length();
                        }
                        std::string l_query_item = l_query.substr(l_qi_begin, l_qi_end - l_qi_begin);
                        // Search for '='
                        size_t l_qi_val_pos = 0;
                        l_qi_val_pos = l_query_item.find('=', 0);
                        std::string l_q_k;
                        std::string l_q_v;
                        if(l_qi_val_pos != std::string::npos)
                        {
                                l_q_k = l_query_item.substr(0, l_qi_val_pos);
                                l_q_v = l_query_item.substr(l_qi_val_pos + 1, l_query_item.length());
                        }
                        else
                        {
                                l_q_k = l_query_item;
                        }
                        //NDBG_PRINT("%s__QUERY__%s: k[%s]: %s\n",
                        //                ANSI_COLOR_BG_WHITE, ANSI_COLOR_OFF, l_q_k.c_str(), l_q_v.c_str());
                        // Add to list
                        kv_list_map_t::iterator i_key = m_query.find(l_q_k);
                        if(i_key == m_query.end())
                        {
                                value_list_t l_list;
                                l_list.push_back(l_q_v);
                                m_query[l_q_k] = l_list;
                        }
                        else
                        {
                                i_key->second.push_back(l_q_v);
                        }
                        // Move fwd
                        l_qi_begin = l_qi_end + 1;
                }
        }
}
#endif
#if 0
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
const kv_list_map_t &subr::get_uri_decoded_query(void)
{
        if(m_query_uri_decoded.empty() && !m_query.empty())
        {
                // Decode the arguments for now
                for(kv_list_map_t::const_iterator i_kv = m_query.begin();
                    i_kv != m_query.end();
                    ++i_kv)
                {
                        value_list_t l_value_list;
                        for(value_list_t::const_iterator i_v = i_kv->second.begin();
                            i_v != i_kv->second.end();
                            ++i_v)
                        {
                                std::string l_v = uri_decode(*i_v);
                                l_value_list.push_back(l_v);
                        }
                        std::string l_k = uri_decode(i_kv->first);
                        m_query_uri_decoded[l_k] = l_value_list;
                }
        }
        return m_query_uri_decoded;
}
#endif
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
#if 0
void subr::show(bool a_color)
{
        std::string l_host_color = "";
        std::string l_query_color = "";
        std::string l_header_color = "";
        std::string l_body_color = "";
        std::string l_off_color = "";
        if(a_color)
        {
                l_host_color = ANSI_COLOR_FG_BLUE;
                l_query_color = ANSI_COLOR_FG_MAGENTA;
                l_header_color = ANSI_COLOR_FG_GREEN;
                l_body_color = ANSI_COLOR_FG_YELLOW;
                l_off_color = ANSI_COLOR_OFF;
        }
        // Host
        NDBG_OUTPUT("%sUri%s:  %s\n", l_host_color.c_str(), l_off_color.c_str(), m_uri.c_str());
        NDBG_OUTPUT("%sPath%s: %s\n", l_host_color.c_str(), l_off_color.c_str(), m_path.c_str());
        // Query
        for(kv_list_map_t::iterator i_key = m_query.begin();
                        i_key != m_query.end();
            ++i_key)
        {
                NDBG_OUTPUT("%s%s%s: %s\n",
                                l_query_color.c_str(), i_key->first.c_str(), l_off_color.c_str(),
                                i_key->second.begin()->c_str());
        }

        // Headers
        for(kv_list_map_t::iterator i_key = m_headers.begin();
            i_key != m_headers.end();
            ++i_key)
        {
                NDBG_OUTPUT("%s%s%s: %s\n",
                                l_header_color.c_str(), i_key->first.c_str(), l_off_color.c_str(),
                                i_key->second.begin()->c_str());
        }
        // Body
        NDBG_OUTPUT("%sBody%s: %s\n", l_body_color.c_str(), l_off_color.c_str(), m_body.c_str());
}
#endif
//: ----------------------------------------------------------------------------
//: upstream object
//: ----------------------------------------------------------------------------
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
subr_u::subr_u(session &a_session, subr *a_subr):
        base_u(a_session),
        m_subr(a_subr)
{
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
subr_u::~subr_u(void)
{
        if(m_subr)
        {
                delete m_subr;
        }
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
ssize_t subr_u::ups_read(size_t a_len)
{
        if(m_subr &&
           m_subr->m_ups_session &&
           m_subr->m_ups_session->m_resp &&
           m_subr->m_ups_session->m_resp->m_complete)
        {
                m_state = UPS_STATE_DONE;
        }
        return a_len;
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
ssize_t subr_u::ups_read_ahead(size_t a_len)
{
        return 0;
}
//: ----------------------------------------------------------------------------
//: \details: Cancel and cleanup
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
int32_t subr_u::ups_cancel(void)
{
        if(ups_done())
        {
                return STATUS_OK;
        }
        m_state = UPS_STATE_DONE;
        if(m_subr)
        {
                m_subr->cancel();
        }
        //NDBG_PRINT("%sUPS_CANCEL%s\n", ANSI_COLOR_BG_RED, ANSI_COLOR_OFF);
        return STATUS_OK;
}
} //namespace ns_is2 {
