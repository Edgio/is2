//! ----------------------------------------------------------------------------
//! Copyright Edgecast Inc.
//!
//! \file:    TODO
//! \details: TODO
//!
//! Licensed under the terms of the Apache 2.0 open source license.
//! Please refer to the LICENSE file in the project root for the terms.
//! ----------------------------------------------------------------------------
//! ----------------------------------------------------------------------------
//! includes
//! ----------------------------------------------------------------------------
#include "is2/srvr/base_u.h"
#include "is2/evr/evr.h"
#include "is2/srvr/session.h"
namespace ns_is2 {
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
base_u::base_u(session &a_session):
               m_session(a_session),
               m_state(UPS_STATE_IDLE),
               m_shutdown(false)
{
}
} //namespace ns_is2 {
