//: ----------------------------------------------------------------------------
//: Copyright (C) 2018 Verizon.  All Rights Reserved.
//: All Rights Reserved
//:
//: \file:    default_rqst_h.cc
//: \details: TODO
//: \author:  Reed P. Morrison
//: \date:    08/29/2015
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
#include "is2/srvr/default_rqst_h.h"
#include "is2/srvr/rqst.h"
namespace ns_is2 {
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
default_rqst_h::default_rqst_h(void)
{
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
default_rqst_h::~default_rqst_h()
{
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
h_resp_t default_rqst_h::do_get(session &a_session, rqst &a_rqst, const url_pmap_t &a_url_pmap)
{
        return send_not_implemented(a_session, a_rqst.m_supports_keep_alives);
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
h_resp_t default_rqst_h::do_post(session &a_session, rqst &a_rqst, const url_pmap_t &a_url_pmap)
{
        return send_not_implemented(a_session, a_rqst.m_supports_keep_alives);
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
h_resp_t default_rqst_h::do_put(session &a_session, rqst &a_rqst, const url_pmap_t &a_url_pmap)
{
        return send_not_implemented(a_session, a_rqst.m_supports_keep_alives);
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
h_resp_t default_rqst_h::do_delete(session &a_session, rqst &a_rqst, const url_pmap_t &a_url_pmap)
{
        return send_not_implemented(a_session, a_rqst.m_supports_keep_alives);
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
h_resp_t default_rqst_h::do_default(session &a_session, rqst &a_rqst, const url_pmap_t &a_url_pmap)
{
        return send_not_implemented(a_session, a_rqst.m_supports_keep_alives);
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
bool default_rqst_h::get_do_default(void)
{
        return false;
}
} //namespace ns_is2 {
