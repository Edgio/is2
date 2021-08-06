//! ----------------------------------------------------------------------------
//! Copyright Verizon.
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
#include "is2/support/ndebug.h"
#include "is2/nconn/nconn.h"
#include "is2/support/nbq.h"
#include "is2/status.h"
#include "is2/support/time_util.h"
#include "is2/support/trace.h"
#include "is2/nconn/conn_status.h"
#include <errno.h>
#include <string.h>
#include <strings.h>
namespace ns_is2 {
//! ----------------------------------------------------------------------------
//! static
//! ----------------------------------------------------------------------------
ssl_accept_cb_t nconn::s_ssl_accept_cb = NULL;
void* nconn::s_ssl_accept_ctx = NULL;
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t nconn::nc_read(nbq *a_in_q, char **ao_buf, uint32_t &ao_read)
{
        ao_read = 0;
        if (!a_in_q)
        {
                TRC_ERROR("a_in_q == NULL\n");
                return NC_STATUS_ERROR;
        }
        // -------------------------------------------------
        // while connection is readable...
        //   read up to next read size
        //   if size read == read_q free size
        //     add block to queue
        // -------------------------------------------------
        int32_t l_s = 0;
        uint32_t l_read_size = 0;
        if (a_in_q->read_avail_is_max_limit())
        {
                return NC_STATUS_READ_UNAVAILABLE;
        }
        if (a_in_q->b_write_avail() <= 0)
        {
                int32_t l_s = a_in_q->b_write_add_avail();
                if (l_s <= 0)
                {
                        return NC_STATUS_ERROR;
                }
        }
        if ((a_in_q->get_max_read_queue() > 0) &&
           ((a_in_q->read_avail() + a_in_q->b_write_avail()) > (uint64_t)a_in_q->get_max_read_queue()))
        {
                l_read_size = a_in_q->get_max_read_queue() - a_in_q->read_avail();
        }
        else
        {
                l_read_size = a_in_q->b_write_avail();
        }
        char *l_buf = a_in_q->b_write_ptr();
        l_s = ncread(l_buf, l_read_size);
        if (l_s < 0)
        {
                switch(l_s)
                {
                case NC_STATUS_ERROR:
                case NC_STATUS_AGAIN:
                case NC_STATUS_OK:
                case NC_STATUS_EOF:
                {
                        return l_s;
                }
                default:
                {
                        break;
                }
                }
                //???
                return l_s;
        }
        else if (l_s == 0)
        {
                //???
                return l_s;
        }
        ao_read += l_s;
        *ao_buf = l_buf;
        a_in_q->b_write_incr(l_s);
        return NC_STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t nconn::nc_write(nbq *a_out_q, uint32_t &ao_written)
{
        ao_written = 0;
        if (!a_out_q)
        {
                TRC_ERROR("a_out_q == NULL\n");
                return NC_STATUS_ERROR;
        }
        if (!a_out_q->read_avail())
        {
                return NC_STATUS_OK;
        }
        // -------------------------------------------------
        // while connection is writeable...
        //   wrtie up to next write size
        //   if size write == write_q free size
        //     add
        // -------------------------------------------------
        int32_t l_s;
        l_s = ncwrite(a_out_q->b_read_ptr(), a_out_q->b_read_avail());
        if (l_s < 0)
        {
                switch(l_s)
                {
                case NC_STATUS_AGAIN:
                {
                        return l_s;
                }
                default:
                {
                        return NC_STATUS_ERROR;
                }
                }
                //???
                return l_s;
        }
        else if (l_s == 0)
        {
                //???
                return NC_STATUS_OK;
        }
        ao_written += l_s;
        // and not error?
        a_out_q->b_read_incr(l_s);
        a_out_q->shrink();
        return NC_STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
bool nconn::can_reuse(void)
{
        if (((m_num_reqs_per_conn == -1) ||
            (m_num_reqs < m_num_reqs_per_conn)))
        {
                return true;
        }
        else
        {
                return false;
        }
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t nconn::nc_set_listening(int32_t a_val)
{
        int32_t l_s;
        l_s = ncset_listening(a_val);
        if (l_s != NC_STATUS_OK)
        {
                return STATUS_ERROR;
        }
        m_nc_state = NC_STATE_LISTENING;
        return STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t nconn::nc_set_listening_nb(int32_t a_val)
{
        int32_t l_s;
        l_s = ncset_listening_nb(a_val);
        if (l_s != NC_STATUS_OK)
        {
                return STATUS_ERROR;
        }
        m_nc_state = NC_STATE_LISTENING;
        return STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t nconn::nc_set_accepting(int a_fd)
{
        int32_t l_s;
        l_s = ncset_accepting(a_fd);
        if (l_s != NC_STATUS_OK)
        {
                return STATUS_ERROR;
        }
        m_nc_state = NC_STATE_ACCEPTING;
        return STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t nconn::nc_set_connected(void)
{
        int32_t l_s;
        l_s = ncset_connected();
        if (l_s != NC_STATUS_OK)
        {
                return STATUS_ERROR;
        }
        m_nc_state = NC_STATE_CONNECTED;
        return STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t nconn::nc_cleanup()
{
        TRC_VERBOSE("tearing down: label: %s\n", m_label.c_str());
        int32_t l_s;
        l_s = nccleanup();
        m_nc_state = NC_STATE_FREE;
        m_num_reqs = 0;
        if (l_s != NC_STATUS_OK)
        {
                TRC_ERROR("Error performing nccleanup.\n");
                return STATUS_ERROR;
        }
        m_data = NULL;
        m_host_info_is_set = false;
        return STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
nconn::nconn(void):
      m_evr_loop(NULL),
      m_evr_fd(),
      m_scheme(SCHEME_NONE),
      m_label(),
      m_ctx(NULL),
      m_data(NULL),
      m_conn_status(CONN_STATUS_OK),
      m_last_error(""),
      m_host_data(NULL),
      m_host_info(),
      m_host_info_is_set(false),
      m_num_reqs_per_conn(-1),
      m_num_reqs(0),
      m_remote_sa(),
      m_remote_sa_len(0),
      m_nc_state(NC_STATE_FREE),
      m_id(0),
      m_idx(0),
      m_pool_id(0),
      m_alpn(ALPN_HTTP_VER_V1_1),
      m_alpn_buf(NULL),
      m_alpn_buf_len(0),
      m_timer_obj(NULL)
{
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
nconn::~nconn(void)
{
        if (m_alpn_buf)
        {
                free(m_alpn_buf);
                m_alpn_buf = NULL;
                m_alpn_buf_len = 0;
        }
}
//! ----------------------------------------------------------------------------
//! nconn_utils
//! ----------------------------------------------------------------------------
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
conn_status_t nconn_get_status(nconn &a_nconn)
{
        return a_nconn.get_status();
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
const std::string &nconn_get_last_error_str(nconn &a_nconn)
{
        return a_nconn.get_last_error();
}
} // ns_is2
