//! ----------------------------------------------------------------------------
//! Copyright Edgecast Inc.
//!
//! \file:    TODO
//! \details: TODO
//!
//! Licensed under the terms of the Apache 2.0 open source license.
//! Please refer to the LICENSE file in the project root for the terms.
//! ----------------------------------------------------------------------------
#ifndef _SESSION_H
#define _SESSION_H
//! ----------------------------------------------------------------------------
//! includes
//! ----------------------------------------------------------------------------
#include "is2/evr/evr.h"
#include "is2/srvr/access.h"
#include "is2/srvr/default_rqst_h.h"
#include "is2/nconn/host_info.h"
#include "is2/nconn/scheme.h"
#include <stdint.h>
namespace ns_is2 {
//! ----------------------------------------------------------------------------
//! Fwd Decl's
//! ----------------------------------------------------------------------------
class nconn;
class rqst;
class nbq;
class t_srvr;
class lsnr;
class session;
class base_u;
class subr;
class srvr;
#ifndef resp_done_cb_t
// TODO move to handler specific resp cb...
typedef int32_t (*resp_done_cb_t)(session &);
#endif
//! ----------------------------------------------------------------------------
//! Connection data
//! ----------------------------------------------------------------------------
class session {
public:
        // -------------------------------------------------
        // public members
        // -------------------------------------------------
        nconn *m_nconn;
        t_srvr &m_t_srvr;
        evr_event_t *m_evr_timeout;
        evr_event_t *m_evr_readable;
        evr_event_t *m_evr_writeable;
        rqst *m_rqst;
        lsnr *m_lsnr;
        nbq *m_in_q;
        nbq *m_out_q;
        uint64_t m_idx;
        base_u *m_u;
        access_info m_access_info;
        resp_done_cb_t m_resp_done_cb;
        // -------------------------------------------------
        // public static default
        // -------------------------------------------------
        static default_rqst_h s_default_rqst_h;
        // -------------------------------------------------
        // public static (class) methods
        // -------------------------------------------------
        static int32_t evr_fd_readable_cb(void *a_data);
        static int32_t evr_fd_writeable_cb(void *a_data);
        static int32_t evr_fd_error_cb(void *a_data);
        static int32_t evr_event_timeout_cb(void *a_data);
        static int32_t evr_event_readable_cb(void *a_data);
        static int32_t evr_event_writeable_cb(void *a_data);
        // -------------------------------------------------
        // public methods
        // -------------------------------------------------
        session(t_srvr &a_t_srvr);
        ~session(void);
        uint64_t get_idx(void) {return m_idx;}
        void set_idx(uint64_t a_idx) {m_idx = a_idx;}
        nbq *get_nbq(void);
        uint32_t get_timeout_ms(void);
        host_info get_host_info(void);
        scheme_t get_scheme(void);
        void set_timeout_ms(uint32_t a_t_ms);
        uint64_t get_last_active_ms(void);
        void set_last_active_ms(uint64_t a_time_ms);
        int32_t queue_output(void);
        int32_t teardown(void);
        int32_t handle_req(void);
        void log_status(void);
        int32_t add_evr_timer(uint32_t a_ms);
        int32_t cancel_evr_timer(void);
        int32_t cancel_evr_readable(void);
        int32_t cancel_evr_writeable(void);
        const std::string &get_server_name(void);
        int32_t add_timer(uint32_t a_time_ms, evr_event_cb_t a_cb, void *a_data, void **ao_event);
        evr_loop *get_evr_loop(void);
        srvr &get_srvr(void);
        int32_t subr_enqueue(subr &a_subr);
private:
        // -------------------------------------------------
        // private methods
        // -------------------------------------------------
        // Disallow copy/assign
        session& operator=(const session &);
        session(const session &);
        // -------------------------------------------------
        // private members
        // -------------------------------------------------
        uint64_t m_last_active_ms;
        uint32_t m_timeout_ms;
};
} // ns_is2
#endif
