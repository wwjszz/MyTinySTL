cmake_minimum_required(VERSION 4.0)
project(wyne_test)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

include_directories(${PROJECT_SOURCE_DIR}/wyne)
include_directories(/opt/homebrew/opt/googletest/include)

find_package(GTest REQUIRED)

enable_testing()

# file(GLOB TEST_SOURCES "test/*.cpp")
add_executable(algo_test test/algo_test.cpp)
add_executable(vector_test test/vector_test.cpp)
add_executable(optional_test test/optional_test.cpp)

target_link_libraries(algo_test  GTest::GTest GTest::Main)
target_link_libraries(vector_test GTest::GTest GTest::Main)
target_link_libraries(optional_test GTest::GTest GTest::Main)