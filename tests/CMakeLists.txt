include(FetchContent)

# download and configure GoogleTest
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# one file per area of kan - core/, cli and so on
file(GLOB test_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*_test.cpp")

add_executable(kan_tests ${test_SOURCES})
target_include_directories(kan_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(kan_tests PRIVATE kan_lib GTest::gtest_main)

# discover tests
include(GoogleTest)
gtest_discover_tests(kan_tests)
