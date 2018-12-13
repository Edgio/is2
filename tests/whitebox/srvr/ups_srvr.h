//: ----------------------------------------------------------------------------
//: Copyright (C) 2018 Verizon.  All Rights Reserved.
//: All Rights Reserved
//:
//: \file:    ups_srvr.cc
//: \details: TODO
//: \author:  Reed P. Morrison
//: \date:    02/09/2017
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
#include <stdint.h>
//: ----------------------------------------------------------------------------
//: fwd decl's
//: ----------------------------------------------------------------------------
namespace ns_is2 {
        class srvr;
}
namespace ns_wb_ups_srvr {
//: ----------------------------------------------------------------------------
//: control
//: ----------------------------------------------------------------------------
int32_t run(void);
int32_t stop(void);
//: ----------------------------------------------------------------------------
//: externs
//: ----------------------------------------------------------------------------
extern ns_is2::srvr *g_srvr;
} // namespace ns_wb_ups_srvr {
