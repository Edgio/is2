//: ----------------------------------------------------------------------------
//: Copyright (C) 2018 Verizon.  All Rights Reserved.
//: All Rights Reserved
//:
//: \file:    base64.cc
//: \details: TODO
//: \author:  Reed P. Morrison
//: \date:    1/2/2016
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
//: ----------------------------------------------------------------------------
//: includes
//: ----------------------------------------------------------------------------
#include "is2/support/os.h"
#include "is2/support/trace.h"
#include "is2/status.h"
namespace ns_is2 {
//: ----------------------------------------------------------------------------
//: \details: get os path
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
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
