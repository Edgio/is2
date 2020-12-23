//! ----------------------------------------------------------------------------
//! Copyright Verizon.
//!
//! \file:    TODO
//! \details: TODO
//!
//! Licensed under the terms of the Apache 2.0 open source license.
//! Please refer to the LICENSE file in the project root for the terms.
//! ----------------------------------------------------------------------------
#ifndef _BASE_U_H
#define _BASE_U_H
//! ----------------------------------------------------------------------------
//! includes
//! ----------------------------------------------------------------------------
#include <stdint.h>
#include <stdio.h>
namespace ns_is2 {
//! ----------------------------------------------------------------------------
//! Fwd decl's
//! ----------------------------------------------------------------------------
class nbq;
class session;
//! ----------------------------------------------------------------------------
//! base class for all upstream objects
//! ----------------------------------------------------------------------------
class base_u
{
public:
        // -------------------------------------------------
        // public methods
        // -------------------------------------------------
        base_u(session &a_session);
        virtual ~base_u() {};
        virtual ssize_t ups_read(size_t a_len) = 0;
        virtual ssize_t ups_read_ahead(size_t a_len) = 0;
        virtual int32_t ups_cancel(void) = 0;
        virtual uint32_t ups_get_type(void) = 0;
        bool ups_done(void) {return m_state == UPS_STATE_DONE;}
        void set_ups_done(void) { m_state = UPS_STATE_DONE;}
        session &get_session(void) {return m_session;}
        // Signal upstream was shutdown
        void set_shutdown(void) {m_shutdown = true;}
        bool get_shutdown(void) {return m_shutdown;}
protected:
        // -------------------------------------------------
        // Types
        // -------------------------------------------------
        typedef enum
        {
                UPS_STATE_IDLE,
                UPS_STATE_SENDING,
                UPS_STATE_DONE
        } state_t;
        // -------------------------------------------------
        // protected members
        // -------------------------------------------------
        session &m_session;
        state_t m_state;
        bool m_shutdown;
private:
        // -------------------------------------------------
        // private methods
        // -------------------------------------------------
        // Disallow copy/assign
        base_u& operator=(const base_u &);
        base_u(const base_u &);
};
} //namespace ns_is2 {
#endif
