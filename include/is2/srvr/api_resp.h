//! ----------------------------------------------------------------------------
//! Copyright Edgio Inc.
//!
//! \file:    TODO
//! \details: TODO
//!
//! Licensed under the terms of the Apache 2.0 open source license.
//! Please refer to the LICENSE file in the project root for the terms.
//! ----------------------------------------------------------------------------
#ifndef _API_RESP_H
#define _API_RESP_H
//! ----------------------------------------------------------------------------
//! includes
//! ----------------------------------------------------------------------------
#include "is2/srvr/http_status.h"
#include "is2/support/data.h"
#include <stdint.h>
#include <string>
//! ----------------------------------------------------------------------------
//! Fwd Decl's
//! ----------------------------------------------------------------------------
namespace ns_is2 {
class nbq;
}
namespace ns_is2 {
//! ----------------------------------------------------------------------------
//! api_resp
//! ----------------------------------------------------------------------------
class api_resp
{
public:
        // -------------------------------------------------
        // public methods
        // -------------------------------------------------
        api_resp(void);
        ~api_resp();
        const kv_map_list_t &get_headers(void);
        http_status_t get_status(void);
        void set_status(http_status_t a_status);
        int set_header(const std::string &a_header);
        int set_header(const std::string &a_key, const std::string &a_val);
        void set_headers(const kv_map_list_t &a_headers_list);
        int set_headerf(const std::string &a_key, const char* fmt, ...) __attribute__((format(__printf__,3,4)));;
        void set_body_data(const char *a_ptr, uint32_t a_len);
        void add_std_headers(http_status_t a_status,
                             const char *a_content_type,
                             uint64_t a_len,
                             bool a_keep_alive,
                             const std::string &a_server_name);
        void clear_headers(void);
        int32_t serialize(nbq &ao_q);
private:
        // -------------------------------------------------
        // private methods
        // -------------------------------------------------
        // Disallow copy/assign
        api_resp& operator=(const api_resp &);
        api_resp(const api_resp &);
        // -------------------------------------------------
        // private members
        // -------------------------------------------------
        http_status_t m_status;
        kv_map_list_t m_headers;
        const char *m_body_data;
        uint32_t m_body_data_len;
};
//! ----------------------------------------------------------------------------
//! helpers
//! ----------------------------------------------------------------------------
const char *get_resp_status_str(http_status_t a_status);
//! ----------------------------------------------------------------------------
//! nbq_utils
//! ----------------------------------------------------------------------------
int32_t nbq_write_request_line(nbq &ao_q, const char *a_buf, uint32_t a_len);
int32_t nbq_write_status(nbq &ao_q, http_status_t a_status);
int32_t nbq_write_header(nbq &ao_q,
                         const char *a_key_buf, uint32_t a_key_len,
                         const char *a_val_buf, uint32_t a_val_len);
int32_t nbq_write_header(nbq &ao_q, const char *a_key_buf, const char *a_val_buf);
int32_t nbq_write_body(nbq &ao_q, const char *a_buf, uint32_t a_len);
void create_json_resp_str(http_status_t a_status, std::string &ao_resp_str);
}
#endif
