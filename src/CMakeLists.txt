# ------------------------------------------------------------------------------
# Project setup
# ------------------------------------------------------------------------------
project(is2)
cmake_minimum_required(VERSION 2.6.0)
if (POLICY CMP0046)
    cmake_policy(SET CMP0046 NEW)
endif()
# ------------------------------------------------------------------------------
# include paths
# ------------------------------------------------------------------------------
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/ext
    ${CMAKE_SOURCE_DIR}/ext/udns-0.4
    ${CMAKE_SOURCE_DIR}/ext/nghttp2/include
    ${CMAKE_SOURCE_DIR}/ext/nghttp2/src
)
add_definitions(-DHAVE_CONFIG_H)
# ------------------------------------------------------------------------------
# udns source files
# ------------------------------------------------------------------------------
AUX_SOURCE_DIRECTORY(${CMAKE_SOURCE_DIR}/ext/nghttp2/src NGHTTP2_SRCS)
# ------------------------------------------------------------------------------
# udns source files
# ------------------------------------------------------------------------------
SET(UDNS_SRCS
    # -----------------------------------
    # udns
    # -----------------------------------
    ${CMAKE_SOURCE_DIR}/ext/udns-0.4/udns_dn.c
    ${CMAKE_SOURCE_DIR}/ext/udns-0.4/udns_dntosp.c
    ${CMAKE_SOURCE_DIR}/ext/udns-0.4/udns_parse.c
    ${CMAKE_SOURCE_DIR}/ext/udns-0.4/udns_resolver.c
    ${CMAKE_SOURCE_DIR}/ext/udns-0.4/udns_init.c
    ${CMAKE_SOURCE_DIR}/ext/udns-0.4/udns_misc.c
    ${CMAKE_SOURCE_DIR}/ext/udns-0.4/udns_XtoX.c
    ${CMAKE_SOURCE_DIR}/ext/udns-0.4/udns_rr_a.c
    ${CMAKE_SOURCE_DIR}/ext/udns-0.4/udns_rr_ptr.c
    ${CMAKE_SOURCE_DIR}/ext/udns-0.4/udns_rr_mx.c
    ${CMAKE_SOURCE_DIR}/ext/udns-0.4/udns_rr_txt.c
    ${CMAKE_SOURCE_DIR}/ext/udns-0.4/udns_bl.c
    ${CMAKE_SOURCE_DIR}/ext/udns-0.4/udns_rr_srv.c
    ${CMAKE_SOURCE_DIR}/ext/udns-0.4/udns_rr_naptr.c
    ${CMAKE_SOURCE_DIR}/ext/udns-0.4/udns_codes.c
    ${CMAKE_SOURCE_DIR}/ext/udns-0.4/udns_jran.c
    ${CMAKE_SOURCE_DIR}/ext/udns-0.4/udns_rr_ptr.c
)
# ------------------------------------------------------------------------------
# Source files
# ------------------------------------------------------------------------------
SET(SRCS
    # -----------------------------------
    # ext
    # -----------------------------------
    ${CMAKE_SOURCE_DIR}/ext/base64/base64.cc
    ${CMAKE_SOURCE_DIR}/ext/hostcheck/hostcheck.cc
    # -----------------------------------
    # http-parse
    # -----------------------------------
    ${CMAKE_SOURCE_DIR}/ext/http_parser/http_parser.cc
    # -----------------------------------
    # url_router
    # -----------------------------------
    url_router/url_router.cc
    # -----------------------------------
    # support
    # -----------------------------------
    support/file_util.cc
    support/nbq.cc
    support/ndebug.cc
    support/string_util.cc
    support/time_util.cc
    support/tls_util.cc
    support/trace.cc
    support/uri.cc
    support/date_util.cc
    support/os.cc
    # -----------------------------------
    # evr
    # -----------------------------------
    evr/evr.cc
    # TODO OS X support...
    #evr/evr_kqueue.cc
    evr/evr_select.cc
    # -----------------------------------
    # nconn
    # -----------------------------------
    nconn/host_info.cc
    nconn/nconn.cc
    nconn/nconn_tcp.cc
    nconn/nconn_tls.cc
    # -----------------------------------
    # dns
    # -----------------------------------
    dns/ai_cache.cc
    dns/nlookup.cc
    dns/nresolver.cc
    # -----------------------------------
    # srvr
    # -----------------------------------
    srvr/cb.cc
    srvr/hmsg.cc
    srvr/resp.cc
    srvr/rqst.cc
    srvr/lsnr.cc
    srvr/srvr.cc
    srvr/session.cc
    srvr/api_resp.cc
    srvr/rqst_h.cc
    srvr/default_rqst_h.cc
    srvr/t_srvr.cc
    srvr/access.cc
    srvr/base_u.cc
    srvr/nconn_pool.cc
    srvr/subr.cc
    srvr/ups_session.cc
    srvr/stat.cc
    # -----------------------------------
    # handlers
    # -----------------------------------
    handler/file/file_h.cc
    handler/proxy/proxy_h.cc
    handler/stat/stat_h.cc
)
# ------------------------------------------------------------------------------
# Headers
# ------------------------------------------------------------------------------
SET(HDRS
    ${CMAKE_SOURCE_DIR}/include/is2/status.h
    ${CMAKE_SOURCE_DIR}/include/is2/support/data.h
    ${CMAKE_SOURCE_DIR}/include/is2/support/time_util.h
    ${CMAKE_SOURCE_DIR}/include/is2/support/trace.h
    ${CMAKE_SOURCE_DIR}/include/is2/nconn/conn_status.h
    ${CMAKE_SOURCE_DIR}/include/is2/nconn/host_info.h
    ${CMAKE_SOURCE_DIR}/include/is2/nconn/scheme.h
    ${CMAKE_SOURCE_DIR}/include/is2/url_router/url_router.h
    ${CMAKE_SOURCE_DIR}/include/is2/srvr/access.h
    ${CMAKE_SOURCE_DIR}/include/is2/srvr/api_resp.h
    ${CMAKE_SOURCE_DIR}/include/is2/srvr/base_u.h
    ${CMAKE_SOURCE_DIR}/include/is2/srvr/cr.h
    ${CMAKE_SOURCE_DIR}/include/is2/srvr/default_rqst_h.h
    ${CMAKE_SOURCE_DIR}/include/is2/srvr/hmsg.h
    ${CMAKE_SOURCE_DIR}/include/is2/srvr/h_resp.h
    ${CMAKE_SOURCE_DIR}/include/is2/srvr/http_status.h
    ${CMAKE_SOURCE_DIR}/include/is2/srvr/lsnr.h
    ${CMAKE_SOURCE_DIR}/include/is2/srvr/resp.h
    ${CMAKE_SOURCE_DIR}/include/is2/srvr/rqst.h
    ${CMAKE_SOURCE_DIR}/include/is2/srvr/rqst_h.h
    ${CMAKE_SOURCE_DIR}/include/is2/srvr/session.h
    ${CMAKE_SOURCE_DIR}/include/is2/srvr/srvr.h
    ${CMAKE_SOURCE_DIR}/include/is2/srvr/stat.h
    ${CMAKE_SOURCE_DIR}/include/is2/srvr/subr.h
    ${CMAKE_SOURCE_DIR}/include/is2/handler/file_h.h
    ${CMAKE_SOURCE_DIR}/include/is2/handler/proxy_h.h
)
# ------------------------------------------------------------------------------
# properties
# ------------------------------------------------------------------------------
set_source_files_properties(${SRCS} PROPERTIES COMPILE_FLAGS -Weffc++)
set_source_files_properties(${SRCS} PROPERTIES COMPILE_FLAGS -Werror)
set_source_files_properties(${SRCS} PROPERTIES COMPILE_FLAGS -std=c++11)
# ------------------------------------------------
# Linux Specific
# ------------------------------------------------
IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    # Linux specific code
    SET(OperatingSystem "Linux")
    set (SRCS ${SRCS} ./evr/evr_epoll.cc)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
# ------------------------------------------------------------------------------
# build and install is2
# ------------------------------------------------------------------------------
ADD_LIBRARY(is2 ${HDRS} ${UDNS_SRCS} ${NGHTTP2_SRCS} ${SRCS})
SET_TARGET_PROPERTIES(is2 PROPERTIES SOVERSION 0)
SET_PROPERTY(TARGET is2 PROPERTY PUBLIC_HEADER ${INST_HDRS})
IF(${CMAKE_SYSTEM_NAME} MATCHES "LINUX")
   SET_PROPERTY(TARGET is2 PROPERTY FRAMEWORK ON)
   INSTALL(TARGETS is2 ARCHIVE DESTINATION lib COMPONENT Libraries)
   INSTALL( DIRECTORY ${CMAKE_SOURCE_DIR}/include/ DESTINATION include/ )
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "LINUX")

