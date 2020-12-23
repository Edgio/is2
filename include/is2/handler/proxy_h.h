//! ----------------------------------------------------------------------------
//! Copyright Verizon.
//!
//! \file:    TODO
//! \details: TODO
//!
//! Licensed under the terms of the Apache 2.0 open source license.
//! Please refer to the LICENSE file in the project root for the terms.
//! ----------------------------------------------------------------------------
#ifndef _PROXY_H_H
#define _PROXY_H_H
//! ----------------------------------------------------------------------------
//! includes
//! ----------------------------------------------------------------------------
#include "is2/srvr/default_rqst_h.h"
#include "is2/srvr/base_u.h"
#include <string>
namespace ns_is2 {
//! ----------------------------------------------------------------------------
//! fwd decl's
//! ----------------------------------------------------------------------------
class t_srvr;
class subr;
//! ----------------------------------------------------------------------------
//! handler
//! ----------------------------------------------------------------------------
class proxy_h: public default_rqst_h
{
public:
        // -------------------------------------------------
        // public methods
        // -------------------------------------------------
        proxy_h(void);
        proxy_h(const std::string &a_ups_host, const std::string &a_route);
        ~proxy_h();
        h_resp_t do_default(session &a_session,
                            rqst &a_rqst,
                            const url_pmap_t &a_url_pmap);
        // Do default method override
        bool get_do_default(void);
        // Setters
        void set_timeout_ms(uint32_t a_val);
        void set_max_in_q_size(int64_t a_val);
private:
        // -------------------------------------------------
        // private  methods
        // -------------------------------------------------
        // Disallow copy/assign
        proxy_h& operator=(const proxy_h &);
        proxy_h(const proxy_h &);
        // -------------------------------------------------
        // private  members
        // -------------------------------------------------
        std::string m_ups_host;
        std::string m_route;
        uint32_t m_timeout_ms;
        int64_t m_max_in_q_size;
};
//! ----------------------------------------------------------------------------
//! upstream object
//! ----------------------------------------------------------------------------
class proxy_u: public base_u
{
public:
        // -------------------------------------------------
        // const
        // -------------------------------------------------
        static const uint32_t S_UPS_TYPE_PROXY = 0xFFFF000B;
        // -------------------------------------------------
        // public methods
        // -------------------------------------------------
        proxy_u(session &a_session, subr *a_subr);
        ~proxy_u();
#if 0
        int32_t cancel_timer(void *a_timer);
#endif
        // -------------------------------------------------
        // upstream methods
        // -------------------------------------------------
        ssize_t ups_read(size_t a_len);
        ssize_t ups_read_ahead(size_t a_len);
        int32_t ups_cancel(void);
        uint32_t ups_get_type(void) { return S_UPS_TYPE_PROXY;}
        // -------------------------------------------------
        // public class methods
        // -------------------------------------------------
        static int32_t s_completion_cb(subr &a_subr, nconn &a_nconn, resp &a_resp);
        static int32_t s_error_cb(subr &a_subr,
                                  nconn *a_nconn,
                                  http_status_t a_status,
                                  const char *a_error_str);
private:
        // -------------------------------------------------
        // private  methods
        // -------------------------------------------------
        // Disallow copy/assign
        proxy_u& operator=(const proxy_u &);
        proxy_u(const proxy_u &);
        // -------------------------------------------------
        // private  methods
        // -------------------------------------------------
        subr *m_subr;
};
} //namespace ns_is2 {
#endif
