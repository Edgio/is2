//! ----------------------------------------------------------------------------
//! Copyright Edgio Inc.
//!
//! \file:    TODO
//! \details: TODO
//!
//! Licensed under the terms of the Apache 2.0 open source license.
//! Please refer to the LICENSE file in the project root for the terms.
//! ----------------------------------------------------------------------------
#ifndef _JA3_H
#define _JA3_H
//! ----------------------------------------------------------------------------
//! includes
//! ----------------------------------------------------------------------------
#include <stdint.h>
#include <string>
#include <list>
//! ----------------------------------------------------------------------------
//! fwd decl's
//! ----------------------------------------------------------------------------
typedef struct ssl_st SSL;
namespace ns_is2 {
//! ----------------------------------------------------------------------------
//! types
//! ----------------------------------------------------------------------------
typedef std::list<uint32_t> uint16_list_t;
//! ----------------------------------------------------------------------------
//! ja3
//! ----------------------------------------------------------------------------
class ja3
{
public:
        // -------------------------------------------------
        // public methods
        // -------------------------------------------------
        ja3(void):
                m_fp_ssl_version(0),
                m_fp_cipher_list(0),
                m_fp_ssl_ext_list(0),
                m_fp_ec_curve_list(0),
                m_fp_ec_pt_format_list(0)
        {}
        ~ja3() {}
        void reset(void)
        {
                m_fp_ssl_version = 0;
                m_fp_cipher_list.clear();
                m_fp_ssl_ext_list.clear();
                m_fp_ec_curve_list.clear();
                m_fp_ec_pt_format_list.clear();
                m_str.clear();
                m_md5.clear();
        }
        int32_t extract_fp(SSL* a_ssl);
        const std::string& get_str(void);
        const std::string& get_md5(void);
        // -------------------------------------------------
        // public members
        // -------------------------------------------------
        // -------------------------------------------------
        // *************************************************
        // properties
        // *************************************************
        // -------------------------------------------------
        uint16_t m_fp_ssl_version;
        uint16_list_t m_fp_cipher_list;
        uint16_list_t m_fp_ssl_ext_list;
        uint16_list_t m_fp_ec_curve_list;
        uint16_list_t m_fp_ec_pt_format_list;
private:
        // -------------------------------------------------
        // private methods
        // -------------------------------------------------
        // Disallow copy/assign
        ja3& operator=(const ja3 &);
        ja3(const ja3 &);
        int32_t extract_bytes(const char* a_buf, uint16_t a_len);
        // -------------------------------------------------
        // private members
        // -------------------------------------------------
        std::string m_str;
        std::string m_md5;
};
} //namespace ns_is2 {
#endif

