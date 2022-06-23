//! ----------------------------------------------------------------------------
//! Copyright Edgio Inc.
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
#include "is2/support/os.h"
#include "is2/support/trace.h"
#include "is2/status.h"
namespace ns_is2 {
//! ----------------------------------------------------------------------------
//! \details: get os path
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t get_path(std::string &ao_path,
                 const std::string &a_route,
                 const std::string &a_url_path)
{
        // disallow traversal
        if((a_url_path.find("./") != std::string::npos) ||
           (a_url_path.find("../") != std::string::npos))
        {
                TRC_WARN("requested path with traversal -path: %s\n", a_url_path.c_str());
                return STATUS_ERROR;
        }
        std::string l_route = a_route;
        if(a_route[a_route.length() - 1] == '*')
        {
                l_route = a_route.substr(0, a_route.length() - 2);
        }
        if(l_route.empty())
        {
                ao_path = a_url_path;
        }
        else if(a_url_path.find(l_route, 0) != std::string::npos)
        {
                ao_path = a_url_path.substr(l_route.length(), a_url_path.length() - l_route.length());
        }
        else
        {
                TRC_WARN("requested path with missing route: %s\n", l_route.c_str());
                return STATUS_ERROR;
        }
        return STATUS_OK;
}
}
