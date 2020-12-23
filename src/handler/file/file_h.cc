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
#include "is2/support/os.h"
#include "srvr/t_srvr.h"
#include "handler/file/mime_types.h"
#include "is2/support/string_util.h"
#include "is2/support/ndebug.h"
#include "is2/support/nbq.h"
#include "is2/status.h"
#include "is2/support/trace.h"
#include "is2/handler/file_h.h"
#include "is2/srvr/rqst.h"
#include "is2/srvr/session.h"
#include "is2/srvr/api_resp.h"
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
namespace ns_is2 {
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
file_h::file_h(void):
        default_rqst_h(),
        m_root(),
        m_index("index.html"),
        m_route()
{
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
file_h::~file_h(void)
{
}
//! ----------------------------------------------------------------------------
//! Handler
//! ----------------------------------------------------------------------------
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
h_resp_t file_h::do_get(session &a_session, rqst &a_rqst, const url_pmap_t &a_url_pmap)
{
        // GET
        std::string l_path;
        int32_t l_s;
        std::string l_url_path_str;
        const data_t &l_url_path = a_rqst.get_url_path();
        l_url_path_str.assign(l_url_path.m_data, l_url_path.m_len);
        l_s = get_path(l_path,
                       m_route,
                       l_url_path_str);
        if(l_s != STATUS_OK)
        {
                return H_RESP_CLIENT_ERROR;
        }
        if(l_path.empty() ||
          (l_path == "/"))
        {
                if(!m_index.empty())
                {
                        l_path = "/" + m_index;
                }
                else
                {
                        return H_RESP_CLIENT_ERROR;
                }
        }
        if(!m_root.empty())
        {
                l_path = m_root + l_path;
        }
        else
        {
                l_path = "." + l_path;
        }
        return get_file(a_session, a_rqst, l_path);
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t file_h::set_root(const std::string &a_root)
{
        m_root = a_root;
        return STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t file_h::set_index(const std::string &a_index)
{
        m_index = a_index;
        return STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t file_h::set_route(const std::string &a_route)
{
        m_route = a_route;
        return STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
h_resp_t file_h::get_file(session &a_session,
                          rqst &a_rqst,
                          const std::string &a_path)
{
        if(!a_session.m_out_q)
        {
                a_session.m_out_q = a_session.m_t_srvr.get_nbq(NULL);
                if(!a_session.m_out_q)
                {
                        TRC_ERROR("a_session.m_out_q\n");
                        return send_internal_server_error(a_session, a_rqst.m_supports_keep_alives);
                }
        }
        // Make relative...
        file_u *l_fs = new file_u(a_session);
        int32_t l_s;
        l_s = l_fs->fsinit(a_path.c_str());
        if(l_s != STATUS_OK)
        {
                delete l_fs;
                l_fs = NULL;
                // TODO -use status code to determine is actual 404
                return send_not_found(a_session, a_rqst.m_supports_keep_alives);
        }
        a_session.m_u = l_fs;
        nbq &l_q = *(a_session.m_out_q);
        // ---------------------------------------
        // Write headers
        // ---------------------------------------
        nbq_write_status(l_q, HTTP_STATUS_OK);
        nbq_write_header(l_q, "Server", a_session.m_t_srvr.get_server_name().c_str());
        nbq_write_header(l_q, "Date", get_date_str());
        // Get extension
        std::string l_ext = get_file_ext(a_path);
        mime_types::ext_type_map_t::const_iterator i_m = mime_types::S_EXT_TYPE_MAP.find(l_ext);
        if(i_m != mime_types::S_EXT_TYPE_MAP.end())
        {
                nbq_write_header(l_q, "Content-type", i_m->second.c_str());
        }
        else
        {
                nbq_write_header(l_q, "Content-type", "text/html");
        }
        char l_length_str[32];
        sprintf(l_length_str, "%zu", l_fs->fssize());
        nbq_write_header(l_q, "Content-Length", l_length_str);
        // TODO get last modified for file...
        nbq_write_header(l_q, "Last-Modified", get_date_str());
        if(a_rqst.m_supports_keep_alives)
        {
                nbq_write_header(l_q, "Connection", "keep-alive");
        }
        else
        {
                nbq_write_header(l_q, "Connection", "close");
        }
        l_q.write("\r\n", strlen("\r\n"));
        // -------------------------------------------------
        // check for zero bytes
        // -------------------------------------------------
        if(l_fs->fssize() <= 0)
        {
                // all done; close the file.
                l_fs->set_ups_done();
                int32_t l_s;
                l_s = a_session.queue_output();
                if(l_s != STATUS_OK)
                {
                        TRC_ERROR("performing queue_output\n");
                        return H_RESP_SERVER_ERROR;
                }
                return H_RESP_DONE;
        }
        // -------------------------------------------------
        // Read up to 64k
        // -------------------------------------------------
        uint32_t l_read = 64*1024;
        if(l_read - l_q.read_avail() > 0)
        {
                l_read = l_read - l_q.read_avail();
        }
        if(l_fs->fssize() < l_read)
        {
                l_read = l_fs->fssize();
        }
        ssize_t l_ups_read_s;
        l_ups_read_s = l_fs->ups_read_ahead(l_read);
        if(l_ups_read_s < 0)
        {
                TRC_ERROR("performing ups_read\n");
        }
        return H_RESP_DONE;
}
//! ----------------------------------------------------------------------------
//! Upstream Object
//! ----------------------------------------------------------------------------
//! ----------------------------------------------------------------------------
//! \details: Constructor
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
file_u::file_u(session &a_session):
        base_u(a_session),
        m_fd(-1),
        m_size(0),
        m_read(0)
{
}
//! ----------------------------------------------------------------------------
//! \details: Destructor
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
file_u::~file_u()
{
        if(m_fd > 0)
        {
                close(m_fd);
                m_fd = -1;
        }
}
//! ----------------------------------------------------------------------------
//! \details: Setup file for reading
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t file_u::fsinit(const char *a_filename)
{
        // ---------------------------------------
        // Check is a file
        // TODO
        // ---------------------------------------
        int32_t l_s = STATUS_OK;
        // Open the file
        //NDBG_PRINT("a_filename: %s\n", a_filename);
        m_fd = open(a_filename, O_RDONLY|O_NONBLOCK);
        if (m_fd < 0)
        {
                TRC_ERROR("performing open on file: %s.  Reason: %s\n", a_filename, strerror(errno));
                return STATUS_ERROR;
        }
        struct stat l_stat;
        l_s = fstat(m_fd, &l_stat);
        if(l_s != 0)
        {
                TRC_ERROR("performing stat on file: %s.  Reason: %s\n", a_filename, strerror(errno));
                if(m_fd > 0)
                {
                        close(m_fd);
                        m_fd = -1;
                }
                return STATUS_ERROR;
        }
        // Check if is regular file
        if(!(l_stat.st_mode & S_IFREG))
        {
                TRC_ERROR("opening file: %s.  Reason: is NOT a regular file\n", a_filename);
                if(m_fd > 0)
                {
                        close(m_fd);
                        m_fd = -1;
                }
                return STATUS_ERROR;
        }
        // Set size
        m_size = l_stat.st_size;
        // Start sending it
        m_read = 0;
        m_state = UPS_STATE_SENDING;
        return STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: Read part from file
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
#if 0
ssize_t file_u::ups_read(char *ao_dst, size_t a_len)
{
        // Get one chunk of the file from disk
        ssize_t l_read = 0;
        l_read = read(m_fd, (void *)ao_dst, a_len);
        if (l_read == 0)
        {
                // All done; close the file.
                close(m_fd);
                m_fd = 0;
                m_state = DONE;
                return 0;
        }
        else if(l_read < 0)
        {
                //NDBG_PRINT("Error performing read. Reason: %s\n", strerror(errno));
                return STATUS_ERROR;
        }
        if(l_read > 0)
        {
                m_read += l_read;
        }
        return l_read;
}
#endif
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
ssize_t file_u::ups_read(size_t a_len)
{
        return 0;
}
//! ----------------------------------------------------------------------------
//! \details: Continue sending the file started by sendFile().
//!           Call this periodically. Returns nonzero when done.
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
ssize_t file_u::ups_read_ahead(size_t a_len)
{
        if(m_fd < 0)
        {
                return 0;
        }
        m_state = UPS_STATE_SENDING;
        if(!(m_session.m_out_q))
        {
                TRC_ERROR("m_session->m_out_q == NULL\n");
                return STATUS_ERROR;
        }
        // Get one chunk of the file from disk
        ssize_t l_read = 0;
        ssize_t l_last;
        size_t l_len_read = (a_len > (m_size - m_read))?(m_size - m_read): a_len;
        l_read = m_session.m_out_q->write_fd(m_fd, l_len_read, l_last);
        if(l_read < 0)
        {
                TRC_ERROR("performing read. Reason: %s\n", strerror(errno));
                return STATUS_ERROR;
        }
        m_read += l_read;
        //NDBG_PRINT("READ: B %9ld / %9lu / %9lu\n", l_len_read, m_read, m_size);
        if ((((size_t)l_read) < a_len) || (m_read >= m_size))
        {
                // All done; close the file.
                close(m_fd);
                m_fd = -1;
                m_state = UPS_STATE_DONE;
        }
        if(l_read > 0)
        {
                int32_t l_s;
                l_s = m_session.queue_output();
                if(l_s != STATUS_OK)
                {
                        TRC_ERROR("performing queue_output\n");
                        return STATUS_ERROR;
                }
        }
        return l_read;
}
//! ----------------------------------------------------------------------------
//! \details: Cancel and cleanup
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t file_u::ups_cancel(void)
{
        if(m_fd > 0)
        {
                close(m_fd);
                m_state = UPS_STATE_DONE;
                m_fd = -1;
        }
        return STATUS_OK;
}
} //namespace ns_is2 {
