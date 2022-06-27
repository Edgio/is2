# ------------------------------------------------------------------------------
# wb_nconn_tcp
# ------------------------------------------------------------------------------
SET(SRCS wb_nconn_tcp.cc)
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../
  ../../../include/
  ../../../src/core/support/
  ../../../src/core/evr/
  ../../../src/core/dns/
  ../../../src/core/nconn/
  ../../../src/core/ndebug/
)
add_executable(wb_nconn_tcp ${HDRS} ${SRCS})
target_link_libraries(wb_nconn_tcp ${DEBUG_LIBRARIES} compiled_catch ${CMAKE_BINARY_DIR}/src/libis2.a ${LIBRARIES})
SET_TARGET_PROPERTIES(wb_nconn_tcp PROPERTIES OUTPUT_NAME "wb_nconn_tcp")
add_test(nconn_tcp wb_nconn_tcp)
# ------------------------------------------------------------------------------
# wb_nconn_tls
# ------------------------------------------------------------------------------
if(BUILD_TLS)
SET(SRCS wb_nconn_tls.cc)
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../
  ../../../ext
  ../../../include  
  ../../../src/core/support/
  ../../../src/core/evr/
  ../../../src/core/dns/
  ../../../src/core/nconn/
  ../../../src/core/ndebug/
)
add_executable(wb_nconn_tls ${HDRS} ${SRCS})
target_link_libraries(wb_nconn_tls ${DEBUG_LIBRARIES} compiled_catch ${CMAKE_BINARY_DIR}/src/libis2.a ${LIBRARIES})
SET_TARGET_PROPERTIES(wb_nconn_tls PROPERTIES OUTPUT_NAME "wb_nconn_tls")
add_test(nconn_tls wb_nconn_tls)
endif()