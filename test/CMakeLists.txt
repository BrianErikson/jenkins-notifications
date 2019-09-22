find_package(GTest REQUIRED)


set(TEST_SRCS
  tests.cpp
)

add_executable(tests
  ${TEST_SRCS}
)

target_include_directories(tests PUBLIC
  ${CMAKE_SOURCE_DIR}/src
  ${GTEST_INCLUDE_DIRS}
  ${CMAKE_BINARY_DIR}
)

target_link_libraries(tests
  libjnotify
  ${GTEST_BOTH_LIBRARIES}
)

gtest_discover_tests(tests)
