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
#include "is2/srvr/default_rqst_h.h"
#include "is2/srvr/rqst.h"
namespace ns_is2 {
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
default_rqst_h::default_rqst_h(void)
{
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
default_rqst_h::~default_rqst_h()
{
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
h_resp_t default_rqst_h::do_get(session &a_session, rqst &a_rqst, const url_pmap_t &a_url_pmap)
{
        return send_not_found(a_session, a_rqst.m_supports_keep_alives);
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
h_resp_t default_rqst_h::do_post(session &a_session, rqst &a_rqst, const url_pmap_t &a_url_pmap)
{
        return send_not_found(a_session, a_rqst.m_supports_keep_alives);
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
h_resp_t default_rqst_h::do_put(session &a_session, rqst &a_rqst, const url_pmap_t &a_url_pmap)
{
        return send_not_found(a_session, a_rqst.m_supports_keep_alives);
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
h_resp_t default_rqst_h::do_delete(session &a_session, rqst &a_rqst, const url_pmap_t &a_url_pmap)
{
        return send_not_found(a_session, a_rqst.m_supports_keep_alives);
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
h_resp_t default_rqst_h::do_default(session &a_session, rqst &a_rqst, const url_pmap_t &a_url_pmap)
{
        return send_not_found(a_session, a_rqst.m_supports_keep_alives);
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
bool default_rqst_h::get_do_default(void)
{
        return false;
}
} //namespace ns_is2 {
