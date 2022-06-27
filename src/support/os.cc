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
#include "is2/support/os.h"
#include "is2/support/trace.h"
#include "is2/status.h"
// ---------------------------------------------------------
// std libs
// ---------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
namespace ns_is2 {
//! ----------------------------------------------------------------------------
//! \details: get os path
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t get_path(std::string &ao_path,
                 const std::string &a_route,
                 const std::string &a_url_path)
{
        // disallow traversal
        if((a_url_path.find("./") != std::string::npos) ||
           (a_url_path.find("../") != std::string::npos))
        {
                TRC_WARN("requested path with traversal -path: %s\n", a_url_path.c_str());
                return STATUS_ERROR;
        }
        std::string l_route = a_route;
        if(a_route[a_route.length() - 1] == '*')
        {
                l_route = a_route.substr(0, a_route.length() - 2);
        }
        if(l_route.empty())
        {
                ao_path = a_url_path;
        }
        else if(a_url_path.find(l_route, 0) != std::string::npos)
        {
                ao_path = a_url_path.substr(l_route.length(), a_url_path.length() - l_route.length());
        }
        else
        {
                TRC_WARN("requested path with missing route: %s\n", l_route.c_str());
                return STATUS_ERROR;
        }
        return STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t read_file(const char *a_file, char **a_buf, size_t *a_len)
{
        struct stat l_stat;
        int32_t l_s = STATUS_OK;
        l_s = stat(a_file, &l_stat);
        if(l_s != 0)
        {
                TRC_ERROR("error performing stat on file: %s.  Reason: %s", a_file, strerror(errno));
                return STATUS_ERROR;
        }
        if(!(l_stat.st_mode & S_IFREG))
        {
                TRC_ERROR("error opening file: %s.  Reason: is NOT a regular file", a_file);
                return STATUS_ERROR;
        }
        FILE * l_file;
        l_file = fopen(a_file,"r");
        if (NULL == l_file)
        {
                TRC_ERROR("error opening file: %s.  Reason: %s", a_file, strerror(errno));
                return STATUS_ERROR;
        }
        int32_t l_size = l_stat.st_size;
        char *l_buf;
        l_buf = (char *)malloc(sizeof(char)*l_size+1);
        int32_t l_read_size;
        l_read_size = fread(l_buf, 1, l_size, l_file);
        if(l_read_size != l_size)
        {
                TRC_ERROR("error performing fread.  Reason: %s [%d:%d]", strerror(errno), l_read_size, l_size);
                return STATUS_ERROR;
        }
        l_buf[l_size] = '\0';
        l_s = fclose(l_file);
        if (l_s != 0)
        {
                TRC_ERROR("error performing fclose.  Reason: %s", strerror(errno));
                return STATUS_ERROR;
        }
        *a_buf = l_buf;
        *a_len = l_size;
        return STATUS_OK;
}

}
