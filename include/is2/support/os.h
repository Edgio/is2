//: ----------------------------------------------------------------------------
//: Copyright (C) 2018 Verizon.  All Rights Reserved.
//: All Rights Reserved
//:
//: \file:    os.h
//: \details: TODO
//: \author:  Reed P. Morrison
//: \date:    2/7/2017
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
#ifndef _OS_H
#define _OS_H
//: ----------------------------------------------------------------------------
//: includes
//: ----------------------------------------------------------------------------
#include <stdint.h>
#include <string>
namespace ns_is2 {
//: ----------------------------------------------------------------------------
//: fwd decl's
//: ----------------------------------------------------------------------------
int32_t get_path(std::string &ao_path,
                 const std::string &a_route,
                 const std::string &a_url_path);
}
#endif
