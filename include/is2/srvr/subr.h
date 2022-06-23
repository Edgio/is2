//! ----------------------------------------------------------------------------
//! Copyright Edgio Inc.
//!
//! \file:    TODO
//! \details: TODO
//!
//! Licensed under the terms of the Apache 2.0 open source license.
//! Please refer to the LICENSE file in the project root for the terms.
//! ----------------------------------------------------------------------------
#ifndef _SUBR_H
#define _SUBR_H
//! ----------------------------------------------------------------------------
//! includes
//! ----------------------------------------------------------------------------
#include "is2/srvr/http_status.h"
#include "is2/srvr/base_u.h"
#include "is2/support/data.h"
#include "is2/nconn/scheme.h"
#include "is2/nconn/host_info.h"
#include <string>
#include <list>
namespace ns_is2 {
//! ----------------------------------------------------------------------------
//! fwd decl's
//! ----------------------------------------------------------------------------
class nconn;
class resp;
class nbq;
class t_srvr;
class session;
class ups_session;
class subr;
typedef std::list <subr *> subr_list_t;
class base_u;
//! ----------------------------------------------------------------------------
//! subr
//! ----------------------------------------------------------------------------
class subr
{
public:
        // -------------------------------------------------
        // public types
        // -------------------------------------------------
        // state
        typedef enum {
                SUBR_STATE_NONE = 0,
                SUBR_STATE_QUEUED,
                SUBR_STATE_DNS_LOOKUP,
                SUBR_STATE_ACTIVE
        } subr_state_t;
        // -------------------------------------------------
        // Callbacks
        // -------------------------------------------------
        typedef int32_t (*error_cb_t)(subr &, nconn *, http_status_t, const char *);
        typedef int32_t (*completion_cb_t)(subr &, nconn &, resp &);
        // -------------------------------------------------
        // public methods
        // -------------------------------------------------
        subr(session &a_session);
        subr(const subr &a_subr);
        ~subr();
        const std::string &get_label(void);
        bool get_expect_resp_body_flag(void);
        void set_keepalive(bool a_val);
        void set_host(const std::string &a_val);
        void set_headers(const kv_map_list_t &a_headers_list);
        int set_header(const std::string &a_key, const std::string &a_val);
        int del_header(const std::string &a_key);
        void clear_headers(void);
        void reset_label(void);
        int32_t init_with_url(const std::string &a_url);
        int32_t create_request(nbq &ao_q);
        int32_t cancel(void);
        // -------------------------------------------------
        // public members
        // -------------------------------------------------
        subr_state_t m_state;
        scheme_t m_scheme;
        std::string m_host;
        uint16_t m_port;
        std::string m_server_label;
        int32_t m_timeout_ms;
        std::string m_path;
        std::string m_query;
        std::string m_fragment;
        std::string m_userinfo;
        std::string m_hostname;
        std::string m_verb;
        bool m_keepalive;
        std::string m_id;
        std::string m_where;
        kv_map_list_t m_headers;
        nbq *m_body_q;
        error_cb_t m_error_cb;
        completion_cb_t m_completion_cb;
        void *m_data;
        bool m_detach_resp;
        uint64_t m_uid;
        session *m_session;
        host_info m_host_info;
        uint64_t m_start_time_ms;
        uint64_t m_end_time_ms;
        void *m_lookup_job;
        subr_list_t::iterator m_i_q;
        bool m_tls_verify;
        bool m_tls_sni;
        bool m_tls_self_ok;
        bool m_tls_no_host_check;
        ups_session *m_ups_session;
        base_u *m_u;
private:
        // -------------------------------------------------
        // private  methods
        // -------------------------------------------------
        // Disallow assign
        subr& operator=(const subr &);
};
//! ----------------------------------------------------------------------------
//! upstream object
//! ----------------------------------------------------------------------------
class subr_u: public base_u
{
public:
        // -------------------------------------------------
        // const
        // -------------------------------------------------
        static const uint32_t S_UPS_TYPE_SUBR = 0xFFFF000F;
        // -------------------------------------------------
        // public methods
        // -------------------------------------------------
        subr_u(session &a_session, subr *a_subr);
        ~subr_u();
        // -------------------------------------------------
        // upstream methods
        // -------------------------------------------------
        ssize_t ups_read(size_t a_len);
        ssize_t ups_read_ahead(size_t a_len);
        int32_t ups_cancel(void);
        uint32_t ups_get_type(void) { return S_UPS_TYPE_SUBR;}
private:
        // -------------------------------------------------
        // private  methods
        // -------------------------------------------------
        // Disallow copy/assign
        subr_u& operator=(const subr_u &);
        subr_u(const subr_u &);
        // -------------------------------------------------
        // private  methods
        // -------------------------------------------------
        subr *m_subr;
};
} //namespace ns_is2 {
#endif
