# ------------------------------------------------------------------------------
# wb_nconn_pool
# ------------------------------------------------------------------------------
SET(SRCS wb_nconn_pool.cc)
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/src)
add_executable(wb_nconn_pool ${HDRS} ${SRCS})
add_dependencies(wb_nconn_pool is2)
target_link_libraries(wb_nconn_pool compiled_catch ${CMAKE_BINARY_DIR}/src/libis2.a ${LIBRARIES} pthread)
SET_TARGET_PROPERTIES(wb_nconn_pool PROPERTIES OUTPUT_NAME "wb_nconn_pool")
add_test(nconn_pool wb_nconn_pool)
# ------------------------------------------------------------------------------
# wb_t_srvr
# ------------------------------------------------------------------------------
#SET(SRCS wb_t_srvr.cc)
#include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/src)
#add_executable(wb_nconn_pool ${HDRS} ${SRCS})
#add_dependencies(wb_t_srvr is2)
#target_link_libraries(wb_t_srvr compiled_catch ${CMAKE_BINARY_DIR}/src/libis2.a ${LIBRARIES} pthread)
#SET_TARGET_PROPERTIES(wb_t_srvr PROPERTIES OUTPUT_NAME "wb_nconn_pool")
#add_test(t_srvr wb_t_srvr)
# ------------------------------------------------------------------------------
# wb_rqst
# ------------------------------------------------------------------------------
SET(SRCS wb_rqst.cc)
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/src)
add_executable(wb_rqst ${HDRS} ${SRCS})
add_dependencies(wb_rqst is2)
target_link_libraries(wb_rqst compiled_catch ${CMAKE_BINARY_DIR}/src/libis2.a ${LIBRARIES} pthread)
SET_TARGET_PROPERTIES(wb_rqst PROPERTIES OUTPUT_NAME "wb_rqst")
add_test(rqst wb_rqst)
