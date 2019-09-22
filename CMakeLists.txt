cmake_minimum_required(VERSION 3.10)
project(jnotify)

set(CMAKE_CXX_STANDARD 14)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBNOTIFY REQUIRED IMPORTED_TARGET libnotify)

set(APP_SRCS
  src/jnotify/JNotify.cpp
  src/jnotify/JNotify.h
)

add_library(libjnotify STATIC
  ${APP_SRCS}
)

target_link_libraries(libjnotify
  PkgConfig::LIBNOTIFY
)

set_target_properties(libjnotify PROPERTIES
  OUTPUT_NAME "jnotify$<$<CONFIG:Debug>:d>"
)

add_executable(jnotify
  src/main.cpp
)

target_include_directories(jnotify PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/jnotify
)

target_link_libraries(jnotify
  libjnotify
)

if(BUILD_TESTS)
  add_subdirectory(test)
endif()
