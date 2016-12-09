project(indicator-transfer C CXX)
cmake_minimum_required(VERSION 2.8.9)

### Always Be Testing
enable_testing()

### Set variables for our binary build targets
set(SERVICE_LIB_NAME "indicator-printers")
set(SERVICE_EXEC_NAME "indicator-printers-ng")

### Some default compile flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -g -Wall -Wextra -Werror -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O2 -g -Wextra -Wall -Werror -Werror=conversion-null -Wno-ignored-qualifiers -fPIC")

### Find deps
find_package(PkgConfig REQUIRED)
find_package(Intltool REQUIRED)
find_package(Threads)

pkg_check_modules(SERVICE_DEPS REQUIRED
  glib-2.0>=2.36
  gio-unix-2.0>=2.36
)

### Find libcups
find_program(CUPS_CONFIG cups-config)
execute_process(COMMAND ${CUPS_CONFIG} --cflags OUTPUT_VARIABLE CUPS_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND ${CUPS_CONFIG} --libs OUTPUT_VARIABLE CUPS_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE)

### Add sub-directories
add_subdirectory(src)
add_subdirectory(tests)

### Enable coverage reporting
include(EnableCoverageReport)
ENABLE_COVERAGE_REPORT(
  TARGETS ${SERVICE_LIB_NAME} ${SERVICE_EXEC_NAME}
  TESTS test-utils
  FILTER /usr/include ${CMAKE_BINARY_DIR}/*
)

