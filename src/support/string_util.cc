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
//! Includes
//! ----------------------------------------------------------------------------
#include "is2/support/string_util.h"
#include "is2/support/ndebug.h"
#include <stdlib.h>
#include <string.h>
//! ----------------------------------------------------------------------------
//! macros
//! ----------------------------------------------------------------------------
#define VALID_HEX(X) \
        (((X >= '0') && (X <= '9')) || \
         ((X >= 'a') && (X <= 'f')) || \
         ((X >= 'A') && (X <= 'F')))
namespace ns_is2 {
//! ----------------------------------------------------------------------------
//! statics
//! ----------------------------------------------------------------------------
//! ----------------------------------------------------------------------------
//! \details: Converts a byte given as its hexadecimal representation into a
//!           proper byte. Handles uppercase and lowercase letters but does not
//!           check for overflows.
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
static unsigned char x2c(const unsigned char *a_nbl)
{
        unsigned char l_c;
        l_c =  ((a_nbl[0] >= 'A') ?
                                   (((a_nbl[0] & 0xdf) - 'A') + 10) :
                                   (a_nbl[0] - '0'));
        l_c *= 16;
        l_c += ((a_nbl[1] >= 'A') ?
                                   (((a_nbl[1] & 0xdf) - 'A') + 10) :
                                   (a_nbl[1] - '0'));
        return l_c;
}
//! ----------------------------------------------------------------------------
//! insert
//! ----------------------------------------------------------------------------
#if 0
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
#endif
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t break_header_string(const std::string &a_header_str,
		std::string &ao_header_key,
		std::string &ao_header_val)
{
	// Find port prefix ":"
	size_t l_colon_pos = 0;
	l_colon_pos = a_header_str.find(":", 0);
	if(l_colon_pos == std::string::npos)
	{
		return -1;
	}
	ao_header_key = a_header_str.substr(0, l_colon_pos);
	++l_colon_pos;
	// ignore spaces...
	while(a_header_str[l_colon_pos] == ' ') ++l_colon_pos;
	ao_header_val = a_header_str.substr(l_colon_pos, a_header_str.length());
	return 0;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
std::string get_file_wo_path(const std::string &a_filename)
{
        std::string fName(a_filename);
        size_t pos = fName.rfind("/");
        if(pos == std::string::npos)
        {
                return fName;
        }
        if(pos == 0)
        {
                return fName;
        }
        return fName.substr(pos + 1, fName.length());
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
std::string get_file_path(const std::string &a_filename)
{
        std::string fName(a_filename);
        size_t pos = fName.rfind("/");
        if(pos == std::string::npos)
        {
                return fName;
        }
        if(pos == 0)
        {
                return fName;
        }
        return fName.substr(0, pos);
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
std::string get_base_filename(const std::string &a_filename)
{
        std::string fName(a_filename);
        size_t pos = fName.rfind(".");
        if(pos == std::string::npos)
        {
                return fName;
        }
        if(pos == 0)
        {
                return fName;
        }
        return fName.substr(0, pos);
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
std::string get_file_ext(const std::string &a_filename)
{
        std::string fName(a_filename);
        size_t pos = fName.rfind(".");
        if(pos == std::string::npos)
        {
                return std::string();
        }
        if(pos == 0)
        {
                return std::string();
        }
        return fName.substr(pos + 1, fName.length());
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
std::string get_file_wo_ext(const std::string &a_filename)
{
        std::string fName(a_filename);
        size_t pos = fName.rfind(".");
        if(pos == std::string::npos)
        {
                return std::string();
        }
        if(pos == 0)
        {
                return std::string();
        }
        return fName.substr(0, pos);
}
//! ----------------------------------------------------------------------------
//! \details TODO
//! \return: TODO
//! \param:  TODO
//! ----------------------------------------------------------------------------
int32_t convert_hex_to_uint(uint64_t &ao_val, const char *a_str)
{
        ao_val = strtoull(a_str, NULL, 16);
        if((ao_val == ULLONG_MAX) ||
           (ao_val == 0))
        {
                ao_val = 0;
                return STATUS_ERROR;
        }
        return STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: url decode -non strict ???
//! \return:  TODO
//! \param:   TODO
//! \notes:   IMP1 Assumes NUL-terminated
//! ----------------------------------------------------------------------------
int32_t urldecode_ns(char **ao_buf,
                     uint32_t &ao_len,
                     uint32_t &ao_invalid_count,
                     const char *a_buf,
                     uint32_t a_len)
{
        // -------------------------------------------------
        // check exist
        // -------------------------------------------------
        if(!a_buf ||
           !a_len)
        {
                // TODO -log reason???
                return STATUS_ERROR;
        }
        uint32_t i_char = 0;
        uint32_t l_count = 0;
        char *l_buf = (char *)malloc(sizeof(char)*a_len + 1);
        memcpy(l_buf, a_buf, a_len);
        l_buf[a_len] = '\0';;
        char *l_d = (char *)l_buf;
        while(i_char < a_len)
        {
                // -----------------------------------------
                // encoding...
                // -----------------------------------------
                if(l_buf[i_char] == '%')
                {
                        // ---------------------------------
                        // enough bytes available???
                        // ---------------------------------
                        if(i_char + 2 < a_len)
                        {
                                char l_c1 = l_buf[i_char + 1];
                                char l_c2 = l_buf[i_char + 2];
                                if(VALID_HEX(l_c1) &&
                                   VALID_HEX(l_c2))
                                {
                                        // Valid encoding - decode it.
                                        *l_d = x2c((unsigned char *)(&l_buf[i_char + 1]));
                                        ++l_d;
                                        ++l_count;
                                        i_char += 3;
                                }
                                else
                                {
                                        // Not a valid encoding, skip this %
                                        *l_d = l_buf[i_char];
                                        ++l_d;
                                        ++i_char;
                                        ++l_count;
                                        ++ao_invalid_count;
                                }
                        }
                        // ---------------------------------
                        // not enough bytes available, copy
                        // raw bytes.
                        // ---------------------------------
                        else
                        {
                                // Not enough bytes available, copy the raw bytes.
                                *l_d = l_buf[i_char];
                                ++l_d;
                                ++i_char;
                                ++l_count;
                                ++ao_invalid_count;
                        }
                }
                // -----------------------------------------
                // not encoding maker
                // -----------------------------------------
                else
                {
                        if(l_buf[i_char] == '+')
                        {
                                *l_d = ' ';
                        }
                        else
                        {
                                *l_d = l_buf[i_char];
                        }
                        ++l_d;
                        ++i_char;
                        ++l_count;
                }
        }
        // -------------------------------------------------
        // terminate...
        // -------------------------------------------------
        *l_d = '\0';
        // -------------------------------------------------
        // done...
        // -------------------------------------------------
        ao_len = l_count;
        *ao_buf = l_buf;
        return STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t parse_args(mutable_arg_list_t &ao_arg_list,
                   uint32_t &ao_invalid_cnt,
                   const char *a_buf,
                   uint32_t a_buf_len,
                   char a_arg_sep)
{
        //NDBG_PRINT("a_buf:     %p\n", a_buf);
        //NDBG_PRINT("a_buf_len: %d\n", (int)a_buf_len);
        // -------------------------------------------------
        // TODO -make zero copy impl
        // -------------------------------------------------
        if(!a_buf ||
           !a_buf_len)
        {
                // TODO log reason???
                // No query string in request
                return STATUS_OK;
        }
        char *l_buf;
        l_buf = (char *)malloc(a_buf_len + 1);
        if(!l_buf)
        {
                // TODO log reason???
                return STATUS_ERROR;
        }
        char *l_val = NULL;
        int32_t l_status = 0;
        uint32_t i_char = 0;
        uint32_t i_w_off = 0;
        uint32_t l_key_orig_len = 0;
        uint32_t l_val_orig_len = 0;
        mutable_arg_t l_arg;
        ao_invalid_cnt = 0;
        // -------------------------------------------------
        // parse buffer char by char
        // -------------------------------------------------
        while(i_char < a_buf_len)
        {
                // -----------------------------------------
                // parameter name
                // -----------------------------------------
                // Special case if there is no param
                // e.g ?&a=b&&&c=d
                // status = 0 means a param key
                if(a_buf[i_char] == a_arg_sep &&
                   l_status == 0)
                {
                        ++i_char;
                        continue;
                }
                if(l_status == 0)
                {
                        uint32_t l_key_off = i_char;
                        while((i_char < a_buf_len) &&
                              (a_buf[i_char] != '=') &&
                              (a_buf[i_char] != a_arg_sep))
                        {
                                l_buf[i_w_off] = a_buf[i_char];
                                ++i_w_off;
                                ++i_char;
                        }
                        l_buf[i_w_off++] = '\0';
                        l_key_orig_len = i_char - l_key_off;
                }
                // -----------------------------------------
                // parameter value
                // -----------------------------------------
                else
                {
                        uint32_t l_val_off = i_char;
                        while((i_char < a_buf_len) &&
                              (a_buf[i_char] != a_arg_sep))
                        {
                                l_buf[i_w_off] = a_buf[i_char];
                                ++i_w_off;
                                ++i_char;
                        }
                        l_buf[i_w_off++] = '\0';
                        l_val_orig_len = i_char - l_val_off;
                }
                // -----------------------------------------
                //
                // -----------------------------------------
                if(l_status == 0)
                {
                        //NDBG_PRINT("decode: %.*s\n", (int)l_key_orig_len, l_buf);
                        //Empty key, set it to null
                        if(!l_key_orig_len)
                        {
                                l_arg.m_key = NULL;
                                l_arg.m_key_len = 0;
                        }
                        int32_t l_s;
                        l_s = urldecode_ns(&(l_arg.m_key),
                                           l_arg.m_key_len,
                                           ao_invalid_cnt,
                                           l_buf,
                                           l_key_orig_len);
                        UNUSED(l_s);
                        // TODO -check for error
                        if((i_char < a_buf_len) &&
                           (a_buf[i_char] == a_arg_sep))
                        {
                                // Empty parameter
                                l_arg.m_val = NULL;
                                l_arg.m_val_len = 0;
                                //NDBG_PRINT("%.*s: %.*s\n", l_arg.m_key_len, l_arg.m_key, l_arg.m_val_len, l_arg.m_val);
                                ao_arg_list.push_back(l_arg);
                                l_arg.clear();
                                // unchanged
                                l_status = 0;
                                i_w_off = 0;
                        }
                        else
                        {
                                l_status = 1;
                                l_val = &l_buf[i_w_off];
                        }
                }
                else
                {
                        //NDBG_PRINT("decode: %.*s\n", (int)l_val_orig_len, l_val);
                        if(!l_val ||
                           !l_val_orig_len)
                        {
                               // Empty parameter
                                l_arg.m_val = NULL;
                                l_arg.m_val_len = 0;
                                //NDBG_PRINT("%.*s: %.*s\n", l_arg.m_key_len, l_arg.m_key, l_arg.m_val_len, l_arg.m_val);
                                ao_arg_list.push_back(l_arg);
                                l_arg.clear();
                                // unchanged
                                l_status = 0;
                                i_w_off = 0;
                        }
                        else
                        {
                                int32_t l_s;
                                l_s = urldecode_ns(&(l_arg.m_val),
                                                   l_arg.m_val_len,
                                                   ao_invalid_cnt,
                                                   l_val,
                                                   l_val_orig_len);
                                UNUSED(l_s);
                                // TODO -check for error
                                //NDBG_PRINT("%.*s: %.*s\n", l_arg.m_key_len, l_arg.m_key, l_arg.m_val_len, l_arg.m_val);
                                ao_arg_list.push_back(l_arg);
                                l_arg.clear();
                                l_status = 0;
                                i_w_off = 0;
                        }
                }
                // skip over the separator
                ++i_char;
        }
        // -------------------------------------------------
        // last parameter empty
        // -------------------------------------------------
        if(l_status == 1)
        {
                l_arg.m_val = NULL;
                l_arg.m_val_len = 0;
                //NDBG_PRINT("%.*s: %.*s\n", l_arg.m_key_len, l_arg.m_key, l_arg.m_val_len, l_arg.m_val);
                ao_arg_list.push_back(l_arg);

        }
        if(l_buf) { free(l_buf); l_buf = NULL;}
        return STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details parse cookie string:
//!          format: 'key1=val1; key2; key3=val3; key4\0'
//! \return  TODO
//! \param   TODO
//! ----------------------------------------------------------------------------
static bool is_char_in_set(const char *a_arr, uint32_t a_arr_len, char a_char)
{
        for(uint32_t i_c = 0; i_c < a_arr_len; ++i_c)
        {
                if(a_char == a_arr[i_c]) return true;
        }
        return false;
}
//! ----------------------------------------------------------------------------
//! \details parse cookie string:
//!          format: 'key1=val1; key2; key3=val3; key4\0'
//! \return  TODO
//! \param   TODO
//! ----------------------------------------------------------------------------
int32_t parse_cookies(arg_list_t &ao_cookie_list,
                      const char *a_buf,
                      uint32_t a_buf_len)
{
        static const char l_del_set[]    = {'=',';',' ','\t','\f','\r','\n'};
        static const char l_valdel_set[] = {'=',' ','\t','\f','\r','\n'};
        // -------------------------------------------------
        // Parsing logic
        // -------------------------------------------------
        // 1: Skip delimiters
        // 2: Match until ';' or '\0' for key
        // 3: If '=' found, skip to first non-delimiter char
        // 4: Look for value until either ';' or '\0'
        // 5: Back to step 1
        // Example 'cookie: abc= =123  ;def;;;'
        //  - key='abc', val='123'
        //  - key='def', val=''
        // RFC: http://tools.ietf.org/html/rfc6265#section-4.1
        // -------------------------------------------------
        // TODO !!!
        // trim trailing whitespace(s) from values...
        // in ex above -cookie split results in
        //  - key='abc', val='123  '
        // -------------------------------------------------
        // start at first non-delimiter char
        // -------------------------------------------------
        const char *l_key = a_buf;
        const char *l_val=NULL;
        const char *l_keyend=NULL;
        //NDBG_PRINT("SKIP l_del_chars\n");
        for(; is_char_in_set(l_del_set, sizeof(l_del_set), *l_key); ++l_key) {}
        if (*l_key == '\0') return STATUS_OK;
        //NDBG_PRINT("l_key: %s\n", l_key);
        // -------------------------------------------------
        // NOTE: assume \0 terminated string
        // -------------------------------------------------
        arg_t l_arg;
        for(const char* i_p = l_key + 1; ; ++i_p)
        {
                //NDBG_PRINT("i_p: %s\n", i_p);
                switch (*i_p)
                {
                // -----------------------------------------
                // \0
                // -----------------------------------------
                case '\0':
                {
                        if(l_val)
                        {
                                // we got "key=value; "
                                l_arg.m_key = l_key;
                                l_arg.m_key_len = l_keyend - l_key;
                                int l_len = (int)(i_p - l_val);
                                const char *l_p_i = i_p - 1;
                                while(l_len && *l_p_i == ' ') { --l_len; --l_p_i; }
                                l_arg.m_val = l_val;
                                l_arg.m_val_len = l_len;
                                //NDBG_PRINT("l_key: \"%s\"\n", l_key_str.c_str());
                                //NDBG_PRINT("l_val: \"%s\"\n", l_val_str.c_str());
                                ao_cookie_list.push_back(l_arg);
                                l_arg.clear();
                        }
                        else
                        {
                                // we got a key with no value
                                l_arg.m_key = l_key;
                                l_arg.m_key_len = i_p - l_key;
                                l_arg.m_val = NULL;
                                l_arg.m_val_len = 0;
                                ao_cookie_list.push_back(l_arg);
                                l_arg.clear();
                        }
                        return STATUS_OK;
                }
                // -----------------------------------------
                // =
                // -----------------------------------------
                case '=':
                {
                        if (l_val) break;
                        // ---------------------------------
                        // mark end of key and jump to
                        // next non-delimiter character
                        // ---------------------------------
                        l_keyend = i_p++;
                        //NDBG_PRINT("SKIP l_valdel_chars\n");
                        for(; is_char_in_set(l_valdel_set, sizeof(l_valdel_set), *i_p); ++i_p) {}
                        if (*i_p == '\0') return STATUS_OK;
                        if (*i_p != ';')
                        {
                                l_val = i_p;
                                break;
                        }
                        // fall-thru
                }
                // -----------------------------------------
                // ;
                // -----------------------------------------
                case ';':
                {
                        if(l_val)
                        {
                                // we got "key=value;"
                                l_arg.m_key = l_key;
                                l_arg.m_key_len = l_keyend - l_key;
                                int l_len = (int)(i_p - l_val);
                                const char *l_p_i = i_p - 1;
                                while(l_len && *l_p_i == ' ') { --l_len; --l_p_i; }
                                l_arg.m_val = l_val;
                                l_arg.m_val_len = l_len;
                                //NDBG_PRINT("l_key: \"%s\"\n", l_key_str.c_str());
                                //NDBG_PRINT("l_val: \"%s\"\n", l_val_str.c_str());
                                ao_cookie_list.push_back(l_arg);
                                l_arg.clear();
                        }
                        else
                        {
                                // we got a key with no value
                                l_arg.m_key = l_key;
                                l_arg.m_key_len = i_p - l_key;
                                l_arg.m_val = NULL;
                                l_arg.m_val_len = 0;
                                ao_cookie_list.push_back(l_arg);
                                l_arg.clear();
                        }
                        // jump to next non-delimiter char
                        ++i_p;
                        //NDBG_PRINT("SKIP l_del_chars\n");
                        for(; is_char_in_set(l_del_set, sizeof(l_del_set), *i_p); ++i_p) {}
                        if (*i_p == '\0') return STATUS_OK;
                        l_key = i_p;
                        l_val = NULL;
                        l_keyend = NULL;
                }
                }
        }
        return STATUS_OK;
}
} //namespace ns_is2 {
