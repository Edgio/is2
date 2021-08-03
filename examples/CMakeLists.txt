# ------------------------------------------------------------------------------
# include paths
# ------------------------------------------------------------------------------
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)
# ------------------------------------------------------------------------------
# tls support
# ------------------------------------------------------------------------------
if(BUILD_TLS)
    set(LIBRARIES is2 crypto ssl pthread)
else()
    set(LIBRARIES is2 pthread)
endif()
# ------------------------------------------------------------------------------
# basic
# ------------------------------------------------------------------------------
ADD_EXECUTABLE(basic_cmd basic.cc)
ADD_DEPENDENCIES(basic_cmd is2)
SET_TARGET_PROPERTIES(basic_cmd PROPERTIES OUTPUT_NAME "basic")
TARGET_LINK_LIBRARIES(basic_cmd ${LIBRARIES})
# ------------------------------------------------------------------------------
# files
# ------------------------------------------------------------------------------
ADD_EXECUTABLE(files_cmd files.cc)
ADD_DEPENDENCIES(files_cmd is2)
SET_TARGET_PROPERTIES(files_cmd PROPERTIES OUTPUT_NAME "files")
TARGET_LINK_LIBRARIES(files_cmd ${LIBRARIES})
# ------------------------------------------------------------------------------
# sfiles
# ------------------------------------------------------------------------------
if(BUILD_TLS)
    ADD_EXECUTABLE(sfiles_cmd sfiles.cc)
    ADD_DEPENDENCIES(sfiles_cmd is2)
    SET_TARGET_PROPERTIES(sfiles_cmd PROPERTIES OUTPUT_NAME "sfiles")
    TARGET_LINK_LIBRARIES(sfiles_cmd ${LIBRARIES})
endif()
# ------------------------------------------------------------------------------
# proxy
# ------------------------------------------------------------------------------
ADD_EXECUTABLE(proxy_cmd proxy.cc)
ADD_DEPENDENCIES(proxy_cmd is2)
SET_TARGET_PROPERTIES(proxy_cmd PROPERTIES OUTPUT_NAME "proxy")
TARGET_LINK_LIBRARIES(proxy_cmd ${LIBRARIES})
# ------------------------------------------------------------------------------
# with accesslog
# ------------------------------------------------------------------------------
ADD_EXECUTABLE(w_accesslog_cmd w_accesslog.cc)
ADD_DEPENDENCIES(w_accesslog_cmd is2)
SET_TARGET_PROPERTIES(w_accesslog_cmd PROPERTIES OUTPUT_NAME "w_accesslog")
TARGET_LINK_LIBRARIES(w_accesslog_cmd ${LIBRARIES})
# ------------------------------------------------------------------------------
# subrequest
# ------------------------------------------------------------------------------
ADD_EXECUTABLE(subrequest_cmd subrequest.cc)
ADD_DEPENDENCIES(subrequest_cmd is2)
SET_TARGET_PROPERTIES(subrequest_cmd PROPERTIES OUTPUT_NAME "subrequest")
TARGET_LINK_LIBRARIES(subrequest_cmd ${LIBRARIES})
# ------------------------------------------------------------------------------
# w_stat
# ------------------------------------------------------------------------------
ADD_EXECUTABLE(w_stat_cmd w_stat.cc)
ADD_DEPENDENCIES(w_stat_cmd is2)
SET_TARGET_PROPERTIES(w_stat_cmd PROPERTIES OUTPUT_NAME "w_stat")
TARGET_LINK_LIBRARIES(w_stat_cmd ${LIBRARIES})
