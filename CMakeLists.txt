# ------------------------------------------------------------------------------
# Project setup
# ------------------------------------------------------------------------------
project(is2)
cmake_minimum_required(VERSION 2.8.0)
if (POLICY CMP0054)
    cmake_policy(SET CMP0054 NEW)
endif()
# ------------------------------------------------------------------------------
# Build options
# ------------------------------------------------------------------------------
option(GCC_OPTIONS          "Command line options passed to gcc or 'native' to compile for this hardware" OFF)
option(FORTIFY              "Fortify Source GCC options" OFF)
option(BUILD_H2             "Build with h2 support (w/ nghttp2)" OFF)
option(BUILD_TLS            "Build with tls support (w/ openssl)" OFF)
option(BUILD_CUSTOM_OPENSSL "Build for custom OpenSSL" OFF)
option(DEBUG_MODE           "Compile in debug mode." OFF)
option(BUILD_TESTS          "Build the unit tests." ON)
option(BUILD_EXAMPLES       "Build the examples." ON)
option(BUILD_SYMBOLS        "Build with Symbols" ON)
option(BUILD_UDNS           "Build with udns (async dns support)" ON)
option(BUILD_TCMALLOC       "Build with tcmalloc" OFF)
option(BUILD_PROFILER       "Enable google cpu and heap profiler support" OFF)
option(BUILD_ASAN           "Build with Address Sanitizer" OFF)
option(BUILD_UBSAN          "Build with Undefined Behavior Sanitizer" OFF)
# ------------------------------------------------------------------------------
# Compiler options
# ------------------------------------------------------------------------------
add_definitions(-Wall -Wformat -Wformat-security)
# ------------------------------------------------------------------------------
# Display the current settings
# ------------------------------------------------------------------------------
message(STATUS "Build Configuration:")
message("")
message("    Build Option                               Variable                Value                 ")
message("    -----------------------------------------------------------------------------------------")
message("    Build with h2 support (w/ nghttp2):        " "BUILD_H2                " ${BUILD_H2})
message("    Build with tls support (w/ openssl):       " "BUILD_TLS               " ${BUILD_TLS})
message("    Build for custom OpenSSL:                  " "BUILD_CUSTOM_OPENSSL    " ${BUILD_CUSTOM_OPENSSL})
message("    Debug mode:                                " "DEBUG_MODE              " ${DEBUG_MODE})
message("    Build unit tests:                          " "BUILD_TESTS             " ${BUILD_TESTS})
message("    Fortify Source:                            " "FORTIFY                 " ${FORTIFY})
message("    Build unit tests:                          " "BUILD_TESTS             " ${BUILD_TESTS})
message("    Build examples:                            " "BUILD_EXAMPLES          " ${BUILD_EXAMPLES})
message("    Install path:                              " "INSTALL_PREFIX          " ${CMAKE_INSTALL_PREFIX})
message("    Build Symbols                              " "BUILD_SYMBOLS           " ${BUILD_SYMBOLS})
message("    Build with udns support:                   " "BUILD_UDNS              " ${BUILD_UDNS})
message("    Build with tcmalloc:                       " "BUILD_TCMALLOC          " ${BUILD_TCMALLOC})
message("    Enable google cpu/heap profiler support:   " "BUILD_PROFILER          " ${BUILD_PROFILER})
message("    Build with Address Sanitizer:              " "BUILD_ASAN              " ${BUILD_ASAN})
message("    Build with Undefined Behavior Sanitizer:   " "BUILD_UBSAN             " ${BUILD_UBSAN})
message("")
# ------------------------------------------------------------------------------
# build h2
# ------------------------------------------------------------------------------
if(BUILD_H2)
  add_definitions(-DBUILD_H2_WITH_NGHTTP2=1)
endif()
# ------------------------------------------------------------------------------
# build tls
# ------------------------------------------------------------------------------
if(BUILD_TLS)
  add_definitions(-DBUILD_TLS_WITH_OPENSSL=1)
endif()
# ------------------------------------------------------------------------------
# OS X specific code
# ------------------------------------------------------------------------------
IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
   SET(OperatingSystem "Mac OS X")
   INCLUDE_DIRECTORIES(/usr/local/opt/rapidjson/include)
   # Add MacPorts
   if(BUILD_TLS)
     INCLUDE_DIRECTORIES(/usr/local/opt/openssl/include)
     LINK_DIRECTORIES(/usr/local/opt/openssl/lib)
   endif()
   if(BUILD_TCMALLOC OR BUILD_PROFILER)
     INCLUDE_DIRECTORIES(/usr/local/opt/gperftools/include)
     LINK_DIRECTORIES(/usr/local/opt/gperftools/lib)
   endif()
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
# ------------------------------------------------------------------------------
# Fortify Options
# ------------------------------------------------------------------------------
if (FORTIFY)
  add_definitions(-D_FORTIFY_SOURCE=2 -O1 -fstack-protector-all -Wl,-z,relro,-z,now)
endif()
# ------------------------------------------------------------------------------
# fail if not found
# ------------------------------------------------------------------------------
macro(fail_if_not_found_library a_lib)
  find_library(${a_lib}_LIBRARY
               NAME ${a_lib}
               PATH_SUFFIXES ${CMAKE_LIBRARY_ARCHITECTURE})
  if(NOT ${a_lib}_LIBRARY)
    message(FATAL_ERROR "${a_lib} library not found")
  endif()
endmacro(fail_if_not_found_library)
# ------------------------------------------------------------------------------
# ASAN
# ------------------------------------------------------------------------------
if(BUILD_ASAN)
  set(DEBUG_LIBRARIES asan ${DEBUG_LIBRARIES})
  add_definitions(-g3 -fno-omit-frame-pointer -fsanitize=address)
  set(DEBUG_MODE ON)
  set(BUILD_PROFILER OFF)
  set(BUILD_TCMALLOC OFF)
# ------------------------------------------------------------------------------
# UBSAN
# ------------------------------------------------------------------------------
elseif(BUILD_UBSAN)
  set(DEBUG_LIBRARIES asan ${DEBUG_LIBRARIES})
  add_definitions(-g3 -fno-omit-frame-pointer -fsanitize=undefined -fno-sanitize-recover)
  set(DEBUG_MODE ON)
  set(BUILD_PROFILER OFF)
  set(BUILD_TCMALLOC OFF)
endif()
# ------------------------------------------------------------------------------
# Optional flags
# ------------------------------------------------------------------------------
if (DEBUG_MODE)
  add_definitions(-O0 -g3)
else()
  add_definitions(-O2)
endif()
if (BUILD_SYMBOLS)
  add_definitions(-g3)
endif()
# ------------------------------------------------------------------------------
# Build UDNS
# ------------------------------------------------------------------------------
if(BUILD_UDNS)
  include(ExternalProject)
  ExternalProject_Add(ext_udns
    # Optional approach -including from external url
    #URL http://www.corpit.ru/mjt/udns/udns-0.4.tar.gz
    DOWNLOAD_COMMAND ""
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/ext/udns-0.4
    BINARY_DIR ${CMAKE_SOURCE_DIR}/ext/udns-0.4
    CONFIGURE_COMMAND "./configure"
    #BUILD_COMMAND $(MAKE) COMMAND $(MAKE)
    INSTALL_COMMAND ""
  )
  add_definitions(-DASYNC_DNS_WITH_UDNS=1)
endif()
# ------------------------------------------------------------------------------
# Build PROFILER
# ------------------------------------------------------------------------------
if (BUILD_PROFILER)
  if (BUILD_LINUX)
    fail_if_not_found_library(libprofiler.a)
    set(LIBRARIES ${LIBRARIES} ${libprofiler.a_LIBRARY})
  else()
    fail_if_not_found_library(profiler)
    set(LIBRARIES ${LIBRARIES} profiler)
  endif()
  add_definitions(-DENABLE_PROFILER=1)
  # profiler requires tcmalloc
  set(BUILD_TCMALLOC ON)
endif()
# ------------------------------------------------------------------------------
# Build TCMALLOC
# ------------------------------------------------------------------------------
if (BUILD_TCMALLOC)
  if (BUILD_LINUX)
    fail_if_not_found_library(libtcmalloc.a)
    fail_if_not_found_library(libunwind.a)
    fail_if_not_found_library(liblzma.a)
    set(LIBRARIES ${LIBRARIES} ${libtcmalloc.a_LIBRARY})
    set(LIBRARIES ${LIBRARIES} ${libunwind.a_LIBRARY})
    set(LIBRARIES ${LIBRARIES} ${liblzma.a_LIBRARY})
    set(LIBRARIES ${LIBRARIES} ${liblzma.a_LIBRARY})
  elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    fail_if_not_found_library(tcmalloc)
    set(LIBRARIES ${LIBRARIES} tcmalloc)
    set(LIBRARIES ${LIBRARIES} lzma)
  else()
    fail_if_not_found_library(tcmalloc)
    fail_if_not_found_library(unwind)
    set(LIBRARIES ${LIBRARIES} tcmalloc)
    set(LIBRARIES ${LIBRARIES} unwind)
    set(LIBRARIES ${LIBRARIES} lzma)
  endif()
  set(LIBRARIES ${LIBRARIES} pthread)
endif()
# ------------------------------------------------------------------------------
# special build case for OPENSSL
# ------------------------------------------------------------------------------
if(BUILD_TLS AND BUILD_CUSTOM_OPENSSL)
  INCLUDE_DIRECTORIES("${BUILD_CUSTOM_OPENSSL}/include")
  LINK_DIRECTORIES("${BUILD_CUSTOM_OPENSSL}")
  LINK_DIRECTORIES("${BUILD_CUSTOM_OPENSSL}/lib")
endif()
# ------------------------------------------------------------------------------
#
# ------------------------------------------------------------------------------
# make the cmake list variables into .deb-compatible strings
string(REPLACE ";" ", " CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS_LIST}")
string(REPLACE ";" ", " CPACK_DEBIAN_PACKAGE_BUILDS_DEPENDS "${CPACK_DEBIAN_PACKAGE_BUILDS_DEPENDS_LIST}")
# ------------------------------------------------------------------------------
# Version
# ------------------------------------------------------------------------------
EXECUTE_PROCESS(COMMAND git describe --tags OUTPUT_VARIABLE VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
add_definitions(-DIS2_VERSION="${VERSION}")
# ------------------------------------------------------------------------------
# Debian Package Support
# ------------------------------------------------------------------------------
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  SET(DISTRIBUTION "macOS")
else()
  EXECUTE_PROCESS(COMMAND lsb_release -cs OUTPUT_VARIABLE DISTRIBUTION OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()
SET(CPACK_GENERATOR "DEB")
SET(CPACK_DEBIAN_PACKAGE_VERSION "${VERSION}-${DISTRIBUTION}")
SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
SET(CPACK_PACKAGE_FILE_NAME "is2_${CPACK_DEBIAN_PACKAGE_VERSION}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")
SET(CPACK_DEBIAN_PACKAGE_NAME "is2")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Reed Morrison")
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "http(s) server library")
SET(CPACK_PACKAGE_DESCRIPTION "http(s) server library")
message(STATUS "Package Configuration:")
message("")
message("    Option                                     Value                     ")
message("    ---------------------------------------------------------------------")
message("    Package Version:                           ${CPACK_DEBIAN_PACKAGE_VERSION}")
message("")
INCLUDE(CPack)
# ------------------------------------------------------------------------------
# include source and test directories
# ------------------------------------------------------------------------------
add_subdirectory(src)
if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
if(BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()
# ------------------------------------------------------------------------------
# docs
# ------------------------------------------------------------------------------
add_custom_target(docs
    COMMAND doxygen ${CMAKE_CURRENT_SOURCE_DIR}/doxygen.conf WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
)
