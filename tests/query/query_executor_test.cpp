#include "query/query_executor.h"

#include <filesystem>

#include <gtest/gtest.h>

#include "database.h"
#include "parser/lexer.h"
#include "parser/parser.h"
#include "planner/planner.h"

namespace flashdb {

class QueryExecutorTest : public ::testing::Test {
protected:
    std::filesystem::path database_directory;

    void SetUp() override {
        database_directory =
            std::filesystem::temp_directory_path()
            / "flashdb_query_executor_test";

        std::filesystem::remove_all(database_directory);
    }

    void TearDown() override {
        std::filesystem::remove_all(database_directory);
    }

    void create_student_table(Database& database) {
        Schema schema;
        schema.add_int_field("id");
        schema.add_string_field("name", 50);

        database.catalog().create_table(
            "student",
            schema
        );

        database.file_manager().append(
            "student.tbl"
        );
    }
};

TEST_F(QueryExecutorTest, ExecutesSelect) {
    Database database(
        database_directory.string()
    );

    create_student_table(database);

    auto records =
        database.open_table("student");

    records->insert({
        {"id", "1"},
        {"name", "Alice"}
    });

    records->insert({
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

    // Execute SELECT through the shared database.
    QueryExecutor executor(
        *plan,
        database
    );

    const auto results =
        executor.execute();

    ASSERT_EQ(results.size(), 2);

    EXPECT_EQ(results[0][0], "Alice");
    EXPECT_EQ(results[1][0], "Bob");
}

TEST_F(QueryExecutorTest, ExecutesSelectWithWhere) {
    Database database(
        database_directory.string()
    );

    create_student_table(database);

    auto records =
        database.open_table("student");

    records->insert({
        {"id", "1"},
        {"name", "Alice"}
    });

    records->insert({
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

    // Execute the filtered query through the database.
    QueryExecutor executor(
        *plan,
        database
    );

    const auto results =
        executor.execute();

    ASSERT_EQ(results.size(), 1);

    EXPECT_EQ(results[0][0], "Bob");
}

TEST_F(QueryExecutorTest, ExecutesMultipleColumns) {
    Database database(
        database_directory.string()
    );

    create_student_table(database);

    auto records =
        database.open_table("student");

    records->insert({
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

    // Execute the projection through the database.
    QueryExecutor executor(
        *plan,
        database
    );

    const auto results =
        executor.execute();

    ASSERT_EQ(results.size(), 1);
    ASSERT_EQ(results[0].size(), 2);

    EXPECT_EQ(results[0][0], "1");
    EXPECT_EQ(results[0][1], "Alice");
}

} // namespace flashdb