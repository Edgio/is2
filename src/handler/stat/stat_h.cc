//: ----------------------------------------------------------------------------
//: Copyright (C) 2016 Verizon.  All Rights Reserved.
//: All Rights Reserved
//:
//: \file:    stat_h.cc
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
#include "is2/status.h"
#include "is2/srvr/srvr.h"
#include "is2/srvr/session.h"
#include "is2/srvr/rqst.h"
#include "is2/srvr/api_resp.h"
#include "is2/srvr/stat.h"
#include "is2/handler/stat_h.h"
#include "is2/support/ndebug.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"
namespace ns_is2 {
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
stat_h::stat_h(void):
        default_rqst_h(),
        m_route()
{
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
stat_h::~stat_h(void)
{
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
ns_is2::h_resp_t stat_h::do_get(ns_is2::session &a_session,
                                ns_is2::rqst &a_rqst,
                                const ns_is2::url_pmap_t &a_url_pmap)
{
        // -------------------------------------------------
        // get path
        // -------------------------------------------------
        std::string l_route = m_route;
        if(m_route[m_route.length() - 1] == '*')
        {
                l_route = m_route.substr(0, m_route.length() - 2);
        }
        std::string l_path;
        l_path.assign(a_rqst.get_url_path().m_data, a_rqst.get_url_path().m_len);
        std::string l_url_path;
        if(!l_route.empty() &&
           (l_path.find(l_route, 0) != std::string::npos))
        {
                size_t l_idx = l_route.length();
                if(l_path[l_idx] == '/')
                {
                        ++l_idx;
                }
                l_url_path = l_path.substr(l_idx, l_path.length() - l_route.length());
        }
        //NDBG_PRINT("l_url_path: %s\n", l_url_path.c_str());
        // -------------------------------------------------
        // stats
        // -------------------------------------------------
        if(l_url_path == "stats.json")
        {
                return get_stats(a_session, a_rqst);
        }
        // -------------------------------------------------
        // version
        // -------------------------------------------------
        else if(l_url_path == "version.json")
        {
                // TODO ...
                return get_version(a_session, a_rqst);
        }
        // -------------------------------------------------
        // default
        // -------------------------------------------------
        else
        {
                return get_stats(a_session, a_rqst);
        }
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
ns_is2::h_resp_t stat_h::get_stats(ns_is2::session &a_session,
                                   ns_is2::rqst &a_rqst)
{
        // -------------------------------------------------
        // Create body...
        // -------------------------------------------------
        t_stat_cntr_t l_stat;
        t_stat_calc_t l_stat_calc;
        a_session.get_srvr().get_stat(l_stat, l_stat_calc);
        rapidjson::StringBuffer l_strbuf;
        rapidjson::Writer<rapidjson::StringBuffer> l_writer(l_strbuf);
        l_writer.StartObject();
#define ADD_MEMBER(_m) do {\
                l_writer.Key(#_m);\
                l_writer.Uint64(l_stat.m_##_m);\
        } while(0)
#define ADD_MEMBER_CALC(_m) do {\
                l_writer.Key(#_m);\
                l_writer.Double(l_stat_calc.m_##_m);\
        } while(0)
        // client
        ADD_MEMBER(bytes_read);
        ADD_MEMBER(bytes_written);
        ADD_MEMBER(reqs);
        ADD_MEMBER(resp_status_2xx);
        ADD_MEMBER(resp_status_3xx);
        ADD_MEMBER(resp_status_4xx);
        ADD_MEMBER(resp_status_5xx);
        // upstream
        ADD_MEMBER(upsv_bytes_read);
        ADD_MEMBER(upsv_bytes_written);
        ADD_MEMBER(upsv_reqs);
        ADD_MEMBER(upsv_resp_status_2xx);
        ADD_MEMBER(upsv_resp_status_3xx);
        ADD_MEMBER(upsv_resp_status_4xx);
        ADD_MEMBER(upsv_resp_status_5xx);
        // client calc'd
        ADD_MEMBER_CALC(req_s);
        ADD_MEMBER_CALC(bytes_read_s);
        ADD_MEMBER_CALC(bytes_write_s);
        ADD_MEMBER_CALC(resp_status_2xx_pcnt);
        ADD_MEMBER_CALC(resp_status_3xx_pcnt);
        ADD_MEMBER_CALC(resp_status_4xx_pcnt);
        ADD_MEMBER_CALC(resp_status_5xx_pcnt);
        // upstream calc'd
        ADD_MEMBER_CALC(upsv_req_s);
        ADD_MEMBER_CALC(upsv_bytes_read_s);
        ADD_MEMBER_CALC(upsv_bytes_write_s);
        ADD_MEMBER_CALC(upsv_resp_status_2xx_pcnt);
        ADD_MEMBER_CALC(upsv_resp_status_3xx_pcnt);
        ADD_MEMBER_CALC(upsv_resp_status_4xx_pcnt);
        ADD_MEMBER_CALC(upsv_resp_status_5xx_pcnt);
        l_writer.EndObject();
        ns_is2::api_resp &l_api_resp = create_api_resp(a_session);
        l_api_resp.add_std_headers(ns_is2::HTTP_STATUS_OK,
                                   "application/json",
                                   l_strbuf.GetSize(),
                                   a_rqst.m_supports_keep_alives,
                                   a_session.get_server_name());
        l_api_resp.set_body_data(l_strbuf.GetString(), l_strbuf.GetSize());
        queue_api_resp(a_session, l_api_resp);
        return ns_is2::H_RESP_DONE;
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
ns_is2::h_resp_t stat_h::get_version(ns_is2::session &a_session,
                                     ns_is2::rqst &a_rqst)
{
        // -------------------------------------------------
        // Create body...
        // -------------------------------------------------
        rapidjson::StringBuffer l_strbuf;
        rapidjson::Writer<rapidjson::StringBuffer> l_writer(l_strbuf);
        l_writer.StartObject();
        l_writer.Key("version");
        // TODO -add in is2 version..
        l_writer.String("0.0.0");
        l_writer.EndObject();
        ns_is2::api_resp &l_api_resp = ns_is2::create_api_resp(a_session);
        l_api_resp.add_std_headers(ns_is2::HTTP_STATUS_OK,
                                   "application/json",
                                   l_strbuf.GetSize(),
                                   a_rqst.m_supports_keep_alives,
                                   a_session.get_server_name());
        l_api_resp.set_body_data(l_strbuf.GetString(), l_strbuf.GetSize());
        queue_api_resp(a_session, l_api_resp);
        return ns_is2::H_RESP_DONE;
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
int32_t stat_h::set_route(const std::string &a_route)
{
        m_route = a_route;
        return STATUS_OK;
}
} //namespace ns_is2 {
