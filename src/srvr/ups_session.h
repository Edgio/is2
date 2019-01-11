//: ----------------------------------------------------------------------------
//: Copyright (C) 2018 Verizon.  All Rights Reserved.
//: All Rights Reserved
//:
//: \file:    ups_session.h
//: \details: TODO
//: \author:  Reed P. Morrison
//: \date:    12/12/2015
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
#ifndef _UPS_SESSION_H
#define _UPS_SESSION_H
//: ----------------------------------------------------------------------------
//: includes
//: ----------------------------------------------------------------------------
#include "is2/srvr/http_status.h"
namespace ns_is2 {
//: ----------------------------------------------------------------------------
//: fwd decl's
//: ----------------------------------------------------------------------------
class resp;
//: ----------------------------------------------------------------------------
//: upstream object
//: ----------------------------------------------------------------------------
class ups_session
{
public:
        // -------------------------------------------------
        // public methods
        // -------------------------------------------------
        ups_session(subr &a_subr);
        ~ups_session(void);
        int32_t cancel_timer(void *a_timer);
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
        // -------------------------------------------------
        // teardown
        // -------------------------------------------------
        static int32_t teardown(ups_session *a_ups,
                                t_srvr &a_t_srvr,
                                nconn &a_nconn,
                                http_status_t a_status);
        // *************************************************
        // -------------------------------------------------
        // subrequest support
        // -------------------------------------------------
        // *************************************************
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
        int32_t cancel_evr_timer(void);
        int32_t cancel_evr_readable(void);
        int32_t cancel_evr_writeable(void);
};
#ifdef ASYNC_DNS_SUPPORT
int32_t adns_resolved_cb(const host_info *a_host_info, void *a_data);
#endif
} //namespace ns_is2 {
#endif
