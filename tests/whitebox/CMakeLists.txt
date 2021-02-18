# ------------------------------------------------------------------------------
# Project setup
# ------------------------------------------------------------------------------
project(is2)
cmake_minimum_required(VERSION 2.8.0)
# ------------------------------------------------------------------------------
# build catch
# ------------------------------------------------------------------------------
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ../../ext)
add_library(compiled_catch STATIC ${CMAKE_CURRENT_SOURCE_DIR}/../../ext/catch/catch.hpp _catch.cc)
# ------------------------------------------------------------------------------
# tls support
# ------------------------------------------------------------------------------
if(BUILD_TLS)
  set(LIBRARIES crypto ssl ${LIBRARIES})
endif()
# ------------------------------------------------------------------------------
# build tests
# ------------------------------------------------------------------------------
add_subdirectory(support)
add_subdirectory(url_router)
add_subdirectory(srvr)

