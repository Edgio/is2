# ----------------------------------------------------------------------------
# Copyright (C) 2018 Verizon.  All Rights Reserved.
# All Rights Reserved
#
#   Author: Reed P Morrison
#   Date:   09/30/2015  
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#
# ------------------------------------------------------------------------------
# Project setup
project(is2)
cmake_minimum_required(VERSION 2.8.0)
# ------------------------------------------------
# wb_nconn_pool
# ------------------------------------------------
SET(SRCS wb_nconn_pool.cc)
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/src)
add_executable(wb_nconn_pool ${HDRS} ${SRCS})
add_dependencies(wb_nconn_pool is2)
target_link_libraries(wb_nconn_pool compiled_catch ${CMAKE_BINARY_DIR}/src/libis2.a ${LIBRARIES} ssl crypto pthread)
SET_TARGET_PROPERTIES(wb_nconn_pool PROPERTIES OUTPUT_NAME "wb_nconn_pool")
add_test(nconn_pool wb_nconn_pool)
# ------------------------------------------------
# wb_t_srvr
# ------------------------------------------------
#SET(SRCS wb_t_srvr.cc)
#include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/src)
#add_executable(wb_nconn_pool ${HDRS} ${SRCS})
#add_dependencies(wb_t_srvr is2)
#target_link_libraries(wb_t_srvr compiled_catch ${CMAKE_BINARY_DIR}/src/libis2.a ${LIBRARIES} ssl crypto pthread)
#SET_TARGET_PROPERTIES(wb_t_srvr PROPERTIES OUTPUT_NAME "wb_nconn_pool")
#add_test(t_srvr wb_t_srvr)
# ------------------------------------------------
# wb_rqst
# ------------------------------------------------
SET(SRCS wb_rqst.cc)
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/src)
add_executable(wb_rqst ${HDRS} ${SRCS})
add_dependencies(wb_rqst is2)
target_link_libraries(wb_rqst compiled_catch ${CMAKE_BINARY_DIR}/src/libis2.a ${LIBRARIES} ssl crypto pthread)
SET_TARGET_PROPERTIES(wb_rqst PROPERTIES OUTPUT_NAME "wb_rqst")
add_test(rqst wb_rqst)
