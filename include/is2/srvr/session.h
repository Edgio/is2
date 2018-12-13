//: ----------------------------------------------------------------------------
;//: Copyright (C) 2018 Verizon.  All Rights Reserved.
//: All Rights Reserved
//:
//: \file:    session.h
//: \details: TODO
//: \author:  Reed P. Morrison
//: \date:    07/20/2015
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
#ifndef _SESSION_H
#define _SESSION_H
//: ----------------------------------------------------------------------------
//: includes
//: ----------------------------------------------------------------------------
#include "is2/srvr/access.h"
#include "is2/srvr/default_rqst_h.h"
#include "is2/nconn/host_info.h"
#include "is2/nconn/scheme.h"
#include <stdint.h>
namespace ns_is2 {
//: ----------------------------------------------------------------------------
//: Fwd Decl's
//: ----------------------------------------------------------------------------
class nconn;
class rqst;
class nbq;
class t_srvr;
class lsnr;
class session;
class base_u;
class subr;
struct evr_event;
#ifndef evr_event_t
typedef struct evr_event evr_event_t;
#endif
#ifndef resp_done_cb_t
// TODO move to handler specific resp cb...
typedef int32_t (*resp_done_cb_t)(session &);
#endif
//: ----------------------------------------------------------------------------
//: Connection data
//: ----------------------------------------------------------------------------
class session {
public:
        // -------------------------------------------------
        // Public members
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
        // Public static default
        // -------------------------------------------------
        static default_rqst_h s_default_rqst_h;
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
        // Public methods
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
        int32_t add_evr_timer(uint32_t a_ms);
        int32_t cancel_evr_timer(void);
        int32_t cancel_evr_readable(void);
        int32_t cancel_evr_writeable(void);
        const std::string &get_server_name(void);
        // *************************************************
        // -------------------------------------------------
        // subrequest support
        // -------------------------------------------------
        // *************************************************
        int32_t subr_enqueue(subr &a_subr);
private:
        // -------------------------------------------------
        // Private methods
        // -------------------------------------------------
        // Disallow copy/assign
        session& operator=(const session &);
        session(const session &);
        // -------------------------------------------------
        // Private members
        // -------------------------------------------------
        uint64_t m_last_active_ms;
        uint32_t m_timeout_ms;
};
} // ns_is2
#endif
