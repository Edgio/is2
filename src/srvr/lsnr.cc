//: ----------------------------------------------------------------------------
//: Copyright (C) 2018 Verizon.  All Rights Reserved.
//: All Rights Reserved
//:
//: \file:    lsnr.cc
//: \details: TODO
//: \author:  Reed P. Morrison
//: \date:    05/28/2015
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
#include "srvr/t_srvr.h"
#include "is2/support/ndebug.h"
#include "nconn/nconn_tls.h"
#include "is2/status.h"
#include "is2/support/trace.h"
#include "is2/srvr/rqst.h"
#include "is2/srvr/lsnr.h"
#include "is2/url_router/url_router.h"
#include "is2/srvr/session.h"
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
//: ----------------------------------------------------------------------------
//: constants
//: ----------------------------------------------------------------------------
#define MAX_PENDING_CONNECT_REQUESTS 16384
//: ----------------------------------------------------------------------------
//: macros
//: ----------------------------------------------------------------------------
#define T_CLNT_SET_NCONN_OPT(_conn, _opt, _buf, _len) \
        do { \
                int _status = 0; \
                _status = _conn.set_opt((_opt), (_buf), (_len)); \
                if(_status != nconn::NC_STATUS_OK) { \
                        TRC_ERROR("set_opt %d.  Status: %d.\n", \
                                   _opt, _status); \
                        return STATUS_ERROR;\
                } \
        } while(0)

// Set socket option macro...
#define SET_SOCK_OPT(_sock_fd, _sock_opt_level, _sock_opt_name, _sock_opt_val) \
        do { \
                int _l__sock_opt_val = _sock_opt_val; \
                int _l_status = 0; \
                _l_status = ::setsockopt(_sock_fd, \
                                _sock_opt_level, \
                                _sock_opt_name, \
                                &_l__sock_opt_val, \
                                sizeof(_l__sock_opt_val)); \
                                if(_l_status == -1) { \
                                        TRC_ERROR("STATUS_ERROR: Failed to set %s.  Reason: %s.\n", \
                                                   #_sock_opt_name, strerror(errno)); \
                                        return STATUS_ERROR;\
                                } \
        } while(0)
namespace ns_is2 {
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
lsnr::lsnr(uint16_t a_port,
           scheme_t a_scheme,
           rqst_h* a_default_handler,
           url_router* a_url_router):
        m_scheme(a_scheme),
        m_local_addr_v4(INADDR_ANY),
        m_port(a_port),
        m_sa(),
        m_sa_len(0),
        m_default_handler(a_default_handler),
        m_url_router(NULL),
        m_url_router_ref(false),
        m_t_srvr(NULL),
        m_fd(-1),
        m_is_initd(false)
{
        if(a_url_router)
        {
                m_url_router = a_url_router;
                m_url_router_ref = true;
        }
        else
        {
                m_url_router = new url_router();
        }
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
lsnr::~lsnr()
{
        if(m_fd > 0)
        {
                // shutdown
                shutdown(m_fd, SHUT_RD);
        }
        if(!m_url_router_ref &&
           m_url_router)
        {
                delete m_url_router;
                m_url_router = NULL;
        }
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
int32_t lsnr::set_default_route(rqst_h *a_handler)
{
        m_default_handler = a_handler;
        return STATUS_OK;
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
int32_t lsnr::add_route(const std::string &a_endpoint, const rqst_h *a_rqst_h)
{
        if(!m_url_router)
        {
                return STATUS_ERROR;
        }
        return m_url_router->add_route(a_endpoint, a_rqst_h);
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
int32_t lsnr::init(void)
{
        if(m_is_initd)
        {
                return STATUS_OK;
        }
        // -------------------------------------------------
        // Create listen socket
        // -------------------------------------------------
        sockaddr_in *l_sa = (struct sockaddr_in *)(&m_sa);
        m_sa_len = sizeof(struct sockaddr_in);
        int32_t l_s;
        // -------------------------------------------------
        // Create socket for incoming connections
        // -------------------------------------------------
#if defined(__APPLE__) || defined(__darwin__)
        m_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
#else
        m_fd = socket(PF_INET, SOCK_STREAM | SOCK_CLOEXEC, IPPROTO_TCP);
#endif
        if(m_fd < 0)
        {
                TRC_ERROR("Error socket() failed. Reason[%d]: %s\n", errno, strerror(errno));
                return STATUS_ERROR;
        }
        // -------------------------------------------------
        // Set socket options
        // -Enable Socket reuse.
        // -------------------------------------------------
        SET_SOCK_OPT(m_fd, SOL_SOCKET, SO_REUSEADDR, 1);
#ifdef SO_REUSEPORT
        SET_SOCK_OPT(m_fd, SOL_SOCKET, SO_REUSEPORT, 1);
#endif
        // -------------------------------------------------
        // Construct local address structure
        // -------------------------------------------------
        memset(l_sa, 0, sizeof((*l_sa))); // Zero out structure
        // IPv4 for now
        l_sa->sin_family      = AF_INET;                 // Internet address family
        l_sa->sin_addr.s_addr = htonl(m_local_addr_v4);  // Any incoming interface
        l_sa->sin_port        = htons(m_port);           // Local port
        // -------------------------------------------------
        // Bind to the local address
        // -------------------------------------------------
        l_s = bind(m_fd, (struct sockaddr *) l_sa, sizeof((*l_sa)));
        if(l_s < 0)
        {
                TRC_ERROR("Error bind() failed (port: %d). Reason[%d]: %s\n", m_port, errno, strerror(errno));
                return STATUS_ERROR;
        }
        // -------------------------------------------------
        // Mark the socket so it will listen for
        // incoming connections
        // -------------------------------------------------
        l_s = listen(m_fd, MAX_PENDING_CONNECT_REQUESTS);
        if(l_s < 0)
        {
                TRC_ERROR("Error listen() failed. Reason[%d]: %s\n", errno, strerror(errno));
                return STATUS_ERROR;
        }
        m_is_initd = true;
        return STATUS_OK;
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
int32_t lsnr::set_local_addr_v4(const char *a_addr_str)
{
        int32_t l_s;
        struct sockaddr_in l_c_addr;
        bzero((char *) &l_c_addr, sizeof(l_c_addr));
        l_s = inet_pton(AF_INET, a_addr_str, &(l_c_addr.sin_addr));
        if(l_s != 1)
        {
                TRC_ERROR("inet_pton() failed. Reason[%d]: %s\n", errno, strerror(errno));
                return STATUS_ERROR;
        }
        m_local_addr_v4 = ntohl(l_c_addr.sin_addr.s_addr);
        return STATUS_OK;
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
int32_t lsnr::evr_fd_readable_cb(void *a_data)
{
        //NDBG_PRINT("%sREADABLE%s %p\n", ANSI_COLOR_BG_GREEN, ANSI_COLOR_OFF, a_data);
        if(!a_data)
        {
                return STATUS_OK;
        }
        nconn* l_nconn = static_cast<nconn*>(a_data);
        if(!l_nconn->get_ctx())
        {
                TRC_ERROR("connection context == NULL\n");
                return STATUS_ERROR;
        }
        t_srvr *l_t_srvr = static_cast<t_srvr *>(l_nconn->get_ctx());
        lsnr *l_lsnr = static_cast<lsnr *>(l_nconn->get_data());
        //NDBG_PRINT("%sREADABLE%s LABEL: %s LSNR: %p\n", ANSI_COLOR_FG_GREEN, ANSI_COLOR_OFF,
        //                l_nconn->get_label().c_str(), l_lsnr);
        // Server -incoming client connections
        if(!l_nconn->is_listening())
        {
                TRC_ERROR("connection not listening\n");
                return STATUS_ERROR;
        }
        if(!l_lsnr ||
           !l_nconn)
        {
                TRC_ERROR("no listener[%p] or connection[%p]\n", l_lsnr, l_nconn);
                return STATUS_ERROR;
        }
        // Returns new client fd on success
        int l_fd;
        l_fd = l_nconn->ncaccept();
        if(l_fd == nconn::NC_STATUS_ERROR)
        {
                TRC_ERROR("performing ncaccept\n");
                return STATUS_ERROR;
        }
        else if(l_fd == nconn::NC_STATUS_OK)
        {
                return STATUS_OK;
        }
        else if(l_nconn->is_accepting())
        {
                TRC_ERROR("still accepting...\n");
                // TODO allow for EAGAIN -non-blocking accept???
                return STATUS_ERROR;
        }
        // -------------------------------------------------
        // set back to connected
        // -------------------------------------------------
        l_nconn->set_state(nconn::NC_STATE_CONNECTED);
        // -------------------------------------------------
        // get new connected client conn
        // -------------------------------------------------
        nconn *l_clnt_conn = NULL;
        //NDBG_PRINT("CREATING NEW CONNECTION: a_scheme: %d\n", a_scheme);
        if(l_nconn->get_scheme() == SCHEME_TCP)
        {
                l_clnt_conn = new nconn_tcp();
        }
        else if(l_nconn->get_scheme() == SCHEME_TLS)
        {
                l_clnt_conn = new nconn_tls();
        }
        else
        {
                TRC_ERROR("performing m_nconn_pool.get\n");
                return STATUS_ERROR;
        }
        // -------------------------------------------------
        // init conn
        // -------------------------------------------------
        const t_conf& l_conf = *(l_lsnr->m_t_srvr->m_t_conf);
        l_clnt_conn->set_ctx(l_lsnr->m_t_srvr);
        l_clnt_conn->set_num_reqs_per_conn(l_conf.m_num_reqs_per_conn);
        //l_nconn->set_collect_stats(m_t_srvr->m_t_conf->m_collect_stats);
        l_clnt_conn->set_evr_loop(l_lsnr->m_t_srvr->get_evr_loop());
        l_clnt_conn->setup_evr_fd(session::evr_fd_readable_cb,
                                  session::evr_fd_writeable_cb,
                                  session::evr_fd_error_cb);
        if(l_clnt_conn->get_scheme() == SCHEME_TLS)
        {
                T_CLNT_SET_NCONN_OPT((*l_clnt_conn),nconn_tls::OPT_TLS_CIPHER_STR,l_conf.m_tls_server_ctx_cipher_list.c_str(),l_conf.m_tls_server_ctx_cipher_list.length());
                T_CLNT_SET_NCONN_OPT((*l_clnt_conn),nconn_tls::OPT_TLS_CTX,l_conf.m_tls_server_ctx,sizeof(l_conf.m_tls_server_ctx));
                if(!l_conf.m_tls_server_ctx_crt.empty())
                {
                        T_CLNT_SET_NCONN_OPT((*l_clnt_conn),nconn_tls::OPT_TLS_TLS_CRT,l_conf.m_tls_server_ctx_crt.c_str(),l_conf.m_tls_server_ctx_crt.length());
                }
                if(!l_conf.m_tls_server_ctx_key.empty())
                {
                        T_CLNT_SET_NCONN_OPT((*l_clnt_conn),nconn_tls::OPT_TLS_TLS_KEY,l_conf.m_tls_server_ctx_key.c_str(),l_conf.m_tls_server_ctx_key.length());
                }
                T_CLNT_SET_NCONN_OPT((*l_clnt_conn),nconn_tls::OPT_TLS_OPTIONS,&(l_conf.m_tls_server_ctx_options),sizeof(l_conf.m_tls_server_ctx_options));
        }
        // -------------------------------------------------
        // clnt session setup
        // -------------------------------------------------
        session *l_cs = new session(*(l_lsnr->m_t_srvr));
        l_cs->m_lsnr = l_lsnr;
        l_cs->m_evr_timeout = NULL;
        l_cs->m_resp_done_cb = l_lsnr->m_t_srvr->m_t_conf->m_resp_done_cb;
        l_cs->m_in_q = l_lsnr->m_t_srvr->get_nbq(NULL);
        l_cs->m_out_q = NULL;
        l_cs->m_nconn = l_clnt_conn;
        l_clnt_conn->set_data(l_cs);
        //TRC_DEBUG("l_cs: %p\n", l_cs);
        // stats
        //++m_stat.m_clnt_conn_started;
        // -------------------------------------------------
        // Set access info
        // TODO move to session???
        // -------------------------------------------------
        l_nconn->get_remote_sa(l_cs->m_access_info.m_conn_clnt_sa,
                               l_cs->m_access_info.m_conn_clnt_sa_len);
        l_lsnr->get_sa(l_cs->m_access_info.m_conn_upsv_sa,
                       l_cs->m_access_info.m_conn_upsv_sa_len);
        l_cs->m_access_info.m_start_time_ms = 0;
        l_cs->m_access_info.m_total_time_ms = 0;
        l_cs->m_access_info.m_bytes_in = 0;
        l_cs->m_access_info.m_bytes_out = 0;
        // -------------------------------------------------
        // set accepting
        // -------------------------------------------------
        int32_t l_s;
        l_s = l_clnt_conn->nc_set_accepting(l_fd);
        if(l_s != STATUS_OK)
        {
                TRC_ERROR("performing nc_set_accepting\n");
                // TODO cleanup conn???
                delete l_cs;
                l_cs = NULL;
                return STATUS_ERROR;
        }
        // -------------------------------------------------
        // rqst setup
        // -------------------------------------------------
        l_cs->m_rqst = new rqst();
        l_cs->m_rqst->set_q(l_cs->m_in_q);
        // -------------------------------------------------
        // Add idle timer
        // -------------------------------------------------
        l_cs->set_timeout_ms(l_t_srvr->get_timeout_ms());
        l_cs->set_last_active_ms(get_time_ms());
        l_t_srvr->add_timer(l_cs->get_timeout_ms(),
                            session::evr_event_timeout_cb,
                            l_clnt_conn,
                            (void **)(&(l_cs->m_evr_timeout)));
        // TODO Check status
        return STATUS_OK;
}
}
