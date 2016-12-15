find_package(GMock REQUIRED)

include_directories(
  ${CMAKE_SOURCE_DIR}/src
  ${SERVICE_DEPS_INCLUDE_DIRS}
)
link_directories(
  ${SERVICE_DEPS_LIBRARY_DIRS}
)

add_executable(test-libindicator-printers
  test_exporter.cpp
  test_utils.cpp
)
target_link_libraries(test-libindicator-printers
  ${SERVICE_LIB_NAME}
  ${SERVICE_DEPS_LIBRARIES}

  ${GTEST_LIBRARIES}
  ${GMOCK_LIBRARIES}

  ${CMAKE_THREAD_LIBS_INIT}
)
add_test(test-libindicator-printers test-libindicator-printers)
