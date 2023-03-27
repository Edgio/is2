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
// ---------------------------------------------------------
// is2
// ---------------------------------------------------------
#include "is2/support/ja3.h"
#include "is2/support/trace.h"
#include "is2/support/ndebug.h"
// ---------------------------------------------------------
// openssl
// ---------------------------------------------------------
#include <openssl/opensslv.h>
#include <openssl/ssl.h>
#include <openssl/evp.h>
#include <openssl/err.h>
// ---------------------------------------------------------
// std system includes
// ---------------------------------------------------------
#include <string.h>
#include <arpa/inet.h>
// ---------------------------------------------------------
// std c++ libs
// ---------------------------------------------------------
#include <set>
#include <sstream>
//! ----------------------------------------------------------------------------
//! constants
//! ----------------------------------------------------------------------------
#ifndef STATUS_OK
#define STATUS_OK 0
#endif
#ifndef STATUS_ERROR
#define STATUS_ERROR -1
#endif
//! ----------------------------------------------------------------------------
//! macros
//! ----------------------------------------------------------------------------
#ifndef NDBG_OUTPUT
#define NDBG_OUTPUT(...) \
        do { \
                fprintf(stdout, __VA_ARGS__); \
                fflush(stdout); \
        } while(0)
#endif
#ifndef NDBG_PRINT
#define NDBG_PRINT(...) \
        do { \
                fprintf(stdout, "%s:%s.%d: ", __FILE__, __FUNCTION__, __LINE__); \
                fprintf(stdout, __VA_ARGS__);               \
                fflush(stdout); \
        } while(0)
#endif
//! ----------------------------------------------------------------------------
//! grease code checking
//! ref: https://tools.ietf.org/id/draft-ietf-tls-grease-01.html
//! ----------------------------------------------------------------------------
#define IS_GREASE_VAL(code) (((code)&0x0f0f) == 0x0a0a && ((code)&0xff) == ((code)>>8))
//! ----------------------------------------------------------------------------
//! \details: convert to string
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
template <typename T>
static std::string _to_string(const T& a_num)
{
        std::stringstream l_s;
        l_s << a_num;
        return l_s.str();
}
//! ----------------------------------------------------------------------------
//! md5 hasher obj
//! ----------------------------------------------------------------------------
class md5
{
public:
        // -------------------------------------------------
        // public constants
        // -------------------------------------------------
        static const uint16_t s_hash_len = 16;
        // -------------------------------------------------
        // public methods
        // -------------------------------------------------
        // -------------------------------------------------
        // constructor
        // -------------------------------------------------
        md5():
                m_ctx(nullptr),
                m_finished(false),
                m_hash_hex()
        {
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
                m_ctx = EVP_MD_CTX_new();
#else
                m_ctx = EVP_MD_CTX_create();
#endif
                EVP_DigestInit_ex(m_ctx, EVP_md5(), nullptr);
        }
        // -------------------------------------------------
        // destructor
        // -------------------------------------------------
        ~md5()
        {
                if (nullptr != m_ctx)
                {
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
                        EVP_MD_CTX_free(m_ctx);
#else
                        EVP_MD_CTX_destroy(m_ctx);
#endif
                }
        }
        // -------------------------------------------------
        // update
        // -------------------------------------------------
        void update(const char* a_str, unsigned int a_len)
        {
                EVP_DigestUpdate(m_ctx, (const unsigned char*)a_str, a_len);
        }
        // -------------------------------------------------
        // finish
        // -------------------------------------------------
        void finish()
        {
                if(m_finished)
                {
                        return;
                }
                EVP_DigestFinal_ex(m_ctx, (unsigned char *)m_hash, nullptr);
                static const char s_hexchars[] =
                {
                        '0', '1', '2', '3',
                        '4', '5', '6', '7',
                        '8', '9', 'a', 'b',
                        'c', 'd', 'e', 'f'
                };
                for(size_t i = 0; i < s_hash_len; ++i)
                {
                        m_hash_hex[2 * i + 0] = s_hexchars[(m_hash[i] & 0xf0) >> 4];
                        m_hash_hex[2 * i + 1] = s_hexchars[m_hash[i] & 0x0f];
                }
                m_hash_hex[32] = '\0';
                m_finished = true;
        }
        // -------------------------------------------------
        // get_hash_hex
        // -------------------------------------------------
        const char* get_hash_hex()
        {
                finish();
                return m_hash_hex;
        }
        // -------------------------------------------------
        // get_hash
        // -------------------------------------------------
        const unsigned char* get_hash()
        {
                finish();
                return m_hash;
        }
private:
        // -------------------------------------------------
        // private methods
        // -------------------------------------------------
        md5(const md5&);
        md5& operator=(const md5&);
        // -------------------------------------------------
        // private members
        // -------------------------------------------------
        EVP_MD_CTX* m_ctx;
        bool m_finished;
        unsigned char m_hash[s_hash_len];
        char m_hash_hex[33];
};
namespace ns_is2 {
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t ja3::extract_bytes(const char* a_buf, uint16_t a_len)
{
        // -------------------------------------------------
        // *************************************************
        // check for handshake+hello
        // *************************************************
        // -------------------------------------------------
        if (a_len < 5)
        {
                return STATUS_OK;
        }
#define _TLS_HANDSHAKE_RECORD 0x16
#define _TLS_CLIENT_HELLO     0x01
        uint16_t l_idx = 0;
        while (l_idx < a_len) {
                if (((uint16_t)(a_buf[l_idx]) == _TLS_HANDSHAKE_RECORD) &&
                    ((uint16_t)(a_buf[l_idx+5]) == _TLS_CLIENT_HELLO))
                {
                        break;
                }
                ++l_idx;
        }
        if (l_idx >= a_len) {
                return STATUS_OK;
        }
        // -------------------------------------------------
        // extract
        // -------------------------------------------------
        const char* l_cur = a_buf+l_idx;
        uint16_t l_read = l_idx;
        int32_t l_left = a_len-l_idx;
        // -------------------------------------------------
        // *************************************************
        // record header
        // *************************************************
        // -------------------------------------------------
#define _INCR_BY(_val) do { \
        l_cur+=_val; \
        l_read+=_val; \
        l_left-=_val; \
        if (l_left <= 0) { return STATUS_ERROR; }\
} while(0)
#define _TLS_RECORD_HEADER_SIZE 5
        if (l_left < _TLS_RECORD_HEADER_SIZE)
        {
                return STATUS_ERROR;
        }
        //uint8_t l_rh_type = (uint8_t)(*l_cur); _INCR_BY(1);
        //uint16_t l_rh_p_ver = ntohs(*((const uint16_t*)(l_cur))); _INCR_BY(2);
        //uint16_t l_rh_msg_type = ntohs(*((const uint16_t*)(l_cur))); _INCR_BY(2);
        _INCR_BY(5);
        // -------------------------------------------------
        // *************************************************
        // record header
        // *************************************************
        // -------------------------------------------------
#define _TLS_HANDSHAKE_HEADER_SIZE 4
        if (l_left < _TLS_HANDSHAKE_HEADER_SIZE)
        {
                return STATUS_ERROR;
        }
        uint8_t l_hs_type = (uint8_t)(*l_cur); _INCR_BY(1);
        uint32_t l_hs_len = 0;
        l_hs_len = (l_hs_len << 8) + (uint8_t)(*l_cur); _INCR_BY(1);
        l_hs_len = (l_hs_len << 8) + (uint8_t)(*l_cur); _INCR_BY(1);
        l_hs_len = (l_hs_len << 8) + (uint8_t)(*l_cur); _INCR_BY(1);
        if (l_hs_type != _TLS_CLIENT_HELLO)
        {
                return STATUS_OK;
        }
        // -------------------------------------------------
        // *************************************************
        // client version
        // *************************************************
        // -------------------------------------------------
#define _TLS_CLIENT_VERSION_SIZE 2
        if (l_left < _TLS_CLIENT_VERSION_SIZE)
        {
                return STATUS_ERROR;
        }
        uint16_t l_clnt_ver = ntohs(*((const uint16_t*)(l_cur))); _INCR_BY(2);
        m_fp_ssl_version = l_clnt_ver;
        // -------------------------------------------------
        // *************************************************
        // client random
        // *************************************************
        // -------------------------------------------------
        _INCR_BY(32);
        // -------------------------------------------------
        // *************************************************
        // session id
        // *************************************************
        // -------------------------------------------------
        uint8_t l_sid_len = (uint8_t)(*l_cur); _INCR_BY(1);
        _INCR_BY(l_sid_len);
        // -------------------------------------------------
        // *************************************************
        // cipher suites
        // *************************************************
        // -------------------------------------------------
        uint16_t l_cs_len = (ntohs(*((const uint16_t*)(l_cur))))/2; _INCR_BY(2);
        for (uint16_t i_cs = 0; i_cs < l_cs_len; ++i_cs)
        {
                uint16_t l_cs = ntohs(*((const uint16_t*)(l_cur))); _INCR_BY(2);
                m_fp_cipher_list.push_back(l_cs);
        }
        // -------------------------------------------------
        // *************************************************
        // compression methods
        // *************************************************
        // -------------------------------------------------
        uint8_t l_cs_mt_len = (uint8_t)(*l_cur); _INCR_BY(1);
        _INCR_BY(l_cs_mt_len);
        // -------------------------------------------------
        // *************************************************
        // extensions
        // *************************************************
        // -------------------------------------------------
        if (l_left <= 0)
        {
                return STATUS_OK;
        }
        uint16_t l_exts_len = ntohs(*((const uint16_t*)(l_cur))); _INCR_BY(2);
        // -------------------------------------------------
        // for each extension
        // -------------------------------------------------
#define _INCR_EXT_BY(_val) do { \
        l_cur+=_val; \
        l_read+=_val; \
        l_left-=_val; \
        l_ext_left-=_val; \
        if (l_left < 0) { return STATUS_OK; }\
        if (l_ext_left <= 0) { break; } \
} while(0)
        uint16_t l_ext_idx = 0;
        uint16_t l_ext_left = l_exts_len;
        while (l_ext_left)
        {
                uint16_t l_ext_type = ntohs(*((const uint16_t*)(l_cur))); _INCR_EXT_BY(2);
                uint16_t l_cur_ext_len = ntohs(*((const uint16_t*)(l_cur))); _INCR_EXT_BY(2);
                m_fp_ssl_ext_list.push_back(l_ext_type);
                // -----------------------------------------
                // *****************************************
                // extensions: elliptic point format
                // *****************************************
                // -----------------------------------------
                if (l_ext_type == 0x000B)
                {
                        uint8_t l_ec_pf_len = (uint8_t)(*l_cur); _INCR_EXT_BY(1);
                        for (int i_pf = 0; i_pf < l_ec_pf_len; ++i_pf)
                        {
                                uint8_t l_ec_pf = (uint8_t)(*l_cur); _INCR_EXT_BY(1);
                                m_fp_ec_pt_format_list.push_back(l_ec_pf);
                        }
                        if (l_left <= 0) { return STATUS_OK; }
                        if (l_ext_left <= 0) { break; }
                }
                // -----------------------------------------
                // *****************************************
                // extensions: elliptic curve groups
                // *****************************************
                // -----------------------------------------
                else if (l_ext_type == 0x000A)
                {
                        uint16_t l_ecg_len = (ntohs(*((const uint16_t*)(l_cur))))/2; _INCR_EXT_BY(2);
                        for (int i_ecg = 0; i_ecg < l_ecg_len; ++i_ecg)
                        {
                                uint16_t l_ecg = ntohs(*((const uint16_t*)(l_cur))); _INCR_EXT_BY(2);
                                m_fp_ec_curve_list.push_back(l_ecg);
                        }
                }
                // -----------------------------------------
                // extension padding
                // -----------------------------------------
                else if (l_ext_type == 0x0015)
                {
                        break;
                }
                // -----------------------------------------
                // else skip by ext len
                // -----------------------------------------
                else
                {
                        _INCR_EXT_BY(l_cur_ext_len);
                }
                ++l_ext_idx;
        }
        // -------------------------------------------------
        // done
        // -------------------------------------------------
        return STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t ja3::extract_fp(SSL* a_ssl)
{
        if (m_fp_ssl_version != 0)
        {
                return STATUS_OK;
        }
        // -------------------------------------------------
        // sanity checking
        // -------------------------------------------------
        if (!a_ssl)
        {
                return STATUS_ERROR;
        }
        // -------------------------------------------------
        // get fd
        // -------------------------------------------------
        int l_fd = -1;
        l_fd = SSL_get_fd(a_ssl);
        if (l_fd == -1)
        {
                return STATUS_ERROR;
        }
        // -------------------------------------------------
        // peek read buffer
        // -------------------------------------------------
        int l_peek_size = 0;
        char l_buf[512];
        l_peek_size = recvfrom(l_fd, l_buf, 512, MSG_PEEK, NULL, NULL);
        if (l_peek_size <= 0)
        {
                return STATUS_ERROR;
        }
        // -------------------------------------------------
        // extract
        // -------------------------------------------------
        int32_t l_s;
        l_s = extract_bytes(l_buf, (uint16_t)l_peek_size);
        if (l_s != STATUS_OK)
        {
                return STATUS_ERROR;
        }
        return STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
const std::string& ja3::get_str(void)
{
        if (!m_str.empty())
        {
            return m_str;
        }
        // -------------------------------------------------
        // if unset just return empty str
        // -------------------------------------------------
        if (!m_fp_ssl_version)
        {
            return m_str;
        }
        // -------------------------------------------------
        // helper to print a list
        // -------------------------------------------------
#define _PRINT_LIST(_list) do { \
        for (auto & i_c : _list) { \
                if (IS_GREASE_VAL(i_c)) { continue; } \
                m_str += _to_string(i_c); \
                if (&i_c != &_list.back()) { \
                        m_str += "-"; \
                } } } while(0)
        // -------------------------------------------------
        // ja3_fingerprint
        // -------------------------------------------------
        // -------------------------------------------------
        // ref: https://github.com/salesforce/ja3
        // field order is as follows:
        //   SSLVersion,Cipher,SSLExtension,EllipticCurve,EllipticCurvePointFormat
        // -------------------------------------------------
        // -------------------------------------------------
        // SSLVersion
        // -------------------------------------------------
        m_str += _to_string(m_fp_ssl_version);
        m_str += ",";
        // -------------------------------------------------
        // Cipher
        // -------------------------------------------------
        _PRINT_LIST(m_fp_cipher_list);
        m_str += ",";
        // -------------------------------------------------
        // SSLExtension
        // -------------------------------------------------
        _PRINT_LIST(m_fp_ssl_ext_list);
        m_str += ",";
        // -------------------------------------------------
        // EllipticCurve
        // -------------------------------------------------
        _PRINT_LIST(m_fp_ec_curve_list);
        m_str += ",";
        // -------------------------------------------------
        // EllipticCurvePointFormat
        // -------------------------------------------------
        _PRINT_LIST(m_fp_ec_pt_format_list);
        // -------------------------------------------------
        // done
        // -------------------------------------------------
        return m_str;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
const std::string& ja3::get_md5(void)
{
        if (!m_md5.empty())
        {
            return m_md5;
        }
        // -------------------------------------------------
        // if unset just return empty str
        // -------------------------------------------------
        if (!m_fp_ssl_version)
        {
                return m_md5;
        }
        // -------------------------------------------------
        // get string
        // -------------------------------------------------
        const std::string l_str = get_str();
        // -------------------------------------------------
        // hash
        // -------------------------------------------------
        md5 l_md5;
        l_md5.update(l_str.c_str(), l_str.length());
        l_md5.finish();
        m_md5 = l_md5.get_hash_hex();
        // -------------------------------------------------
        // done
        // -------------------------------------------------
        return m_md5;
}
}
