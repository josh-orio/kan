cmake_minimum_required(VERSION 3.28.0)
set(PROJECT_NAME $ENV{PROJECT_NAME})
project(${PROJECT_NAME} LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS_RELEASE "-g")
set(CMAKE_CXX_FLAGS_DEBUG "-g")

# file(GLOB_RECURSE library_SOURCES "src/*.cpp")
# file(GLOB_RECURSE library_INCLUDE "include/*.hpp")

# add_executable(${PROJECT_NAME} ${library_SOURCES})
# target_include_directories(${PROJECT_NAME} PRIVATE include)

# gather all sources except main.cpp
file(GLOB_RECURSE library_SOURCES "src/*.cpp")
list(REMOVE_ITEM library_SOURCES "${CMAKE_SOURCE_DIR}/src/main.cpp")


include(FetchContent)

# FetchContent_Declare(
#   json
#   URL https://github.com/nlohmann/json.git
# )
FetchContent_Declare(
  json
  URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(json)

FetchContent_Declare(
  termui
  # GIT_REPOSITORY https://github.com/josh-orio/termui.git
  GIT_REPOSITORY /Users/joshorio/work/termui
  GIT_TAG main
)
FetchContent_MakeAvailable(termui)



# # define the test executable
# add_executable(test_all test.cpp)
# target_link_libraries(test_all PRIVATE kan_lib GTest::gtest_main)

# library target — this is what tests link against
add_library(kan_lib STATIC ${library_SOURCES})
target_include_directories(kan_lib PUBLIC include)
target_link_libraries(kan_lib PUBLIC nlohmann_json::nlohmann_json termui)




# the actual app executable — keep using PROJECT_NAME for this
add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE kan_lib )#nlohmann_json::nlohmann_json)



option(BUILD_TESTS "Build unit tests" OFF) # cmake -DBUILD_TESTS=ON/OFF ..
if(BUILD_TESTS)
enable_testing()
add_subdirectory(tests)
endif()
