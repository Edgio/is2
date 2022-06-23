//! ----------------------------------------------------------------------------
//! Copyright Edgio Inc.
//!
//! \file:    TODO
//! \details: TODO
//!
//! Licensed under the terms of the Apache 2.0 open source license.
//! Please refer to the LICENSE file in the project root for the terms.
//! ----------------------------------------------------------------------------
#ifndef _UPS_SESSION_H
#define _UPS_SESSION_H
//! ----------------------------------------------------------------------------
//! includes
//! ----------------------------------------------------------------------------
#include "is2/srvr/http_status.h"
namespace ns_is2 {
//! ----------------------------------------------------------------------------
//! fwd decl's
//! ----------------------------------------------------------------------------
class resp;
//! ----------------------------------------------------------------------------
//! upstream object
//! ----------------------------------------------------------------------------
class ups_session
{
public:
        // -------------------------------------------------
        // public methods
        // -------------------------------------------------
        ups_session(subr &a_subr);
        ~ups_session(void);
        int32_t queue_input(void);
        void log_status(uint16_t a_status = 0);
        // -------------------------------------------------
        // Public Static (class) methods
        // -------------------------------------------------
        static int32_t evr_fd_readable_cb(void *a_data);
        static int32_t evr_fd_writeable_cb(void *a_data);
        static int32_t evr_fd_error_cb(void *a_data);
        static int32_t evr_event_timeout_cb(void *a_data);
        static int32_t evr_event_readable_cb(void *a_data);
        static int32_t evr_event_writeable_cb(void *a_data);
        int32_t cancel_evr_timer(void);
        static int32_t teardown(ups_session *a_ups,
                                t_srvr &a_t_srvr,
                                nconn &a_nconn,
                                http_status_t a_status);
        static void subr_enqueue(subr &a_subr);
        static int32_t subr_dequeue(void *a_data);
        static int32_t subr_start(subr &a_subr);
        // -------------------------------------------------
        // public members
        // -------------------------------------------------
        subr &m_subr;
        resp *m_resp;
        nconn *m_nconn;
        nbq *m_in_q;
        bool m_in_q_detached;
        nbq *m_out_q;
        bool m_detach_resp;
        bool m_again;
        uint32_t m_timeout_ms;
        uint64_t m_last_active_ms;
        evr_event_t *m_evr_timeout;
        evr_event_t *m_evr_readable;
        evr_event_t *m_evr_writeable;
private:
        // -------------------------------------------------
        // private  methods
        // -------------------------------------------------
        // Disallow copy/assign
        ups_session& operator=(const ups_session &);
        ups_session(const ups_session &);
        int32_t cancel_evr_readable(void);
        int32_t cancel_evr_writeable(void);
};
#ifdef ASYNC_DNS_SUPPORT
int32_t adns_resolved_cb(const host_info *a_host_info, void *a_data);
#endif
} //namespace ns_is2 {
#endif
