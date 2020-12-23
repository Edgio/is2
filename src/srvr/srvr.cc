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
#include "dns/nresolver.h"
#include "support/tls_util.h"
#include "nconn/nconn_tls.h"
#include "is2/srvr/srvr.h"
#include "is2/srvr/lsnr.h"
#include "is2/support/ndebug.h"
#include "is2/support/trace.h"
#include "is2/status.h"
#include "srvr/t_srvr.h"
#include <openssl/ssl.h>
//! ----------------------------------------------------------------------------
//! macros
//! ----------------------------------------------------------------------------
#define JS_ADD_MEMBER(_obj, _key, _val, _l_js_alloc)\
_obj.AddMember(_key,\
                rapidjson::Value(_val, _l_js_alloc).Move(),\
                _l_js_alloc)
namespace ns_is2 {
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
srvr::srvr(void):
        m_t_conf(NULL),
        m_num_threads(1),
        m_lsnr_list(),
        m_dns_use_ai_cache(true),
        m_dns_ai_cache_file(NRESOLVER_DEFAULT_AI_CACHE_FILE),
        m_t_srvr_list(),
        m_is_initd(false),
        m_start_time_ms(0),
        m_stat_mutex(),
        m_stat_update_ms(S_STAT_UPDATE_MS_DEFAULT),
        m_stat_last_ms(0),
        m_stat_last(),
        m_stat_calc_last()
{
        m_t_conf = new t_conf();
        pthread_mutex_init(&m_stat_mutex, NULL);
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
srvr::~srvr()
{
        // -------------------------------------------------
        // server list
        // -------------------------------------------------
        for(t_srvr_list_t::iterator i_t = m_t_srvr_list.begin();
                        i_t != m_t_srvr_list.end();
                        ++i_t)
        {
                if(*i_t)
                {
                        delete *i_t;
                }
        }
        m_t_srvr_list.clear();
        // -------------------------------------------------
        // listener list
        // -------------------------------------------------
        for(lsnr_list_t::iterator i_t = m_lsnr_list.begin();
            i_t != m_lsnr_list.end();
            ++i_t)
        {
                if(*i_t)
                {
                        delete *i_t;
                }
        }
        m_lsnr_list.clear();
        // -------------------------------------------------
        // tls cleanup
        // -------------------------------------------------
        if(m_t_conf->m_tls_server_ctx)
        {
                SSL_CTX_free(m_t_conf->m_tls_server_ctx);
                m_t_conf->m_tls_server_ctx = NULL;
        }
        if(m_t_conf->m_tls_client_ctx)
        {
                SSL_CTX_free(m_t_conf->m_tls_client_ctx);
                m_t_conf->m_tls_client_ctx = NULL;
        }
        if(m_t_conf->m_nresolver)
        {
                delete m_t_conf->m_nresolver;
                m_t_conf->m_nresolver = NULL;
        }
        if(m_t_conf)
        {
            delete m_t_conf;
            m_t_conf = NULL;
        }
        pthread_mutex_destroy(&m_stat_mutex);
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
nresolver *srvr::get_nresolver(void)
{
        return m_t_conf->m_nresolver;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
bool srvr::get_dns_use_sync(void)
{
        return m_t_conf->m_dns_use_sync;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
bool srvr::is_running(void)
{
        for (t_srvr_list_t::iterator i_t = m_t_srvr_list.begin();
                        i_t != m_t_srvr_list.end();
                        ++i_t)
        {
                if((*i_t)->is_running())
                {
                        return true;
                }
        }
        return false;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
void srvr::set_num_threads(uint32_t a_num_threads)
{
        m_num_threads = a_num_threads;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
void srvr::set_num_parallel(uint32_t a_num_parallel)
{
        m_t_conf->m_num_parallel = a_num_parallel;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
void srvr::set_num_reqs_per_conn(int32_t a_num_reqs_per_conn)
{
        m_t_conf->m_num_reqs_per_conn = a_num_reqs_per_conn;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
void srvr::set_stat_update_ms(uint32_t a_update_ms)
{
        m_t_conf->m_stat_update_ms = a_update_ms;
}
//! ----------------------------------------------------------------------------
//! \details: Aggregate thread stats.
//! \return:  NA
//! \param:   TODO
//! ----------------------------------------------------------------------------
static void aggregate_stat(t_stat_cntr_t &ao_total, const t_stat_cntr_t &a_stat)
{
        // -------------------------------------------------
        // client
        // -------------------------------------------------
        ao_total.m_reqs += a_stat.m_reqs;
        ao_total.m_bytes_read += a_stat.m_bytes_read;
        ao_total.m_bytes_written += a_stat.m_bytes_written;
        ao_total.m_resp_status_2xx += a_stat.m_resp_status_2xx;
        ao_total.m_resp_status_3xx += a_stat.m_resp_status_3xx;
        ao_total.m_resp_status_4xx += a_stat.m_resp_status_4xx;
        ao_total.m_resp_status_5xx += a_stat.m_resp_status_5xx;
        // -------------------------------------------------
        // upstream
        // -------------------------------------------------
        ao_total.m_upsv_reqs += a_stat.m_upsv_reqs;
        ao_total.m_upsv_bytes_read += a_stat.m_upsv_bytes_read;
        ao_total.m_upsv_bytes_written += a_stat.m_upsv_bytes_written;
        ao_total.m_upsv_resp_status_2xx += a_stat.m_upsv_resp_status_2xx;
        ao_total.m_upsv_resp_status_3xx += a_stat.m_upsv_resp_status_3xx;
        ao_total.m_upsv_resp_status_4xx += a_stat.m_upsv_resp_status_4xx;
        ao_total.m_upsv_resp_status_5xx += a_stat.m_upsv_resp_status_5xx;
}
//! ----------------------------------------------------------------------------
//! \details: Get srvr stats
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
void srvr::get_stat(t_stat_cntr_t &ao_stat, t_stat_calc_t &ao_calc_stat, bool a_no_cache)
{
        pthread_mutex_lock(&m_stat_mutex);
        uint64_t l_cur_time_ms = get_time_ms();
        uint64_t l_delta_ms = l_cur_time_ms - m_stat_last_ms;
        // -------------------------------------------------
        // check if can return cache
        // -------------------------------------------------
        if(!a_no_cache &&
          (l_delta_ms < m_stat_update_ms))
        {
                ao_stat = m_stat_last;
                ao_calc_stat = m_stat_calc_last;
                pthread_mutex_unlock(&m_stat_mutex);
                return;
        }
        // -------------------------------------------------
        // aggregate
        // -------------------------------------------------
        ao_stat.clear();
        ao_calc_stat.clear();
        t_stat_cntr_list_t l_stat_list;
        for(t_srvr_list_t::const_iterator i_client = m_t_srvr_list.begin();
           i_client != m_t_srvr_list.end();
           ++i_client)
        {
                // Get stuff from client...
                t_stat_cntr_t l_stat;
                int32_t l_s;
                l_s = (*i_client)->get_stat(l_stat);
                if(l_s != STATUS_OK)
                {
                        // TODO -do nothing...
                        continue;
                }
                l_stat_list.emplace(l_stat_list.begin(), l_stat);
                aggregate_stat(ao_stat, l_stat);
        }
        // -------------------------------------------------
        // client calc'd stats
        // -------------------------------------------------
        ao_calc_stat.m_req_delta = ao_stat.m_reqs - m_stat_last.m_reqs;
        if(ao_calc_stat.m_req_delta > 0)
        {
                ao_calc_stat.m_resp_status_2xx_pcnt = 100.0*((float)(ao_stat.m_resp_status_2xx - m_stat_last.m_resp_status_2xx))/((float)ao_calc_stat.m_req_delta);
                ao_calc_stat.m_resp_status_3xx_pcnt = 100.0*((float)(ao_stat.m_resp_status_3xx - m_stat_last.m_resp_status_3xx))/((float)ao_calc_stat.m_req_delta);
                ao_calc_stat.m_resp_status_4xx_pcnt = 100.0*((float)(ao_stat.m_resp_status_4xx - m_stat_last.m_resp_status_4xx))/((float)ao_calc_stat.m_req_delta);
                ao_calc_stat.m_resp_status_5xx_pcnt = 100.0*((float)(ao_stat.m_resp_status_5xx - m_stat_last.m_resp_status_5xx))/((float)ao_calc_stat.m_req_delta);
        }
        if(l_delta_ms > 0)
        {
                ao_calc_stat.m_req_s = ((float)ao_calc_stat.m_req_delta*1000)/((float)l_delta_ms);
                ao_calc_stat.m_bytes_read_s = ((float)((ao_stat.m_bytes_read - m_stat_last.m_bytes_read)*1000))/((float)l_delta_ms);
                ao_calc_stat.m_bytes_write_s = ((float)((ao_stat.m_bytes_written - m_stat_last.m_bytes_written)*1000))/((float)l_delta_ms);
        }
        // -------------------------------------------------
        // upstream calc'd stats
        // -------------------------------------------------
        ao_calc_stat.m_upsv_req_delta = ao_stat.m_upsv_reqs - m_stat_last.m_upsv_reqs;
        if(ao_calc_stat.m_upsv_req_delta > 0)
        {
                ao_calc_stat.m_upsv_resp_status_2xx_pcnt = 100.0*((float)(ao_stat.m_upsv_resp_status_2xx - m_stat_last.m_upsv_resp_status_2xx))/((float)ao_calc_stat.m_upsv_req_delta);
                ao_calc_stat.m_upsv_resp_status_3xx_pcnt = 100.0*((float)(ao_stat.m_upsv_resp_status_3xx - m_stat_last.m_upsv_resp_status_3xx))/((float)ao_calc_stat.m_upsv_req_delta);
                ao_calc_stat.m_upsv_resp_status_4xx_pcnt = 100.0*((float)(ao_stat.m_upsv_resp_status_4xx - m_stat_last.m_upsv_resp_status_4xx))/((float)ao_calc_stat.m_upsv_req_delta);
                ao_calc_stat.m_upsv_resp_status_5xx_pcnt = 100.0*((float)(ao_stat.m_upsv_resp_status_5xx - m_stat_last.m_upsv_resp_status_5xx))/((float)ao_calc_stat.m_upsv_req_delta);
        }
        if(l_delta_ms > 0)
        {
                ao_calc_stat.m_upsv_req_s = ((float)ao_calc_stat.m_upsv_req_delta*1000)/((float)l_delta_ms);
                ao_calc_stat.m_upsv_bytes_read_s = ((float)((ao_stat.m_upsv_bytes_read - m_stat_last.m_upsv_bytes_read)*1000))/((float)l_delta_ms);
                ao_calc_stat.m_upsv_bytes_write_s = ((float)((ao_stat.m_upsv_bytes_written - m_stat_last.m_upsv_bytes_written)*1000))/((float)l_delta_ms);
        }
        // -------------------------------------------------
        // copy
        // -------------------------------------------------
        m_stat_last = ao_stat;
        m_stat_calc_last = ao_calc_stat;
        m_stat_last_ms = l_cur_time_ms;
        pthread_mutex_unlock(&m_stat_mutex);
}
//! ----------------------------------------------------------------------------
//! \details: Global timeout for connect/read/write
//! \return:  NA
//! \param:   a_val: timeout in seconds
//! ----------------------------------------------------------------------------
void srvr::set_timeout_ms(uint32_t a_val)
{
        m_t_conf->m_timeout_ms = a_val;
}
//! ----------------------------------------------------------------------------
//! \details: Set response done callback
//! \return:  NA
//! \param:   a_name: server name
//! ----------------------------------------------------------------------------
void srvr::set_resp_done_cb(resp_done_cb_t a_cb)
{
        m_t_conf->m_resp_done_cb = a_cb;
}
//! ----------------------------------------------------------------------------
//! \details: Set server name -used for requests/response headers
//! \return:  NA
//! \param:   a_name: server name
//! ----------------------------------------------------------------------------
void srvr::set_server_name(const std::string &a_name)
{
        m_t_conf->m_server_name = a_name;
}
//! ----------------------------------------------------------------------------
//! \details: Get server name -used for requests/response headers
//! \return:  server name
//! \param:   NA
//! ----------------------------------------------------------------------------
const std::string &srvr::get_server_name(void) const
{
        return m_t_conf->m_server_name;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
void srvr::set_dns_use_sync(bool a_val)
{
        m_t_conf->m_dns_use_sync = a_val;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
void srvr::set_dns_use_ai_cache(bool a_val)
{
        m_dns_use_ai_cache = a_val;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
void srvr::set_dns_ai_cache_file(const std::string &a_file)
{
        m_dns_ai_cache_file = a_file;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
void srvr::set_tls_server_ctx_cipher_list(const std::string &a_cipher_list)
{
        m_t_conf->m_tls_server_ctx_cipher_list = a_cipher_list;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int srvr::set_tls_server_ctx_options(const std::string &a_tls_options_str)
{
        int32_t l_status;
        l_status = get_tls_options_str_val(a_tls_options_str,
                                                    m_t_conf->m_tls_server_ctx_options);
        if(l_status != STATUS_OK)
        {
                return STATUS_ERROR;
        }
        return STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int srvr::set_tls_server_ctx_options(long a_tls_options)
{
        m_t_conf->m_tls_server_ctx_options = a_tls_options;
        return STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
void srvr::set_tls_server_ctx_key(const std::string &a_tls_key)
{
        m_t_conf->m_tls_server_ctx_key = a_tls_key;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
void srvr::set_tls_server_ctx_crt(const std::string &a_tls_crt)
{
        m_t_conf->m_tls_server_ctx_crt = a_tls_crt;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
void srvr::set_tls_client_ctx_cipher_list(const std::string &a_cipher_list)
{
        m_t_conf->m_tls_client_ctx_cipher_list = a_cipher_list;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
void srvr::set_tls_client_ctx_ca_path(const std::string &a_tls_ca_path)
{
        m_t_conf->m_tls_client_ctx_ca_path = a_tls_ca_path;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
void srvr::set_tls_client_ctx_ca_file(const std::string &a_tls_ca_file)
{
        m_t_conf->m_tls_client_ctx_ca_file = a_tls_ca_file;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int srvr::set_tls_client_ctx_options(const std::string &a_tls_options_str)
{
        int32_t l_status;
        l_status = get_tls_options_str_val(a_tls_options_str,
                                                    m_t_conf->m_tls_client_ctx_options);
        if(l_status != STATUS_OK)
        {
                return STATUS_ERROR;
        }
        return STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int srvr::set_tls_client_ctx_options(long a_tls_options)
{
        m_t_conf->m_tls_client_ctx_options = a_tls_options;
        return STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! Running
//! ----------------------------------------------------------------------------
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t srvr::register_lsnr(lsnr *a_lsnr)
{
        if(!a_lsnr)
        {
                return STATUS_ERROR;
        }
        m_lsnr_list.push_back(a_lsnr);
        return STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
srvr::t_srvr_list_t &srvr::get_t_srvr_list(void)
{
        return m_t_srvr_list;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t srvr::run(void)
{
        int32_t l_s = 0;
        if(!m_is_initd)
        {
                l_s = init();
                if(STATUS_OK != l_s)
                {
                        return STATUS_ERROR;
                }
        }
        if(m_num_threads == 0)
        {
                (*(m_t_srvr_list.begin()))->t_run(NULL);
        }
        else
        {
                for(t_srvr_list_t::iterator i_t = m_t_srvr_list.begin();
                                i_t != m_t_srvr_list.end();
                                ++i_t)
                {
                        (*i_t)->run();
                }
        }
        return STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int srvr::stop(void)
{
        int32_t l_retval = STATUS_OK;
        for (t_srvr_list_t::iterator i_t = m_t_srvr_list.begin();
                        i_t != m_t_srvr_list.end();
                        ++i_t)
        {
                (*i_t)->stop();
        }
        return l_retval;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   TODO
//! ----------------------------------------------------------------------------
int32_t srvr::wait_till_stopped(void)
{
        // Join all threads before exit
        for(t_srvr_list_t::iterator i_server = m_t_srvr_list.begin();
           i_server != m_t_srvr_list.end();
            ++i_server)
        {
                pthread_join(((*i_server)->m_t_run_thread), NULL);
        }
        return STATUS_OK;
}
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  client status indicating success or failure
//! \param:   TODO
//! ----------------------------------------------------------------------------
int srvr::init(void)
{
        if(m_is_initd)
        {
                return STATUS_OK;
        }
        // -------------------------------------------------
        // init resolver with cache
        // -------------------------------------------------
        m_t_conf->m_nresolver = new nresolver();
        int32_t l_ldb_init_status;
        l_ldb_init_status = m_t_conf->m_nresolver->init(m_dns_use_ai_cache, m_dns_ai_cache_file);
        if(STATUS_OK != l_ldb_init_status)
        {
                TRC_ERROR("Error performing resolver init with ai_cache: %s\n",
                           m_dns_ai_cache_file.c_str());
                return STATUS_ERROR;
        }
        // -------------------------------------------------
        // SSL init...
        // -------------------------------------------------
        signal(SIGPIPE, SIG_IGN);
        // -------------------------------------------------
        // SSL init...
        // -------------------------------------------------
        tls_init();
        std::string l_unused;
        // -------------------------------------------------
        // ssl server setup
        // -------------------------------------------------
        m_t_conf->m_tls_server_ctx = tls_init_ctx(
                  m_t_conf->m_tls_server_ctx_cipher_list, // ctx cipher list str
                  m_t_conf->m_tls_server_ctx_options,     // ctx options
                  l_unused,                               // ctx ca file
                  l_unused,                               // ctx ca path
                  true,                                   // is server?
                  m_t_conf->m_tls_server_ctx_key,         // tls key
                  m_t_conf->m_tls_server_ctx_crt,         // tls crt
                  true);                                  // force h1 -for now
        if(NULL == m_t_conf->m_tls_server_ctx)
        {
                TRC_ERROR("Error: performing ssl_init(server) with cipher_list: %s\n",
                           m_t_conf->m_tls_server_ctx_cipher_list.c_str());
                return STATUS_ERROR;
        }
        // -------------------------------------------------
        // ssl client setup
        // -------------------------------------------------
        m_t_conf->m_tls_client_ctx = tls_init_ctx(
                m_t_conf->m_tls_client_ctx_cipher_list, // ctx cipher list str
                m_t_conf->m_tls_client_ctx_options,     // ctx options
                m_t_conf->m_tls_client_ctx_ca_file,     // ctx ca file
                m_t_conf->m_tls_client_ctx_ca_path,     // ctx ca path
                false,                                  // is server?
                l_unused,                               // tls key
                l_unused,                               // tls crt
                true);                                  // force h1 -for now
        if(NULL == m_t_conf->m_tls_client_ctx)
        {
                TRC_ERROR("Error: performing ssl_init(client) with cipher_list: %s\n",
                           m_t_conf->m_tls_client_ctx_cipher_list.c_str());
                return STATUS_ERROR;
        }
        // -------------------------------------------------
        // init t svr list
        // -------------------------------------------------
        // -------------------------------------------------
        // 0 threads -make a single srvr
        // -------------------------------------------------
        uint32_t l_num_t_srvr = m_num_threads;
        if(!l_num_t_srvr)
        {
                l_num_t_srvr = 1;
        }
        // -------------------------------------------------
        // foreach thread -create t_srvr instance
        // -------------------------------------------------
        for(uint32_t i_server_idx = 0; i_server_idx < l_num_t_srvr; ++i_server_idx)
        {
                // -----------------------------------------
                // create t_srvr
                // -----------------------------------------
                t_srvr *l_t_srvr = new t_srvr(m_t_conf);
                l_t_srvr->set_srvr_instance(this);
                // -----------------------------------------
                // add listeners
                // -----------------------------------------
                int32_t l_s;
                for(lsnr_list_t::iterator i_t = m_lsnr_list.begin();
                                i_t != m_lsnr_list.end();
                                ++i_t)
                {
                        if(!(*i_t))
                        {
                                continue;
                        }
                        l_s = l_t_srvr->add_lsnr(*(*i_t));
                        if(l_s != STATUS_OK)
                        {
                                delete l_t_srvr;
                                l_t_srvr = NULL;
                                TRC_ERROR("Error performing add_lsnr.\n");
                                return STATUS_ERROR;
                        }
                }
                // -----------------------------------------
                // init...
                // -----------------------------------------
                l_s = l_t_srvr->init();
                if(l_s != STATUS_OK)
                {
                        TRC_ERROR("Error performing init.\n");
                        return STATUS_ERROR;
                }
                m_t_srvr_list.push_back(l_t_srvr);
        }
        // -------------------------------------------------
        // done...
        // -------------------------------------------------
        m_is_initd = true;
        return STATUS_OK;
}
} //namespace ns_is2 {
