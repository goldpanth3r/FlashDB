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
    src/log/log_manager.cpp
    src/buffer/buffer.cpp
    src/buffer/buffer_manager.cpp
    src/tx/transaction.cpp
    src/record/schema.cpp
    src/record/record_page.cpp
    src/record/layout.cpp
    src/record/rid.cpp
    src/record/record_file.cpp
    src/metadata/table_catalog.cpp

)

target_include_directories(flashdb PRIVATE src)

enable_testing()

add_executable(flashdb_tests
    tests/file/page_test.cpp
    tests/file/file_manager_test.cpp
    tests/log/log_manager_test.cpp
    tests/buffer/buffer_manager_test.cpp
    tests/tx/transaction_test.cpp
    tests/record/record_page_test.cpp
    tests/record/schema_test.cpp
    tests/record/layout_test.cpp
    tests/record/record_file_test.cpp
    tests/metadata/table_catalog_test.cpp

    src/file/block_id.cpp
    src/file/page.cpp
    src/file/file_manager.cpp
    src/log/log_manager.cpp
    src/buffer/buffer.cpp
    src/buffer/buffer_manager.cpp
    src/tx/transaction.cpp
    src/record/schema.cpp
    src/record/record_page.cpp
    src/record/layout.cpp
    src/record/rid.cpp
    src/record/record_file.cpp
    src/metadata/table_catalog.cpp
)

target_include_directories(flashdb_tests PRIVATE src)

target_link_libraries(flashdb_tests GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(flashdb_tests)