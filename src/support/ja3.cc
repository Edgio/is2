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
//! ----------------------------------------------------------------------------
#define IS_GREASE_CODE(code) (((code)&0x0f0f) == 0x0a0a && ((code)&0xff) == ((code)>>8))
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
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
static bool _is_ext_greased(uint16_t a_ext)
{
        static std::set<uint16_t> s_grease_ext = {
                        0x0a0a,
                        0x1a1a,
                        0x2a2a,
                        0x3a3a,
                        0x4a4a,
                        0x5a5a,
                        0x6a6a,
                        0x7a7a,
                        0x8a8a,
                        0x9a9a,
                        0xaaaa,
                        0xbaba,
                        0xcaca,
                        0xdada,
                        0xeaea,
                        0xfafa,
        };
        if (s_grease_ext.find(a_ext) != s_grease_ext.end())
        {
                return true;
        }
        return false;
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
int32_t ja3::extract_fp(SSL* a_ssl)
{
        // -------------------------------------------------
        // sanity checking
        // -------------------------------------------------
        if (!a_ssl)
        {
                return STATUS_ERROR;
        }
        // -------------------------------------------------
        // *************************************************
        // extract
        // *************************************************
        // -------------------------------------------------
        // -------------------------------------------------
        // *************************************************
        // TODO TEST TEST!!!
        // *************************************************
        // -------------------------------------------------
#if 1
        // -------------------------------------------------
        // get fd
        // -------------------------------------------------
        int l_fd = -1;
        l_fd = SSL_get_fd(a_ssl);
        if (l_fd == -1)
        {
                return STATUS_ERROR;
        }
        //BIO* l_rbio = nullptr;
        //l_rbio = SSL_get_rbio(m_ssl);
        //NDBG_PRINT("  [%sACPT%s] l_rbio: %p\n", ANSI_COLOR_FG_GREEN, ANSI_COLOR_OFF, l_rbio);
        //BIO_set_callback(l_rbio, _read_bio_cb);
        int l_peek_size = 0;
        char l_buf[512];
        l_peek_size = recvfrom(l_fd, l_buf, 512, MSG_PEEK, NULL, NULL);
        NDBG_PRINT("  [%sACPT%s] display MSG_PEEK: size: %d\n", ANSI_COLOR_FG_GREEN, ANSI_COLOR_OFF, l_peek_size);
        if (l_peek_size > 0)
        {
        mem_display((const uint8_t*)l_buf, (uint32_t)l_peek_size, true);
        }
        NDBG_PRINT("  [%sACPT%s] *******************\n", ANSI_COLOR_FG_GREEN, ANSI_COLOR_OFF);
#endif
        // TODO!!!
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
        // helper to print a list
        // -------------------------------------------------
#define _PRINT_LIST(_list) do { \
        for (auto & i_c : _list) { \
                if (_is_ext_greased(i_c)) { continue; } \
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
