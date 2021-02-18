# ------------------------------------------------------------------------------
# tls support
# ------------------------------------------------------------------------------
if(BUILD_TLS)
  set(LIBRARIES compiled_catch ${CMAKE_BINARY_DIR}/src/libis2.a crypto ssl pthread)
else()
  set(LIBRARIES compiled_catch ${CMAKE_BINARY_DIR}/src/libis2.a pthread)
endif()
# ------------------------------------------------------------------------------
# wb_os
# ------------------------------------------------------------------------------
SET(SRCS wb_os.cc)
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/include
)
add_executable(wb_os ${HDRS} ${SRCS})
add_dependencies(wb_os is2)
target_link_libraries(wb_os ${LIBRARIES})
SET_TARGET_PROPERTIES(wb_os PROPERTIES OUTPUT_NAME "wb_os")
add_test(os wb_os)
# ------------------------------------------------------------------------------
# wb_os
# ------------------------------------------------------------------------------
SET(SRCS wb_nbq.cc)
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/include
)
add_executable(wb_nbq ${HDRS} ${SRCS})
add_dependencies(wb_nbq is2)
target_link_libraries(wb_nbq ${LIBRARIES})
SET_TARGET_PROPERTIES(wb_nbq PROPERTIES OUTPUT_NAME "wb_nbq")
add_test(nbq wb_nbq)

