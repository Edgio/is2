//! ----------------------------------------------------------------------------
//! Copyright Verizon.
//!
//! \file:    TODO
//! \details: TODO
//!
//! Licensed under the terms of the Apache 2.0 open source license.
//! Please refer to the LICENSE file in the project root for the terms.
//! ----------------------------------------------------------------------------
#ifndef _RESP_H
#define _RESP_H
//! ----------------------------------------------------------------------------
//! Includes
//! ----------------------------------------------------------------------------
#include "is2/srvr/hmsg.h"
#include "is2/srvr/http_status.h"
namespace ns_is2 {
//! ----------------------------------------------------------------------------
//! \details: TODO
//! ----------------------------------------------------------------------------
class resp : public hmsg
{
public:
        // -------------------------------------------------
        // public methods
        // -------------------------------------------------
        resp();
        ~resp();
        // Getters
        uint16_t get_status(void);
        // Setters
        void set_status(http_status_t a_code);
        void clear(void);
        void init(void);
        // Debug
        void show();
        // -------------------------------------------------
        // public members
        // -------------------------------------------------
        // -------------------------------------------------
        // raw http request offsets
        // -------------------------------------------------
        cr_t m_p_status;
        // TODO REMOVE
        const char *m_tls_info_protocol_str;
        const char *m_tls_info_cipher_str;
private:
        // -------------------------------------------------
        // private methods
        // -------------------------------------------------
        // Disallow copy/assign
        resp& operator=(const resp &);
        resp(const resp &);
        // -------------------------------------------------
        // private members
        // -------------------------------------------------
        http_status_t m_status;
};
}
#endif
