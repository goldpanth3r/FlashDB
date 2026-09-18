#include "query/query_executor.h"

#include <gtest/gtest.h>

#include "buffer/buffer_manager.h"
#include "file/file_manager.h"
#include "parser/lexer.h"
#include "parser/parser.h"
#include "planner/planner.h"
#include "record/layout.h"
#include "record/record_file.h"

namespace flashdb {

TEST(QueryExecutorTest, ExecutesSelect) {
    FileManager file_manager("query_executor_test_data");
    BufferManager buffer_manager(file_manager, 10);

    Schema schema;
    schema.add_int_field("id");
    schema.add_string_field("name", 50);

    Layout layout(schema);

    RecordFile records(
        file_manager,
        buffer_manager,
        "student",
        layout
    );

    records.insert({
        {"id", "1"},
        {"name", "Alice"}
    });

    records.insert({
        {"id", "2"},
        {"name", "Bob"}
    });

    Lexer lexer(
        "SELECT name FROM student;"
    );

    const auto tokens = lexer.tokenize();

    Parser parser(tokens);

    const auto statement = parser.parse();

    Planner planner;

    const auto plan =
        planner.create_plan(statement);

    ASSERT_NE(plan, nullptr);

    QueryExecutor executor(
        *plan,
        records
    );

    const auto results = executor.execute();

    ASSERT_EQ(results.size(), 2);

    EXPECT_EQ(results[0][0], "Alice");
    EXPECT_EQ(results[1][0], "Bob");
}

TEST(QueryExecutorTest, ExecutesSelectWithWhere) {
    FileManager file_manager("query_executor_where_test_data");
    BufferManager buffer_manager(file_manager, 10);

    Schema schema;
    schema.add_int_field("id");
    schema.add_string_field("name", 50);

    Layout layout(schema);

    RecordFile records(
        file_manager,
        buffer_manager,
        "student",
        layout
    );

    records.insert({
        {"id", "1"},
        {"name", "Alice"}
    });

    records.insert({
        {"id", "2"},
        {"name", "Bob"}
    });

    Lexer lexer(
        "SELECT name FROM student WHERE id = 2;"
    );

    const auto tokens = lexer.tokenize();

    Parser parser(tokens);

    const auto statement = parser.parse();

    Planner planner;

    const auto plan =
        planner.create_plan(statement);

    ASSERT_NE(plan, nullptr);

    QueryExecutor executor(
        *plan,
        records
    );

    const auto results = executor.execute();

    ASSERT_EQ(results.size(), 1);

    EXPECT_EQ(results[0][0], "Bob");
}

TEST(QueryExecutorTest, ExecutesMultipleColumns) {
    FileManager file_manager("query_executor_columns_test_data");
    BufferManager buffer_manager(file_manager, 10);

    Schema schema;
    schema.add_int_field("id");
    schema.add_string_field("name", 50);

    Layout layout(schema);

    RecordFile records(
        file_manager,
        buffer_manager,
        "student",
        layout
    );

    records.insert({
        {"id", "1"},
        {"name", "Alice"}
    });

    Lexer lexer(
        "SELECT id, name FROM student;"
    );

    const auto tokens = lexer.tokenize();

    Parser parser(tokens);

    const auto statement = parser.parse();

    Planner planner;

    const auto plan =
        planner.create_plan(statement);

    ASSERT_NE(plan, nullptr);

    QueryExecutor executor(
        *plan,
        records
    );

    const auto results = executor.execute();

    ASSERT_EQ(results.size(), 1);
    ASSERT_EQ(results[0].size(), 2);

    EXPECT_EQ(results[0][0], "1");
    EXPECT_EQ(results[0][1], "Alice");
}

}