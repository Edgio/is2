//! ----------------------------------------------------------------------------
//! Copyright Verizon.
//!
//! \file:    TODO
//! \details: TODO
//!
//! Licensed under the terms of the Apache 2.0 open source license.
//! Please refer to the LICENSE file in the project root for the terms.
//! ----------------------------------------------------------------------------
#include <is2/srvr/srvr.h>
#include <is2/srvr/lsnr.h>
#include <is2/handler/file_h.h>
#include <is2/support/trace.h>
#include <string.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
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
        fprintf(a_stream, "  -b, --block_size    set block size for contiguous send/recv sizes\n");
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
        uint32_t l_block_size = 4096;
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
                { "block_size",   1, 0, 'b' },
                // list sentinel
                { 0, 0, 0, 0 }
        };
        // -------------------------------------------------
        // Args...
        // -------------------------------------------------
        char l_short_arg_list[] = "hp:k:c:b:";
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
                // block_size
                // -----------------------------------------
                case 'b':
                {
                        int l_port_val;
                        l_port_val = atoi(optarg);
                        if ((l_port_val < 1))
                        {
                                printf("Error bad block size value: %d.\n", l_port_val);
                                print_usage(stdout, STATUS_ERROR);
                        }
                        l_block_size = (uint32_t)l_port_val;
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
        ns_is2::file_h *l_file_h = new ns_is2::file_h();
        l_lsnr->add_route("/*", l_file_h);
        ns_is2::srvr *l_srvr = new ns_is2::srvr();
        l_srvr->set_tls_server_ctx_key(l_key);
        l_srvr->set_tls_server_ctx_crt(l_cert);
        l_srvr->set_block_size(l_block_size);
        l_srvr->register_lsnr(l_lsnr);
        l_srvr->set_num_threads(0);
        l_srvr->run();
        if (l_srvr) {delete l_srvr; l_srvr = NULL;}
        if (l_file_h) {delete l_file_h; l_file_h = NULL;}
        return 0;
}

