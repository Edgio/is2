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
# wb_os
# ------------------------------------------------
SET(SRCS wb_os.cc)
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/include
)
add_executable(wb_os ${HDRS} ${SRCS})
add_dependencies(wb_os is2)
target_link_libraries(wb_os
  compiled_catch
  ${CMAKE_BINARY_DIR}/src/libis2.a
  ${LIBRARIES}
)
SET_TARGET_PROPERTIES(wb_os PROPERTIES OUTPUT_NAME "wb_os")
add_test(os wb_os)
