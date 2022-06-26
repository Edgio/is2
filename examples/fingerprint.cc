//! ----------------------------------------------------------------------------
//! Copyright Edgio Inc.
//!
//! \file:    TODO
//! \details: TODO
//!
//! Licensed under the terms of the Apache 2.0 open source license.
//! Please refer to the LICENSE file in the project root for the terms.
//! ----------------------------------------------------------------------------
// ---------------------------------------------------------
// is2
// ---------------------------------------------------------
#include "is2/srvr/srvr.h"
#include "is2/srvr/lsnr.h"
#include "is2/nconn/nconn.h"
#include "is2/srvr/access.h"
#include "is2/srvr/default_rqst_h.h"
#include "is2/srvr/api_resp.h"
#include "is2/srvr/rqst.h"
#include "is2/srvr/session.h"
#include "is2/support/ja3.h"
#include "is2/support/trace.h"
#include "is2/support/ndebug.h"
// ---------------------------------------------------------
// rapidjson
// ---------------------------------------------------------
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>
// ---------------------------------------------------------
// openssl
// ---------------------------------------------------------
#include <openssl/opensslv.h>
#include <openssl/evp.h>
#include <openssl/ssl.h>
// ---------------------------------------------------------
// std libs
// ---------------------------------------------------------
#include <string.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <arpa/inet.h>
//! ----------------------------------------------------------------------------
//! const
//! ----------------------------------------------------------------------------
#ifndef STATUS_OK
#define STATUS_OK 0
#endif
#ifndef STATUS_ERROR
#define STATUS_ERROR 0
#endif
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
static int32_t get_client_ip(ns_is2::session &a_session,
                             std::string& ao_client_ip)
{
        ao_client_ip.clear();
        // -------------------------------------------------
        // Get access_info
        // -------------------------------------------------
        const ns_is2::access_info &l_ai = ns_is2::get_access_info(a_session);
        char l_clnt_addr_str[INET6_ADDRSTRLEN];
        l_clnt_addr_str[0] = '\0';
        // -------------------------------------------------
        // ipv4
        // -------------------------------------------------
        if (l_ai.m_conn_clnt_sa_len == sizeof(sockaddr_in))
        {
                errno = 0;
                const char *l_s;
                l_s = inet_ntop(AF_INET,
                                &(((sockaddr_in *)(&l_ai.m_conn_clnt_sa))->sin_addr),
                                l_clnt_addr_str,
                                INET_ADDRSTRLEN);
                if (!l_s)
                {
                        TRC_ERROR("Error performing inet_ntop. Reason: %s\n", strerror(errno));
                        return STATUS_ERROR;
                }
        }
        // -------------------------------------------------
        // ipv6
        // -------------------------------------------------
        else if (l_ai.m_conn_clnt_sa_len == sizeof(sockaddr_in6))
        {
                errno = 0;
                const char *l_s;
                l_s = inet_ntop(AF_INET6,
                                &(((sockaddr_in6 *)(&l_ai.m_conn_clnt_sa))->sin6_addr),
                                l_clnt_addr_str,
                                INET6_ADDRSTRLEN);
                if (!l_s)
                {
                        TRC_ERROR("Error performing inet_ntop. Reason: %s\n", strerror(errno));
                        return STATUS_ERROR;
                }
        }
        // -------------------------------------------------
        // localhost
        // -------------------------------------------------
        if (strnlen(l_clnt_addr_str, INET6_ADDRSTRLEN) <= 4)
        {
                snprintf(l_clnt_addr_str, INET6_ADDRSTRLEN, "127.0.0.1");
        }
        // -------------------------------------------------
        // done
        // -------------------------------------------------
        ao_client_ip = l_clnt_addr_str;
        return STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! ****************************************************************************
//! fingerprint handler
//! ****************************************************************************
//! ----------------------------------------------------------------------------
class fingerprint_h: public ns_is2::default_rqst_h
{
public:
        // -------------------------------------------------
        // public methods
        // -------------------------------------------------
        fingerprint_h(void) {};
        ~fingerprint_h() {};
        ns_is2::h_resp_t do_get(ns_is2::session &a_session,
                                ns_is2::rqst &a_rqst,
                                const ns_is2::url_pmap_t &a_url_pmap);
private:
        // -------------------------------------------------
        // private methods
        // -------------------------------------------------
        // Disallow copy/assign
        fingerprint_h& operator=(const fingerprint_h &);
        fingerprint_h(const fingerprint_h &);
};
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
ns_is2::h_resp_t fingerprint_h::do_get(ns_is2::session &a_session,
                                       ns_is2::rqst &a_rqst,
                                       const ns_is2::url_pmap_t &a_url_pmap)
{
        // -------------------------------------------------
        // Create body...
        // -------------------------------------------------
        rapidjson::StringBuffer l_strbuf;
        rapidjson::Writer<rapidjson::StringBuffer> l_writer(l_strbuf);
        l_writer.StartObject();
        // -------------------------------------------------
        // client ip
        // -------------------------------------------------
        std::string l_client_ip;
        get_client_ip(a_session, l_client_ip);
        l_writer.Key("client_ip");
        l_writer.String(l_client_ip.c_str());
        // -------------------------------------------------
        // user-agent
        // -------------------------------------------------
        l_writer.Key("user_agent");
        const ns_is2::access_info &l_ai = ns_is2::get_access_info(a_session);
        l_writer.String(l_ai.m_rqst_http_user_agent.c_str());
        // -------------------------------------------------
        // openssl version
        // -------------------------------------------------
        l_writer.Key("openssl_version");
        l_writer.String(OPENSSL_VERSION_TEXT);
        // -------------------------------------------------
        // get ja3
        // -------------------------------------------------
        ns_is2::ja3* l_ja3 = nullptr;
        l_ja3 = ns_is2::nconn_get_ja3(*(a_session.m_nconn));
        if (!l_ja3)
        {
                goto obj_done;
        }
        // -------------------------------------------------
        // write out keys
        // -------------------------------------------------
        l_writer.Key("ja3_fingerprint");
        l_writer.String(l_ja3->get_str().c_str());
        l_writer.Key("ja3_md5");
        l_writer.String(l_ja3->get_md5().c_str());
        // -------------------------------------------------
        // end
        // -------------------------------------------------
obj_done:
        l_writer.EndObject();
        // -------------------------------------------------
        // generate resp
        // -------------------------------------------------
        ns_is2::api_resp &l_api_resp = ns_is2::create_api_resp(a_session);
        l_api_resp.add_std_headers(ns_is2::HTTP_STATUS_OK,
                                   "application/json",
                                   l_strbuf.GetSize(),
                                   a_rqst.m_supports_keep_alives,
                                   a_session.get_server_name());
        l_api_resp.set_body_data(l_strbuf.GetString(), l_strbuf.GetSize());
        ns_is2::queue_api_resp(a_session, l_api_resp);
        return ns_is2::H_RESP_DONE;
}
//! ----------------------------------------------------------------------------
//! \details: Print the command line help.
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
void print_usage(FILE* a_stream, int a_exit_code)
{
        fprintf(a_stream, "Usage: https_files [options]\n");
        fprintf(a_stream, "Options:\n");
        fprintf(a_stream, "  -h, --help          display this help and exit.\n");
        fprintf(a_stream, "  -p, --port          port (default: 12345)\n");
        fprintf(a_stream, "  -k, --key           certificate key file\n");
        fprintf(a_stream, "  -c, --cert          public certificate file\n");
        exit(a_exit_code);
}
//! ----------------------------------------------------------------------------
//! main
//! ----------------------------------------------------------------------------
int main(int argc, char** argv)
{
        // -------------------------------------------------
        // defaults
        // -------------------------------------------------
        ns_is2::trc_log_level_set(ns_is2::TRC_LOG_LEVEL_ERROR);
        char l_opt;
        std::string l_arg;
        int l_option_index = 0;
        uint16_t l_port = 12345;
        std::string l_key;
        std::string l_cert;
        // -------------------------------------------------
        // options
        // -------------------------------------------------
        struct option l_long_options[] =
                {
                // -----------------------------------------
                // options
                // -----------------------------------------
                { "help",         0, 0, 'h' },
                { "port",         1, 0, 'p' },
                { "key",          1, 0, 'k' },
                { "cert",         1, 0, 'c' },
                // list sentinel
                { 0, 0, 0, 0 }
        };
        // -------------------------------------------------
        // Args...
        // -------------------------------------------------
        char l_short_arg_list[] = "hp:k:c:";
        while ((l_opt = getopt_long_only(argc, argv, l_short_arg_list, l_long_options, &l_option_index)) != -1)
        {
                if (optarg)
                {
                        l_arg = std::string(optarg);
                }
                else
                {
                        l_arg.clear();
                }
                //NDBG_PRINT("arg[%c=%d]: %s\n", l_opt, l_option_index, l_arg.c_str());
                switch (l_opt)
                {
                // -----------------------------------------
                // *****************************************
                // options
                // *****************************************
                // -----------------------------------------
                // -----------------------------------------
                // Help
                // -----------------------------------------
                case 'h':
                {
                        print_usage(stdout, STATUS_OK);
                        break;
                }
                // -----------------------------------------
                // port
                // -----------------------------------------
                case 'p':
                {
                        int l_port_val;
                        l_port_val = atoi(optarg);
                        if ((l_port_val < 1) ||
                           (l_port_val > 65535))
                        {
                                printf("Error bad port value: %d.\n", l_port_val);
                                print_usage(stdout, STATUS_ERROR);
                        }
                        l_port = (uint16_t)l_port_val;
                        break;
                }
                // -----------------------------------------
                // key
                // -----------------------------------------
                case 'k':
                {
                        l_key = l_arg;
                        break;
                }
                // -----------------------------------------
                // cert
                // -----------------------------------------
                case 'c':
                {
                        l_cert = l_arg;
                        break;
                }
                // -----------------------------------------
                // What???
                // -----------------------------------------
                case '?':
                {
                        // ---------------------------------
                        // Required argument was missing:
                        // '?' is provided when the 3rd arg
                        // to getopt_long does not begin
                        // with a ':', and is preceeded by
                        // automatic error message.
                        // ---------------------------------
                        printf("  Exiting.\n");
                        print_usage(stdout, STATUS_ERROR);
                        break;
                }
                // -----------------------------------------
                // Huh???
                // -----------------------------------------
                default:
                {
                        printf("Unrecognized option.\n");
                        print_usage(stdout, STATUS_ERROR);
                        break;
                }
                }
        }
        // -------------------------------------------------
        // run server
        // -------------------------------------------------
        ns_is2::lsnr *l_lsnr = new ns_is2::lsnr(l_port, ns_is2::SCHEME_TLS);
        fingerprint_h *l_fp_h = new fingerprint_h();
        l_lsnr->add_route("/*", l_fp_h);
        ns_is2::srvr *l_srvr = new ns_is2::srvr();
        l_srvr->set_tls_server_ctx_key(l_key);
        l_srvr->set_tls_server_ctx_crt(l_cert);
        l_srvr->register_lsnr(l_lsnr);
        l_srvr->set_num_threads(0);
        l_srvr->run();
        if (l_srvr) {delete l_srvr; l_srvr = NULL;}
        if (l_fp_h) {delete l_fp_h; l_fp_h = NULL;}
        return 0;
}

