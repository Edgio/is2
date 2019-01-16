//: ----------------------------------------------------------------------------
//: \file:    url_router.h
//: \details: TODO
//: \author:  Reed P. Morrison
//: \date:    08/08/2015
//: ----------------------------------------------------------------------------
#ifndef _URL_ROUTER_H
#define _URL_ROUTER_H
//: ----------------------------------------------------------------------------
//: Includes
//: ----------------------------------------------------------------------------
#include <string>
#include <map>
#include <list>
#include <stdint.h>
namespace ns_is2 {
//: ----------------------------------------------------------------------------
//: Types
//: ----------------------------------------------------------------------------
#ifndef URL_PARAM_MAP_T
#define URL_PARAM_MAP_T
typedef std::map <std::string, std::string> url_pmap_t;
#endif
class node;
//: ----------------------------------------------------------------------------
//: url_router
//: ----------------------------------------------------------------------------
class url_router
{
public:
        // -------------------------------------------------
        // public methods
        // -------------------------------------------------
        url_router();
        ~url_router();
        // -------------------------------------------------
        // public members
        // -------------------------------------------------
        int32_t add_route(const std::string &a_route, const void *a_data);
        const void *find_route(const char *a_route,
                               uint32_t a_route_len,
                               url_pmap_t &ao_url_pmap);
        void display_trie(void);
private:
        // -------------------------------------------------
        // private methods
        // -------------------------------------------------
        // Disallow copy and assign
        url_router& operator=(const url_router &);
        url_router(const url_router &);
        // -------------------------------------------------
        // private members
        // -------------------------------------------------
        node *m_root_node;
};
} //namespace ns_is2 {
#endif // #ifndef _URL_ROUTER_H
