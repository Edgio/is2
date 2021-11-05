//! ----------------------------------------------------------------------------
//! Copyright Edgecast Inc.
//!
//! \file:    TODO
//! \details: TODO
//!
//! Licensed under the terms of the Apache 2.0 open source license.
//! Please refer to the LICENSE file in the project root for the terms.
//! ----------------------------------------------------------------------------
#ifndef _NCONN_POOL_H
#define _NCONN_POOL_H
//! ----------------------------------------------------------------------------
//! includes
//! ----------------------------------------------------------------------------
#include "is2/nconn/scheme.h"
#include "support/nlru.h"
#include <set>
//! ----------------------------------------------------------------------------
//! fwd decl's
//! ----------------------------------------------------------------------------
namespace ns_is2 {
class nconn;
//! ----------------------------------------------------------------------------
//! nconn_pool
//! ----------------------------------------------------------------------------
class nconn_pool
{
public:
        // -------------------------------------------------
        // Types
        // -------------------------------------------------
        typedef nlru <nconn> idle_conn_lru_t;
        typedef std::set<nconn *> nconn_set_t;
        typedef std::list <nconn *> nconn_list_t;
        typedef std::map <std::string, nconn_set_t> active_conn_map_t;
        // -------------------------------------------------
        // Public methods
        // -------------------------------------------------
        nconn_pool(uint64_t a_max_active_size,
                   uint64_t a_max_idle_size);
        ~nconn_pool();
        void evict_all_idle(void);
        nconn * get_new_active(const std::string &a_label, scheme_t a_scheme);
        uint64_t get_active_size(void);
        uint64_t get_active_available(void);
        uint64_t get_active_label(const std::string &a_label);
        nconn * get_idle(const std::string &a_label);
        uint64_t get_idle_size(void);
        int32_t add_idle(nconn *a_nconn);
        int32_t release(nconn *a_nconn);
        void reap(void);
        // accessors
        const active_conn_map_t &get_active_conn_map(void)
        {
                return m_active_conn_map;
        }
        const idle_conn_lru_t &get_idle_conn_lru(void)
        {
                return m_idle_conn_lru;
        }
        // -------------------------------------------------
        // Public static methods
        // -------------------------------------------------
        static nconn *s_create_new_conn(scheme_t a_scheme);
        static int s_delete_cb(void* o_1, void *a_2);
private:
        // -------------------------------------------------
        // Private methods
        // -------------------------------------------------
        nconn_pool& operator=(const nconn_pool &);
        nconn_pool(const nconn_pool &);
        void init(void);
        int32_t add_active(nconn *a_nconn);
        int32_t remove_active(nconn *a_nconn);
        int32_t remove_idle(nconn *a_nconn);
        int32_t cleanup(nconn *a_nconn);
        // -------------------------------------------------
        // Private members
        // -------------------------------------------------
        bool m_initd;
        // Active connections
        active_conn_map_t m_active_conn_map;
        uint64_t m_active_conn_map_size;
        uint64_t m_active_conn_map_max_size;
        // Idle connections
        idle_conn_lru_t m_idle_conn_lru;
        // Reap list
        nconn_list_t m_reap_list;

};
} //namespace ns_is2 {
#endif
