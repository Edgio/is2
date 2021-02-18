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
target_link_libraries(wb_os
  compiled_catch
  ${CMAKE_BINARY_DIR}/src/libis2.a
  ${LIBRARIES}
)
SET_TARGET_PROPERTIES(wb_os PROPERTIES OUTPUT_NAME "wb_os")
add_test(os wb_os)
