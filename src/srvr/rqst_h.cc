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
#include "srvr/t_srvr.h"
#include "is2/srvr/rqst_h.h"
#include "is2/srvr/srvr.h"
#include "is2/srvr/session.h"
#include "is2/srvr/api_resp.h"
#include <string.h>
namespace ns_is2 {
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
h_resp_t rqst_h::send_json_resp(session &a_session,
                                bool a_keep_alive,
                                http_status_t a_status,
                                const char *a_json_resp)
{
        api_resp &l_api_resp = create_api_resp(a_session);
        l_api_resp.add_std_headers(a_status,
                                   "application/json",
                                   strlen(a_json_resp),
                                   a_keep_alive,
                                   a_session.m_t_srvr.get_server_name());
        l_api_resp.set_body_data(a_json_resp, strlen(a_json_resp));
        queue_api_resp(a_session, l_api_resp);
        return H_RESP_DONE;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
h_resp_t rqst_h::send_json_resp_err(session &a_session, bool a_keep_alive, http_status_t a_status)
{
        std::string l_resp;
        create_json_resp_str(a_status, l_resp);
        return send_json_resp(a_session, a_keep_alive, a_status, l_resp.c_str());
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
h_resp_t rqst_h::send_not_found(session &a_session, bool a_keep_alive)
{
        return send_json_resp_err(a_session, a_keep_alive, HTTP_STATUS_NOT_FOUND);
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
h_resp_t rqst_h::send_not_implemented(session &a_session, bool a_keep_alive)
{
        return send_json_resp_err(a_session, a_keep_alive, HTTP_STATUS_NOT_IMPLEMENTED);
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
h_resp_t rqst_h::send_internal_server_error(session &a_session, bool a_keep_alive)
{
        return send_json_resp_err(a_session, a_keep_alive, HTTP_STATUS_INTERNAL_SERVER_ERROR);
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
h_resp_t rqst_h::send_bad_request(session &a_session, bool a_keep_alive)
{
        return send_json_resp_err(a_session, a_keep_alive, HTTP_STATUS_BAD_REQUEST);
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
h_resp_t rqst_h::send_service_not_available(session &a_session, bool a_keep_alive)
{
        return send_json_resp_err(a_session, a_keep_alive, HTTP_STATUS_SERVICE_NOT_AVAILABLE);
}
} //namespace ns_is2 {
