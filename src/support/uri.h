//! ----------------------------------------------------------------------------
//! Copyright Verizon.
//!
//! \file:    TODO
//! \details: TODO
//!
//! Licensed under the terms of the Apache 2.0 open source license.
//! Please refer to the LICENSE file in the project root for the terms.
//! ----------------------------------------------------------------------------
// Uri encode and decode.
// RFC1630, RFC1738, RFC2396
//! ----------------------------------------------------------------------------
//! Includes
//! ----------------------------------------------------------------------------
#include <string>
//! ----------------------------------------------------------------------------
//! Prototypes
//! ----------------------------------------------------------------------------
namespace ns_is2
{
std::string uri_decode(const std::string & a_src);
std::string uri_encode(const std::string & a_src);
}
