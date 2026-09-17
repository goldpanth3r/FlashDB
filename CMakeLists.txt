cmake_minimum_required(VERSION 3.20)

project(FlashDB VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
)

FetchContent_MakeAvailable(googletest)

add_executable(flashdb
    src/main.cpp
    src/file/block_id.cpp
    src/file/page.cpp
    src/file/file_manager.cpp
)

target_include_directories(flashdb PRIVATE src)

enable_testing()

add_executable(flashdb_tests
    tests/file/page_test.cpp
    tests/file/file_manager_test.cpp

    src/file/block_id.cpp
    src/file/page.cpp
    src/file/file_manager.cpp
)

target_include_directories(flashdb_tests PRIVATE src)

target_link_libraries(flashdb_tests GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(flashdb_tests)