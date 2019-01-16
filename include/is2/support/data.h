//: ----------------------------------------------------------------------------
//: Copyright (C) 2018 Verizon.  All Rights Reserved.
//: All Rights Reserved
//:
//: \file:    data.h
//: \details: TODO
//: \author:  Reed P. Morrison
//: \date:    07/05/2018
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
#ifndef _DATA_H
#define _DATA_H
//: ----------------------------------------------------------------------------
//: Includes
//: ----------------------------------------------------------------------------
#include <string>
#include <list>
#include <map>
#include <strings.h>
#include <stdint.h>
namespace ns_is2 {
//: ----------------------------------------------------------------------------
//: types
//: ----------------------------------------------------------------------------
struct case_i_comp
{
        bool operator() (const std::string& lhs, const std::string& rhs) const
        {
                return strcasecmp(lhs.c_str(), rhs.c_str()) < 0;
        }
};
typedef std::list <std::string> str_list_t;
typedef std::map <std::string, str_list_t, case_i_comp> kv_map_list_t;
//: ----------------------------------------------------------------------------
//: insert
//: ----------------------------------------------------------------------------
inline void kv_map_list_insert(kv_map_list_t &ao_kv_map_list,
                               const std::string &a_key,
                               const std::string &a_val)
{
        kv_map_list_t::iterator i_obj = ao_kv_map_list.find(a_key);
        if(i_obj != ao_kv_map_list.end())
        {
                i_obj->second.push_back(a_val);
        }
        else
        {
                str_list_t l_list;
                l_list.push_back(a_val);
                ao_kv_map_list[a_key] = l_list;
        }
}
// ---------------------------------------------------------
// data_t
// ---------------------------------------------------------
typedef struct _data {
        const char *m_data;
        uint32_t m_len;
        _data(void):
                m_data(NULL),
                m_len(0)
        {}
        void clear(void)
        {
                m_data = NULL;
                m_len = 0;
        }
} data_t;
typedef std::list <data_t> data_list_t;
// sorting...
struct data_case_i_comp
{
        bool operator()(const data_t& lhs, const data_t& rhs) const
        {
                uint32_t l_len = lhs.m_len > rhs.m_len ? rhs.m_len : lhs.m_len;
                return strncasecmp(lhs.m_data, rhs.m_data, l_len) < 0;
        }
};
typedef std::map <data_t, data_t, data_case_i_comp> data_map_t;
typedef std::map <data_t, data_list_t, data_case_i_comp> data_map_list_t;
// ---------------------------------------------------------
// mutable_data_t
// ---------------------------------------------------------
typedef struct _mutable_data {
        char *m_data;
        uint32_t m_len;
        _mutable_data(void):
                m_data(NULL),
                m_len(0)
        {}
        void clear(void)
        {
                m_data = NULL;
                m_len = 0;
        }
} mutable_data_t;
typedef std::list <mutable_data_t> mutable_data_list_t;
// sorting...
struct mutable_data_case_i_comp
{
        bool operator()(const mutable_data_t& lhs, const mutable_data_t& rhs) const
        {
                uint32_t l_len = lhs.m_len > rhs.m_len ? rhs.m_len : lhs.m_len;
                return strncasecmp(lhs.m_data, rhs.m_data, l_len) < 0;
        }
};
typedef std::map <mutable_data_t, mutable_data_list_t, mutable_data_case_i_comp> mutable_data_map_list_t;
// ---------------------------------------------------------
// find_first helper
// ---------------------------------------------------------
inline bool find_first(mutable_data_t &ao_val,
                       const mutable_data_map_list_t &a_map,
                       const char *a_key, uint32_t a_len)
{
        ao_val.m_data = NULL;
        ao_val.m_len = 0;
        mutable_data_t l_d;
        l_d.m_data = (char *)a_key;
        l_d.m_len = a_len;
        mutable_data_map_list_t::const_iterator i_h = a_map.find(l_d);
        if(i_h != a_map.end())
        {
                const mutable_data_t &l_v = i_h->second.front();
                ao_val = l_v;
                return true;
        }
        return false;
}
// ---------------------------------------------------------
// arg_t
// ---------------------------------------------------------
typedef struct _arg {
        const char *m_key;
        uint32_t m_key_len;
        const char *m_val;
        uint32_t m_val_len;
        void clear(void)
        {
                m_key = NULL;
                m_key_len = 0;
                m_val = NULL;
                m_val_len = 0;
        }
} arg_t;
typedef std::list <arg_t> arg_list_t;
// ---------------------------------------------------------
// mutable_arg_t
// ---------------------------------------------------------
typedef struct _mutable_arg {
        char *m_key;
        uint32_t m_key_len;
        char *m_val;
        uint32_t m_val_len;
        void clear(void)
        {
                m_key = NULL;
                m_key_len = 0;
                m_val = NULL;
                m_val_len = 0;
        }
} mutable_arg_t;
typedef std::list <mutable_arg_t> mutable_arg_list_t;
}
#endif
