//! ----------------------------------------------------------------------------
//! Copyright Edgecast Inc.
//!
//! \file:    TODO
//! \details: TODO
//!
//! Licensed under the terms of the Apache 2.0 open source license.
//! Please refer to the LICENSE file in the project root for the terms.
//! ----------------------------------------------------------------------------
#ifndef _DEFAULT_RQST_H
#define _DEFAULT_RQST_H
//! ----------------------------------------------------------------------------
//! Includes
//! ----------------------------------------------------------------------------
#include "is2/srvr/rqst_h.h"
namespace ns_is2 {
//! ----------------------------------------------------------------------------
//! default_rqst_h
//! ----------------------------------------------------------------------------
class default_rqst_h: public rqst_h
{
public:
        // -------------------------------------------------
        // public methods
        // -------------------------------------------------
        default_rqst_h(void);
        ~default_rqst_h();
        h_resp_t do_get(session &a_session, rqst &a_rqst, const url_pmap_t &a_url_pmap);
        h_resp_t do_post(session &a_session, rqst &a_rqst, const url_pmap_t &a_url_pmap);
        h_resp_t do_put(session &a_session, rqst &a_rqst, const url_pmap_t &a_url_pmap);
        h_resp_t do_delete(session &a_session, rqst &a_rqst, const url_pmap_t &a_url_pmap);
        h_resp_t do_default(session &a_session, rqst &a_rqst, const url_pmap_t &a_url_pmap);
        // Do default method override
        bool get_do_default(void);
private:
        // Disallow copy/assign
        default_rqst_h& operator=(const default_rqst_h &);
        default_rqst_h(const default_rqst_h &);
};
}
#endif
