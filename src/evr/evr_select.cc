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
#include "evr/evr_select.h"
#include "is2/support/ndebug.h"
#include "is2/support/trace.h"
#include "is2/status.h"
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
namespace ns_is2 {
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
evr_select::evr_select(void) :
        m_conn_map(),
        m_rfdset(),
        m_wfdset()
{
        FD_ZERO(&m_rfdset);
        FD_ZERO(&m_wfdset);

        // Create ctrl fd
        int l_status = pipe(m_ctrl_fd);
        if(l_status == -1)
        {
                TRC_ERROR("pipe() failed: %s\n", strerror(errno));
                exit(-1);
        }
        FD_SET(m_ctrl_fd[0], &m_rfdset);
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int evr_select::wait(evr_events_t* a_ev, int a_max_events, int a_timeout_msec)
{
        //NDBG_PRINT("%swait%s = %d a_max_events = %d\n",
        //           ANSI_COLOR_FG_YELLOW, ANSI_COLOR_OFF,
        //           a_timeout_msec, a_max_events);
        fd_set l_rfdset = m_rfdset;
        fd_set l_wfdset = m_wfdset;
        struct timeval l_timeout;
        if(a_timeout_msec >= 0)
        {
                l_timeout.tv_sec = a_timeout_msec / 1000L;
                l_timeout.tv_usec = (a_timeout_msec % 1000L) * 1000L;
        }
        int l_s = 0;
        uint32_t l_fdsize = 1;
        if(m_conn_map.size())
        {
                l_fdsize = m_conn_map.rbegin()->first + 1;
        }
        // -------------------------------------------------
        // loop over EINTR
        // -------------------------------------------------
        do {
                errno = 0;
                l_s = select(l_fdsize,
                                 &l_rfdset,
                                 &l_wfdset,
                                 NULL,
                                 a_timeout_msec >= 0 ? &l_timeout : NULL);
        } while((l_s < 0) &&
                (errno == EINTR));
        //NDBG_PRINT("l_s: %d\n", l_s);
        if(l_s < 0)
        {
                //NDBG_PRINT("Error select() failed. Reason: %s\n", strerror(errno));
                return STATUS_ERROR;
        }
        if(l_s > a_max_events)
        {
                //NDBG_PRINT("Error select() returned too many events (got %d, expected <= %d)\n",
                //                l_s, a_max_events);
                return STATUS_ERROR;
        }
        if(FD_ISSET(m_ctrl_fd[0], &l_rfdset))
        {
                // NDBG_PRINT("Control fd fired, exiting\n");
                return STATUS_OK;
        }

        int l_p = 0;
        for(conn_map_t::iterator i_conn = m_conn_map.begin(); i_conn != m_conn_map.end(); ++i_conn)
        {
                int l_fd = i_conn->first;
                bool l_inset = false;
                if(FD_ISSET(l_fd, &l_wfdset))
                {
                        //NDBG_PRINT("INSET: EPOLLOUT fd: %d\n", l_fd);
                        a_ev[l_p].events |= EVR_EV_OUT;
                        l_inset = true;
                }
                if(FD_ISSET(l_fd, &l_rfdset))
                {
                        //NDBG_PRINT("INSET: EPOLLIN fd: %d\n", l_fd);
                        a_ev[l_p].events |= EVR_EV_IN;
                        l_inset = true;
                }
                if(l_inset)
                {
                        //NDBG_PRINT("INSET: fd: %d\n", l_fd);
                        a_ev[l_p].data.ptr = i_conn->second;
                        ++l_p;
                        if(l_p > l_s)
                        {
                                //NDBG_PRINT("Error num events exceeds select result.\n");
                                return STATUS_ERROR;
                        }
                }
        }
        //NDBG_PRINT("%swait%s = %d\n",
        //           ANSI_COLOR_BG_YELLOW, ANSI_COLOR_OFF,
        //           l_p);
        return l_p;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int evr_select::add(int a_fd, uint32_t a_attr_mask, evr_fd_t *a_evr_fd_event)
{
        //NDBG_PRINT("%sadd%s: fd: %d --attr: 0x%08X --data: %p\n",
        //           ANSI_COLOR_BG_BLUE, ANSI_COLOR_OFF,
        //           a_fd, a_attr_mask, a_data);
        m_conn_map[a_fd] = a_evr_fd_event;
        mod(a_fd, a_attr_mask, a_evr_fd_event);
        return STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int evr_select::mod(int a_fd, uint32_t a_attr_mask, evr_fd_t *a_evr_fd_event)
{
        //NDBG_PRINT("%smod%s: fd: %d --attr: 0x%08X\n",
        //           ANSI_COLOR_BG_GREEN, ANSI_COLOR_OFF,
        //           a_fd, a_attr_mask);
        FD_CLR(a_fd, &m_wfdset);
        FD_CLR(a_fd, &m_rfdset);
        if(a_attr_mask & EVR_FILE_ATTR_MASK_READ)         { FD_SET(a_fd, &m_rfdset); }
        if(a_attr_mask & EVR_FILE_ATTR_MASK_RD_HUP)       { FD_SET(a_fd, &m_rfdset); }
        if(a_attr_mask & EVR_FILE_ATTR_MASK_HUP)          { FD_SET(a_fd, &m_rfdset); }
        if(a_attr_mask & EVR_FILE_ATTR_MASK_STATUS_ERROR) { FD_SET(a_fd, &m_rfdset); }
        if(a_attr_mask & EVR_FILE_ATTR_MASK_WRITE)        { FD_SET(a_fd, &m_wfdset); }
        return STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int evr_select::del(int a_fd)
{
        //NDBG_PRINT("%sdel%s: fd: %d\n",
        //           ANSI_COLOR_BG_RED, ANSI_COLOR_OFF,
        //           a_fd);
        m_conn_map.erase(a_fd);
        FD_CLR(a_fd, &m_rfdset);
        FD_CLR(a_fd, &m_wfdset);
        return STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int evr_select::signal(void)
{
        // Wake up select by writing to control fd pipe
        uint64_t l_value = 1;
        ssize_t l_write_status = 0;
        //NDBG_PRINT("WRITING m_ctrl_fd: %d\n", m_ctrl_fd[1]);
        l_write_status = write(m_ctrl_fd[1], &l_value, sizeof (l_value));
        if(l_write_status == -1)
        {
                //NDBG_PRINT("l_write_status: %ld\n", l_write_status);
                return STATUS_ERROR;
        }
        return STATUS_OK;
}
} //namespace ns_is2 {
