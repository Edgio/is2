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
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)
# ------------------------------------------------------------------------------
# basic
# ------------------------------------------------------------------------------
ADD_EXECUTABLE(basic_cmd basic.cc)
ADD_DEPENDENCIES(basic_cmd is2)
SET_TARGET_PROPERTIES(basic_cmd PROPERTIES OUTPUT_NAME "basic")
TARGET_LINK_LIBRARIES(basic_cmd ${LIBRARIES} is2 pthread)
# ------------------------------------------------------------------------------
# files
# ------------------------------------------------------------------------------
ADD_EXECUTABLE(files_cmd files.cc)
ADD_DEPENDENCIES(files_cmd is2)
SET_TARGET_PROPERTIES(files_cmd PROPERTIES OUTPUT_NAME "files")
TARGET_LINK_LIBRARIES(files_cmd ${LIBRARIES} is2 pthread)
# ------------------------------------------------------------------------------
# proxy
# ------------------------------------------------------------------------------
ADD_EXECUTABLE(proxy_cmd proxy.cc)
ADD_DEPENDENCIES(proxy_cmd is2)
SET_TARGET_PROPERTIES(proxy_cmd PROPERTIES OUTPUT_NAME "proxy")
TARGET_LINK_LIBRARIES(proxy_cmd ${LIBRARIES} is2 pthread)
# ------------------------------------------------------------------------------
# with accesslog
# ------------------------------------------------------------------------------
ADD_EXECUTABLE(w_accesslog_cmd w_accesslog.cc)
ADD_DEPENDENCIES(w_accesslog_cmd is2)
SET_TARGET_PROPERTIES(w_accesslog_cmd PROPERTIES OUTPUT_NAME "w_accesslog")
TARGET_LINK_LIBRARIES(w_accesslog_cmd ${LIBRARIES} is2 pthread)
# ------------------------------------------------------------------------------
# subrequest
# ------------------------------------------------------------------------------
ADD_EXECUTABLE(subrequest_cmd subrequest.cc)
ADD_DEPENDENCIES(subrequest_cmd is2)
SET_TARGET_PROPERTIES(subrequest_cmd PROPERTIES OUTPUT_NAME "subrequest")
TARGET_LINK_LIBRARIES(subrequest_cmd ${LIBRARIES} is2 pthread)
# ------------------------------------------------------------------------------
# w_stat
# ------------------------------------------------------------------------------
ADD_EXECUTABLE(w_stat_cmd w_stat.cc)
ADD_DEPENDENCIES(w_stat_cmd is2)
SET_TARGET_PROPERTIES(w_stat_cmd PROPERTIES OUTPUT_NAME "w_stat")
TARGET_LINK_LIBRARIES(w_stat_cmd ${LIBRARIES} is2 pthread)
