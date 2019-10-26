cmake_minimum_required(VERSION 3.10)
project(jnotify)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

find_package(Rapidjson REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(LIBNOTIFY REQUIRED IMPORTED_TARGET libnotify)
pkg_check_modules(LIBCURL REQUIRED IMPORTED_TARGET libcurl)
pkg_check_modules(LIBCONFIG REQUIRED IMPORTED_TARGET libconfig++)

set(APP_SRCS
  src/jnotify/JNotify.cpp
  src/jnotify/JNotify.h
  src/jnotify/UrlListener.cpp
  src/jnotify/UrlListener.h
  src/jnotify/JnConfig.cpp
  src/jnotify/JnConfig.h
)

add_library(libjnotify STATIC
  ${APP_SRCS}
)

target_link_libraries(libjnotify
  PkgConfig::LIBNOTIFY
  PkgConfig::LIBCURL
  PkgConfig::LIBCONFIG
)

set_target_properties(libjnotify PROPERTIES
  OUTPUT_NAME "jnotify$<$<CONFIG:Debug>:d>"
)

add_executable(jnotify
  src/main.cpp
)

target_include_directories(jnotify PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/jnotify
  ${RAPIDJSON_INCLUDE_DIRS}
)

target_link_libraries(jnotify
  libjnotify
)

if(BUILD_TESTS)
  set(TEST_RES ${CMAKE_CURRENT_SOURCE_DIR}/test/res/)
  add_subdirectory(test)
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/configuration.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/configuration.h
  @ONLY
)
