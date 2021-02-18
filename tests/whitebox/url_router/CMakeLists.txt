# ------------------------------------------------------------------------------
# tls support
# ------------------------------------------------------------------------------
if(BUILD_TLS)
  set(LIBRARIES compiled_catch ${CMAKE_BINARY_DIR}/src/libis2.a crypto ssl pthread)
else()
  set(LIBRARIES compiled_catch ${CMAKE_BINARY_DIR}/src/libis2.a pthread)
endif()
# ------------------------------------------------------------------------------
# wb_url_router
# ------------------------------------------------------------------------------
SET(SRCS wb_url_router.cc)
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/src
)
add_executable(wb_url_router ${HDRS} ${SRCS})
add_dependencies(wb_url_router is2)
target_link_libraries(wb_url_router ${LIBRARIES})
SET_TARGET_PROPERTIES(wb_url_router PROPERTIES OUTPUT_NAME "wb_url_router")
add_test(url_router wb_url_router)
