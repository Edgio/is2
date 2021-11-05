//! ----------------------------------------------------------------------------
//! Copyright Edgecast Inc.
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
#include "nconn/nconn_tcp.h"
#include "is2/support/ndebug.h"
#include "is2/support/time_util.h"
#include "is2/support/trace.h"
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/ioctl.h>
// From wine msg groups
// https://www.winehq.org/pipermail/wine-cvs/2007-March/030712.html
#ifdef __MACH__
  #ifndef SOL_TCP
  #define SOL_TCP IPPROTO_TCP
  #endif
#endif
//! ----------------------------------------------------------------------------
//! Macros
//! ----------------------------------------------------------------------------
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
                if (_l_status == -1) { \
                        TRC_ERROR("Failed to set sock_opt: %s.  Reason: %s.\n", #_sock_opt_name, strerror(errno)); \
                        return NC_STATUS_ERROR;\
                } \
        } while(0)
namespace ns_is2 {
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t nconn_tcp::set_opt(uint32_t a_opt, const void *a_buf, uint32_t a_len)
{
        switch(a_opt)
        {
        case OPT_TCP_RECV_BUF_SIZE:
        {
                m_sock_opt_recv_buf_size = a_len;
                break;
        }
        case OPT_TCP_SEND_BUF_SIZE:
        {
                m_sock_opt_send_buf_size = a_len;
                break;
        }
        case OPT_TCP_NO_DELAY:
        {
                m_sock_opt_no_delay = (bool)a_len;
                break;
        }
        case OPT_TCP_NO_LINGER:
        {
                m_sock_opt_no_linger = (bool)a_len;
                break;
        }
        default:
        {
                return NC_STATUS_UNSUPPORTED;
        }
        }
        return NC_STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t nconn_tcp::get_opt(uint32_t a_opt, void **a_buf, uint32_t *a_len)
{
        switch(a_opt)
        {
        case OPT_TCP_FD:
        {
                *a_buf = &m_fd;
                *a_len = sizeof(m_fd);
                break;
        }
        default:
        {
                //NDBG_PRINT("Error unsupported option: %d\n", a_opt);
                return NC_STATUS_UNSUPPORTED;
        }
        }
        return NC_STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t nconn_tcp::ncset_listening(int32_t a_val)
{
        m_fd = a_val;
        m_tcp_state = TCP_STATE_LISTENING;
        int l_opt = 1;
        ioctl(m_fd, FIONBIO, &l_opt);
        // Add to event handler
        if (m_evr_loop)
        {
                if (0 != m_evr_loop->add_fd(a_val,
                                           EVR_FILE_ATTR_MASK_READ |
                                           EVR_FILE_ATTR_MASK_STATUS_ERROR |
                                           EVR_FILE_ATTR_MASK_RD_HUP |
                                           EVR_FILE_ATTR_MASK_HUP,
                                           &m_evr_fd))
                {
                        TRC_ERROR("Couldn't add socket file descriptor\n");
                        return NC_STATUS_ERROR;
                }
        }
        return NC_STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t nconn_tcp::ncset_listening_nb(int32_t a_val)
{
        m_fd = a_val;
        m_tcp_state = TCP_STATE_LISTENING;
        // -------------------------------------------------
        // Get/set flags for setting non-blocking
        // Can set with set_sock_opt???
        // -------------------------------------------------
        errno = 0;
        const int flags = ::fcntl(m_fd, F_GETFL, 0);
        if (flags == -1)
        {
                NCONN_ERROR(CONN_STATUS_ERROR_INTERNAL,
                            "LABEL[%s]: Error getting flags for fd. Reason: %s\n",
                            m_label.c_str(), ::strerror(errno));
                return NC_STATUS_ERROR;
        }
        if (::fcntl(m_fd, F_SETFL, flags | O_NONBLOCK) < 0)
        {
                NCONN_ERROR(CONN_STATUS_ERROR_INTERNAL,
                            "LABEL[%s]: Error setting fd to non-block mode. Reason: %s\n",
                            m_label.c_str(), ::strerror(errno));
                return NC_STATUS_ERROR;
        }
        // Add to event handler
        if (m_evr_loop)
        {
                if (0 != m_evr_loop->add_fd(a_val,
                                           EVR_FILE_ATTR_MASK_READ|
                                           EVR_FILE_ATTR_MASK_STATUS_ERROR |
                                           EVR_FILE_ATTR_MASK_RD_HUP |
                                           EVR_FILE_ATTR_MASK_HUP |
                                           EVR_FILE_ATTR_MASK_ET,
                                           &m_evr_fd))
                {
                        TRC_ERROR("Couldn't add socket fd\n");
                        return NC_STATUS_ERROR;
                }
        }
        return NC_STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t nconn_tcp::ncset_accepting(int a_fd)
{
        if (a_fd < 0)
        {
                return NC_STATUS_OK;
        }
        m_tcp_state = TCP_STATE_ACCEPTING;
        m_fd = a_fd;
        // Using accept4 -TODO portable???
#if 0
        // -------------------------------------------------
        // Can set with set_sock_opt???
        // -------------------------------------------------
        // Set the file descriptor to no-delay mode.
        const int flags = ::fcntl(m_fd, F_GETFL, 0);
        if (flags == -1)
        {
                NCONN_ERROR(CONN_STATUS_ERROR_INTERNAL,
                            "LABEL[%s]: Error getting flags for fd. Reason: %s\n",
                            m_label.c_str(), ::strerror(errno));
                return NC_STATUS_ERROR;
        }
        if (::fcntl(m_fd, F_SETFL, flags | O_NONBLOCK) < 0)
        {
                NCONN_ERROR(CONN_STATUS_ERROR_INTERNAL,
                            "LABEL[%s]: Error setting fd to non-block mode. Reason: %s\n",
                            m_label.c_str(), ::strerror(errno));
                return NC_STATUS_ERROR;
        }
#endif
        // Add to event handler
        if (m_evr_loop)
        {
                if (0 != m_evr_loop->add_fd(m_fd,
                                           EVR_FILE_ATTR_MASK_READ|
                                           EVR_FILE_ATTR_MASK_STATUS_ERROR |
                                           EVR_FILE_ATTR_MASK_RD_HUP |
                                           EVR_FILE_ATTR_MASK_HUP |
                                           EVR_FILE_ATTR_MASK_ET,
                                           &m_evr_fd))
                {
                        TRC_ERROR("Couldn't add socket fd[%d]\n", m_fd);
                        return NC_STATUS_ERROR;
                }
        }
        return NC_STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t nconn_tcp::ncset_connected(void)
{
        if (!m_sock_opt_no_delay)
        {
                m_sock_opt_no_delay = true;
                SET_SOCK_OPT(m_fd, SOL_TCP, TCP_NODELAY, 1);
        }
        if (m_evr_loop)
        {
                if (0 != m_evr_loop->mod_fd(m_fd,
                                           EVR_FILE_ATTR_MASK_READ|
                                           EVR_FILE_ATTR_MASK_STATUS_ERROR |
                                           EVR_FILE_ATTR_MASK_RD_HUP |
                                           EVR_FILE_ATTR_MASK_HUP |
                                           EVR_FILE_ATTR_MASK_ET,
                                           &m_evr_fd))
                {
                        TRC_ERROR("Couldn't add socket fd\n");
                        return NC_STATUS_ERROR;
                }
        }
        return NC_STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t nconn_tcp::ncread(char *a_buf, uint32_t a_buf_len)
{
        ssize_t l_s;
        int32_t l_bytes_read = 0;
        //l_s = read(m_fd, a_buf, a_buf_len);
        errno = 0;
        l_s = recvfrom(m_fd, a_buf, a_buf_len, 0, NULL, NULL);
        TRC_ALL("HOST[%s] fd[%3d] READ: %zd bytes. buf: %p. Reason: %s\n",
                m_label.c_str(),
                m_fd,
                l_s,
                a_buf,
                ::strerror(errno));
        if (l_s > 0) TRC_ALL_MEM((const uint8_t *)a_buf, l_s);
        if (l_s > 0)
        {
                l_bytes_read += l_s;
                return l_bytes_read;
        }
        else if (l_s == 0)
        {
                return NC_STATUS_EOF;
        }
        else
        {
                switch(errno)
                {
                        case EAGAIN:
                        {
                                return NC_STATUS_AGAIN;
                        }
                        case ECONNRESET:
                        {
                                TRC_WARN("recvfrom: %s\n", strerror(errno));
                                return NC_STATUS_ERROR;
                        }
                        default:
                        {
                                TRC_WARN("recvfrom: %s\n", strerror(errno));
                                return NC_STATUS_ERROR;
                        }
                }
        }
        return l_bytes_read;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t nconn_tcp::ncwrite(char *a_buf, uint32_t a_buf_len)
{
        int l_s;
        // -------------------------------------------------
        // TODO see post:
        // http://noahdesu.github.io/2014/01/16/port-sendmsg.html
        // for info on porting MSG_NOSIGNAL to OSX
        // OSX has SO_NOSIGPIPE socket option
        // -can be enabled with
        // int val = 1;
        // setsockopt(sd, SOL_SOCKET, SO_NOSIGPIPE, (void*)&val, sizeof(val));
        // -------------------------------------------------
        errno = 0;
#ifdef __MACH__
        l_s = send(m_fd, a_buf, a_buf_len, 0);
#else
        l_s = send(m_fd, a_buf, a_buf_len, MSG_NOSIGNAL);
#endif
        if (l_s > 0)
        {
                TRC_ALL_MEM((const uint8_t*)(a_buf), (uint32_t)(l_s));
        }
        // -------------------------------------------------
        // write >= 0 bytes successfully
        // -------------------------------------------------
        if (l_s >= 0)
        {
                return l_s;
        }
        // -------------------------------------------------
        // EAGAIN
        // -------------------------------------------------
        if (errno == EAGAIN)
        {
                // Add to writeable
                if (m_evr_loop)
                {
                        l_s = m_evr_loop->mod_fd(m_fd,
                                        EVR_FILE_ATTR_MASK_WRITE|
                                        EVR_FILE_ATTR_MASK_STATUS_ERROR |
                                        EVR_FILE_ATTR_MASK_RD_HUP |
                                        EVR_FILE_ATTR_MASK_HUP |
                                        EVR_FILE_ATTR_MASK_ET,
                                        &m_evr_fd);
                        if (l_s != STATUS_OK)
                        {
                                NCONN_ERROR(CONN_STATUS_ERROR_INTERNAL,
                                            "LABEL[%s]: Error: Couldn't add socket file descriptor\n",
                                            m_label.c_str());
                                return NC_STATUS_ERROR;
                        }
                }
                return NC_STATUS_AGAIN;
        }
        // -------------------------------------------------
        // unknown error
        // -------------------------------------------------
        NCONN_ERROR(CONN_STATUS_ERROR_INTERNAL,
                    "LABEL[%s]: Error: performing write.  Reason: %s.\n",
                    m_label.c_str(), ::strerror(errno));
        return NC_STATUS_ERROR;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t nconn_tcp::ncsetup()
{
        // -------------------------------------------------
        // create socket.
        // -------------------------------------------------
#ifdef __linux__
        m_fd = ::socket(m_host_info.m_sock_family,
                        m_host_info.m_sock_type | SOCK_CLOEXEC,
                        m_host_info.m_sock_protocol);
#else
        m_fd = ::socket(m_host_info.m_sock_family,
                        m_host_info.m_sock_type,
                        m_host_info.m_sock_protocol);
        fcntl(m_fd, F_SETFD, FD_CLOEXEC);
#endif
        if (m_fd < 0)
        {
                NCONN_ERROR(CONN_STATUS_ERROR_INTERNAL,
                            "LABEL[%s]: Error creating socket. Reason: %s\n",
                            m_label.c_str(), ::strerror(errno));
                return NC_STATUS_ERROR;
        }
        // -------------------------------------------------
        // socket options
        // -------------------------------------------------
        // TODO --set to REUSE????
        SET_SOCK_OPT(m_fd, SOL_SOCKET, SO_REUSEADDR, 1);
#ifdef SO_REUSEPORT
        SET_SOCK_OPT(m_fd, SOL_SOCKET, SO_REUSEPORT, 1);
#endif
        if (m_sock_opt_send_buf_size)
        {
                SET_SOCK_OPT(m_fd, SOL_SOCKET, SO_SNDBUF, m_sock_opt_send_buf_size);
        }
        if (m_sock_opt_recv_buf_size)
        {
                SET_SOCK_OPT(m_fd, SOL_SOCKET, SO_RCVBUF, m_sock_opt_recv_buf_size);
        }
        if (m_sock_opt_no_delay)
        {
                SET_SOCK_OPT(m_fd, SOL_TCP, TCP_NODELAY, 1);
        }
        // -------------------------------------------------
        // Can set with set_sock_opt???
        // -------------------------------------------------
        // Set the file descriptor to no-delay mode.
        const int flags = ::fcntl(m_fd, F_GETFL, 0);
        if (flags == -1)
        {
                NCONN_ERROR(CONN_STATUS_ERROR_INTERNAL,
                            "LABEL[%s]: Error getting flags for fd. Reason: %s\n",
                            m_label.c_str(), ::strerror(errno));
                return NC_STATUS_ERROR;
        }
        if (::fcntl(m_fd, F_SETFL, flags | O_NONBLOCK) < 0)
        {
                NCONN_ERROR(CONN_STATUS_ERROR_INTERNAL,
                            "LABEL[%s]: Error setting fd to non-block mode. Reason: %s\n",
                            m_label.c_str(), ::strerror(errno));
                return NC_STATUS_ERROR;
        }
        // -------------------------------------------------
        // Add to reactor
        // -------------------------------------------------
        if (m_evr_loop)
        {
                if (0 != m_evr_loop->add_fd(m_fd,
                                            EVR_FILE_ATTR_MASK_READ|
                                            EVR_FILE_ATTR_MASK_RD_HUP|
                                            EVR_FILE_ATTR_MASK_ET,
                                            &m_evr_fd))
                {
                        NCONN_ERROR(CONN_STATUS_ERROR_INTERNAL,
                                    "LABEL[%s]: Error: Couldn't add socket file descriptor\n",
                                    m_label.c_str());
                        return NC_STATUS_ERROR;
                }
        }
        return NC_STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t nconn_tcp::ncaccept()
{
        if (m_tcp_state != TCP_STATE_LISTENING)
        {
                m_tcp_state = TCP_STATE_CONNECTED;
                return nconn::NC_STATUS_OK;
        }
        // -------------------------------------------------
        // init state
        // -------------------------------------------------
        int l_fd;
        m_remote_sa_len = sizeof(m_remote_sa);
        bzero(&m_remote_sa, m_remote_sa_len);
        errno = 0;
        // -------------------------------------------------
        // accept
        // -------------------------------------------------
#ifdef __linux__
        l_fd = accept4(m_fd,
                       (struct sockaddr *)&m_remote_sa,
                       &m_remote_sa_len,
                       SOCK_NONBLOCK|SOCK_CLOEXEC);
#else
        l_fd = accept(m_fd,
                      (struct sockaddr *)&m_remote_sa,
                      &m_remote_sa_len);
#endif
        if (l_fd < 0)
        {
                switch (errno)
                {
                case EAGAIN:
                {
                        // Return here -still in accepting state
                        return NC_STATUS_OK;
                }
                default:
                {
                        TRC_ERROR("accept failed. Reason[%d]: %s\n", errno, ::strerror(errno));
                        return NC_STATUS_ERROR;
                }
                }
        }
        // -------------------------------------------------
        // if no accept4 -setup socket options
        // -------------------------------------------------
#ifndef __linux__
        int l_s;
        errno = 0;
        int l_flags = fcntl(l_fd, F_GETFL, 0);
        if (l_flags == -1)
        {
                TRC_ERROR("fcntl failed. Reason[%d]: %s\n", errno, ::strerror(errno));
                return NC_STATUS_ERROR;
        }
        errno = 0;
        l_s = fcntl(l_fd, F_SETFL, l_flags | O_NONBLOCK);
        if (l_s == -1)
        {
                TRC_ERROR("fcntl failed. Reason[%d]: %s\n", errno, ::strerror(errno));
                return NC_STATUS_ERROR;
        }
        fcntl(l_fd, F_SETFD, FD_CLOEXEC);
        // TODO check for errors
#endif
        return l_fd;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t nconn_tcp::ncconnect()
{
        uint32_t l_retry_connect_count = 0;
        int l_cs = 0;
        // -------------------------------------------------
        // Set to connecting
        // -------------------------------------------------
        m_tcp_state = TCP_STATE_CONNECTING;
state_top:
        errno = 0;
        // -------------------------------------------------
        // try connect
        // -------------------------------------------------
        l_cs = ::connect(m_fd,
                         ((struct sockaddr*) &(m_host_info.m_sa)),
                         (m_host_info.m_sa_len));
        if (l_cs < 0)
        {
                switch (errno)
                {
                case EISCONN:
                {
                        // Is already connected drop out of switch and return OK
                        break;
                }
                case EINVAL:
                {
                        int l_err;
                        socklen_t l_errlen;
                        l_errlen = sizeof(l_err);
                        if (::getsockopt(m_fd, SOL_SOCKET, SO_ERROR, (void*) &l_err, &l_errlen) < 0)
                        {
                                NCONN_ERROR(CONN_STATUS_ERROR_INTERNAL,
                                            "LABEL[%s]: Error performing getsockopt. Unknown connect error\n",
                                            m_label.c_str());
                        }
                        else
                        {
                                NCONN_ERROR(CONN_STATUS_ERROR_INTERNAL,
                                            "LABEL[%s]: Error performing getsockopt. %s\n",
                                            m_label.c_str(), ::strerror(l_err));
                        }
                        return NC_STATUS_ERROR;
                }
                case ECONNREFUSED:
                {
                        TRC_ERROR("LABEL[%s]: connect failed. Reason: %s\n", m_label.c_str(), strerror(errno));
                        return NC_STATUS_ERROR;
                }
                case EAGAIN:
                case EINPROGRESS:
                {
                        // Set to writeable and try again
                        if (m_evr_loop)
                        {
                                int l_s;
                                l_s = m_evr_loop->mod_fd(m_fd,
                                                EVR_FILE_ATTR_MASK_READ |
                                                EVR_FILE_ATTR_MASK_WRITE |
                                                EVR_FILE_ATTR_MASK_STATUS_ERROR |
                                                EVR_FILE_ATTR_MASK_RD_HUP |
                                                EVR_FILE_ATTR_MASK_HUP |
                                                EVR_FILE_ATTR_MASK_ET,
                                                &m_evr_fd);
                                if (l_s != STATUS_OK)
                                {
                                        NCONN_ERROR(CONN_STATUS_ERROR_INTERNAL,
                                                    "LABEL[%s]: Error: Couldn't add socket file descriptor\n",
                                                    m_label.c_str());
                                        return NC_STATUS_ERROR;
                                }
                        }
                        // Return here -still in connecting state
                        return NC_STATUS_OK;
                }
                case EADDRNOTAVAIL:
                {
                        // ---------------------------------
                        // Retry connect
                        // TODO -bad to spin like this???
                        // ---------------------------------
                        if (++l_retry_connect_count < 1024)
                        {
                                usleep(1000);
                                goto state_top;
                        }
                        else
                        {
                                NCONN_ERROR(CONN_STATUS_ERROR_INTERNAL,
                                            "LABEL[%s]: Error connect().  Reason: %s\n",
                                            m_label.c_str(), ::strerror(errno));
                                return NC_STATUS_ERROR;
                        }
                        break;
                }
                default:
                {
                        NCONN_ERROR(CONN_STATUS_ERROR_INTERNAL,
                                    "LABEL[%s]: Error Unkown. Reason: %s\n",
                                    m_label.c_str(), ::strerror(errno));
                        return NC_STATUS_ERROR;
                }
                }
        }
        // -------------------------------------------------
        // Set to connected state
        // -------------------------------------------------
        m_tcp_state = TCP_STATE_CONNECTED;
        // -------------------------------------------------
        // TODO Stats???
        // -------------------------------------------------
        //if (m_collect_stats_flag)
        //{
        //        m_stat.m_tt_connect_us = get_delta_time_us(m_connect_start_time_us);
        //        // Save last connect time for reuse
        //        m_last_connect_time_us = m_stat.m_tt_connect_us;
        //}
        // Add to readable
        if (m_evr_loop)
        {
                int l_s;
                l_s = m_evr_loop->mod_fd(m_fd,
                                EVR_FILE_ATTR_MASK_READ |
                                EVR_FILE_ATTR_MASK_WRITE |
                                EVR_FILE_ATTR_MASK_STATUS_ERROR |
                                EVR_FILE_ATTR_MASK_RD_HUP |
                                EVR_FILE_ATTR_MASK_HUP |
                                EVR_FILE_ATTR_MASK_ET,
                                &m_evr_fd);
                if (l_s != STATUS_OK)
                {
                        NCONN_ERROR(CONN_STATUS_ERROR_INTERNAL,
                                    "LABEL[%s]: Error: Couldn't add socket file descriptor\n",
                                    m_label.c_str());
                        return NC_STATUS_ERROR;
                }
        }
        return NC_STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t nconn_tcp::nccleanup()
{
        // -------------------------------------------------
        // shut down connection
        // -------------------------------------------------
        if (m_evr_loop)
        {
                m_evr_loop->del_fd(m_fd);
        }
        if (m_fd > 0)
        {
                if (m_sock_opt_no_linger)
                {
#if defined(__linux__)
                        shutdown(m_fd, SHUT_RDWR);
                        struct linger l_l;
                        l_l.l_onoff = 1;
                        l_l.l_linger = 0;
                        // TODO check error
                        setsockopt(m_fd, SOL_SOCKET, SO_LINGER, (char *)&l_l, sizeof(linger));
#endif
                }
                close(m_fd);
        }
        //m_evr_fd.m_magic = 0;
        m_fd = -1;
        m_evr_loop = NULL;
        m_tcp_state = TCP_STATE_NONE;
        m_conn_status = CONN_STATUS_CANCELLED;
        // Reset all the values
        // TODO Make init function...
        // Set num use back to zero -we need reset method here?
        m_num_reqs = 0;
        return NC_STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! nconn_utils
//! ----------------------------------------------------------------------------
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int nconn_get_fd(nconn &a_nconn)
{
        int *l_fd;
        uint32_t l_len;
        int l_s;
        l_s = a_nconn.get_opt(nconn_tcp::OPT_TCP_FD, (void **)&l_fd, &l_len);
        if (l_s != nconn::NC_STATUS_OK)
        {
                return -1;
        }
        return *l_fd;
}
} //namespace ns_is2 {
