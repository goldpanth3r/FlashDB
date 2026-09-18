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
    src/parser/token.cpp
    src/parser/lexer.cpp
    src/parser/ast/expression.cpp
    src/parser/ast/select_statement.cpp
    src/parser/ast/condition.cpp
    src/parser/parser.cpp
    src/parser/ast/insert_statement.cpp
    src/parser/ast/create_table_statement.cpp
    src/parser/ast/update_statement.cpp
    src/parser/ast/delete_statement.cpp
    src/planner/plan.cpp
    src/planner/planner.cpp
    src/planner/filter_plan.cpp
    src/query/table_scan_executor.cpp
    src/query/filter_executor.cpp
    src/query/project_executor.cpp
    src/query/query_executor.cpp
    src/query/insert_executor.cpp
    src/query/update_executor.cpp
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
    tests/parser/token_test.cpp
    tests/parser/lexer_test.cpp
    tests/parser/expression_test.cpp
    tests/parser/select_statement_test.cpp
    tests/parser/condition_test.cpp
    tests/parser/parser_test.cpp
    tests/parser/insert_statement_test.cpp
    tests/parser/create_table_statement_test.cpp
    tests/parser/update_statement_test.cpp
    tests/parser/delete_statement_test.cpp
    tests/planner/plan_test.cpp
    tests/planner/planner_test.cpp
    tests/planner/filter_plan_test.cpp
    tests/query/table_scan_executor_test.cpp
    tests/query/filter_executor_test.cpp
    tests/query/project_executor_test.cpp
    tests/query/query_executor_test.cpp
    tests/query/insert_executor_test.cpp
    tests/query/update_executor_test.cpp
    
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
    src/parser/token.cpp
    src/parser/lexer.cpp
    src/parser/ast/expression.cpp
    src/parser/ast/select_statement.cpp
    src/parser/ast/condition.cpp
    src/parser/parser.cpp
    src/parser/ast/insert_statement.cpp
    src/parser/ast/create_table_statement.cpp
    src/parser/ast/update_statement.cpp
    src/parser/ast/delete_statement.cpp
    src/planner/plan.cpp
    src/planner/planner.cpp
    src/planner/filter_plan.cpp
    src/query/table_scan_executor.cpp
    src/query/filter_executor.cpp
    src/query/project_executor.cpp
    src/query/query_executor.cpp
    src/query/insert_executor.cpp
    src/query/update_executor.cpp
)

target_include_directories(flashdb_tests PRIVATE src)

target_link_libraries(flashdb_tests GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(flashdb_tests)