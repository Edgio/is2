//! ----------------------------------------------------------------------------
//! Copyright Verizon.
//!
//! \file:    TODO
//! \details: TODO
//!
//! Licensed under the terms of the Apache 2.0 open source license.
//! Please refer to the LICENSE file in the project root for the terms.
//! ----------------------------------------------------------------------------
#ifndef _H_RESP_H
#define _H_RESP_H
namespace ns_is2 {
// ---------------------------------------
// Handler status
// ---------------------------------------
typedef enum {
        H_RESP_NONE = 0,
        H_RESP_DONE,
        H_RESP_DEFERRED,
        H_RESP_SERVER_ERROR,
        H_RESP_CLIENT_ERROR
} h_resp_t;
}
#endif

