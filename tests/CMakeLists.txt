find_package(GMock REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/src)

add_executable(test-utils test_utils.cpp)
target_link_libraries(test-utils
  ${SERVICE_LIB_NAME}
  ${SERVICE_DEPS_LIBRARIES}

  ${GTEST_LIBRARIES}
  ${GMOCK_LIBRARIES}

  ${CMAKE_THREAD_LIBS_INIT}
)
add_test(test-utils test-utils)
