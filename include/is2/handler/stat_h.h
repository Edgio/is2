//! ----------------------------------------------------------------------------
//! Copyright Verizon.
//!
//! \file:    TODO
//! \details: TODO
//!
//! Licensed under the terms of the Apache 2.0 open source license.
//! Please refer to the LICENSE file in the project root for the terms.
//! ----------------------------------------------------------------------------
#ifndef _STAT_H_H
#define _STAT_H_H
//! ----------------------------------------------------------------------------
//! includes
//! ----------------------------------------------------------------------------
#include "is2/srvr/default_rqst_h.h"
namespace ns_is2 {
//! ----------------------------------------------------------------------------
//! file_h
//! ----------------------------------------------------------------------------
class stat_h: public default_rqst_h
{
public:
        // -------------------------------------------------
        // public methods
        // -------------------------------------------------
        stat_h(void);
        ~stat_h();
        h_resp_t do_get(session &a_session, rqst &a_rqst, const url_pmap_t &a_url_pmap);
        int32_t set_route(const std::string &a_route);
private:
        // -------------------------------------------------
        // private methods
        // -------------------------------------------------
        // Disallow copy/assign
        stat_h& operator=(const stat_h &);
        stat_h(const stat_h &);
        // stats
        h_resp_t get_stats(session &a_session, rqst &a_rqst);
        h_resp_t get_proxy_connections(session &a_session, rqst &a_rqst);
        h_resp_t get_version(session &a_session, rqst &a_rqst);
        // -------------------------------------------------
        // private members
        // -------------------------------------------------
        std::string m_route;
};
} //namespace ns_is2 {
#endif
