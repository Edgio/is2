//: ----------------------------------------------------------------------------
//: Copyright (C) 2018 Verizon.  All Rights Reserved.
//: All Rights Reserved
//:
//: \file:    string_util.h
//: \details: TODO
//: \author:  Reed P. Morrison
//: \date:    02/07/2014
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
#ifndef _STRING_UTIL_H
#define _STRING_UTIL_H
//: ----------------------------------------------------------------------------
//: Includes
//: ----------------------------------------------------------------------------
#include <string>
#include <stdint.h>
#include <limits.h>
#include "data.h"
#include "is2/status.h"
namespace ns_is2 {
//: ----------------------------------------------------------------------------
//: Prototypes
//: ----------------------------------------------------------------------------
int32_t break_header_string(const std::string &a_header_str,
                            std::string &ao_header_key,
                            std::string &ao_header_val);
std::string get_file_wo_path(const std::string &a_filename);
std::string get_file_path(const std::string &a_filename);
std::string get_base_filename(const std::string &a_filename);
std::string get_file_ext(const std::string &a_filename);
std::string get_file_wo_ext(const std::string &a_filename);
int32_t convert_hex_to_uint(uint64_t &ao_val, const char *a_str);
int32_t parse_cookies(arg_list_t &ao_cookie_list, const char *a_buf, uint32_t a_len);
int32_t urldecode_ns(char **ao_buf, uint32_t &ao_len, uint32_t &ao_invalid_count, const char *a_buf, uint32_t a_len);
int32_t parse_args(mutable_arg_list_t &ao_arg_list, uint32_t &ao_invalid_cnt, const char *a_buf, uint32_t a_len, char a_arg_sep);
} //namespace ns_is2 {
#endif
