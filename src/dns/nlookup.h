//! ----------------------------------------------------------------------------
//! Copyright Verizon.
//!
//! \file:    TODO
//! \details: TODO
//!
//! Licensed under the terms of the Apache 2.0 open source license.
//! Please refer to the LICENSE file in the project root for the terms.
//! ----------------------------------------------------------------------------
#ifndef _NLOOKUP_H
#define _NLOOKUP_H
//! ----------------------------------------------------------------------------
//! Includes
//! ----------------------------------------------------------------------------
#include <stdint.h>
#include <string>
namespace ns_is2 {
//! ----------------------------------------------------------------------------
//! Fwd Decl's
//! ----------------------------------------------------------------------------
struct host_info;
//! ----------------------------------------------------------------------------
//! Lookup inline
//! ----------------------------------------------------------------------------
int32_t nlookup(const std::string &a_host, uint16_t a_port, host_info &ao_host_info);
} //namespace ns_is2 {
#endif
