# ------------------------------------------------------------------------------
# macros
# ------------------------------------------------------------------------------
macro (add_is2_wb_test test_name)
    include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src)
    add_executable("wb_${test_name}" "wb_${test_name}.cc")
    if(BUILD_TLS)
        target_link_libraries("wb_${test_name}"
                              ${DEBUG_LIBRARIES}
                              is2_compiled_catch
                              is2
                              ${LIBRARIES})
	else()
        target_link_libraries("wb_${test_name}"
                              ${DEBUG_LIBRARIES}
                              is2_compiled_catch
                              is2
                              ${LIBRARIES})	
	endif()
	target_compile_definitions("wb_${test_name}" PUBLIC TEST_RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data/")
    add_test(${test_name} "wb_${test_name}")
endmacro()
# ------------------------------------------------------------------------------
# build catch
# ------------------------------------------------------------------------------
include_directories(${CMAKE_SOURCE_DIR}/ext)
add_library(is2_compiled_catch STATIC ${CMAKE_SOURCE_DIR}/ext/catch/catch.hpp _catch.cc)
# ------------------------------------------------------------------------------
# tls support
# ------------------------------------------------------------------------------
if(BUILD_TLS)
  set(LIBRARIES ${DEBUG_LIBRARIES} ${CMAKE_BINARY_DIR}/src/libis2.a crypto ssl ${LIBRARIES})
else()
  set(LIBRARIES ${DEBUG_LIBRARIES} ${CMAKE_BINARY_DIR}/src/libis2.a ${LIBRARIES})
endif()
# ------------------------------------------------------------------------------
# build tests
# ------------------------------------------------------------------------------
#add_subdirectory(dns)
#add_subdirectory(nconn)
add_subdirectory(srvr)
add_subdirectory(support)
add_subdirectory(url_router)
