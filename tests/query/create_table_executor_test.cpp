#include <filesystem>
#include <stdexcept>

#include <gtest/gtest.h>

#include "database.h"
#include "parser/lexer.h"
#include "parser/parser.h"
#include "planner/planner.h"
#include "query/create_table_executor.h"

namespace flashdb {

class CreateTableExecutorTest : public ::testing::Test {
protected:
    std::filesystem::path database_directory =
        std::filesystem::temp_directory_path()
        / "flashdb_create_table_executor";

    void SetUp() override {
        std::filesystem::remove_all(
            database_directory
        );
    }

    void TearDown() override {
        std::filesystem::remove_all(
            database_directory
        );
    }
};

TEST_F(CreateTableExecutorTest, CreatesTable) {

    Lexer lexer(
        "CREATE TABLE student ("
        "id INT, "
        "name VARCHAR(50)"
        ");"
    );

    const auto tokens = lexer.tokenize();
    Parser parser(tokens);

    const auto statement = parser.parse();

    Planner planner;

    const auto plan =
        planner.create_plan(statement);

    Database database(
        database_directory.string()
    );

    CreateTableExecutor executor(
        *plan,
        database
    );

    executor.execute();

    ASSERT_TRUE(
        database.catalog().has_table("student")
    );

    EXPECT_EQ(
        database.catalog().table_count(),
        1u
    );

    const Schema& schema =
        database.catalog().get_schema("student");

    ASSERT_EQ(
        schema.field_count(),
        2u
    );

    EXPECT_EQ(
        schema.fields()[0].name,
        "id"
    );

    EXPECT_EQ(
        schema.fields()[0].type,
        FieldType::INT
    );

    EXPECT_EQ(
        schema.fields()[1].name,
        "name"
    );

    EXPECT_EQ(
        schema.fields()[1].type,
        FieldType::STRING
    );

    EXPECT_EQ(
        schema.fields()[1].length,
        50u
    );

    EXPECT_TRUE(
        std::filesystem::exists(
            database_directory / "student.tbl"
        )
    );
}

TEST_F(CreateTableExecutorTest, RejectsDuplicateTable) {

    Lexer lexer(
        "CREATE TABLE student ("
        "id INT"
        ");"
    );

    const auto tokens = lexer.tokenize();
    Parser parser(tokens);

    const auto statement = parser.parse();

    Planner planner;

    const auto plan =
        planner.create_plan(statement);

    Database database(
        database_directory.string()
    );

    CreateTableExecutor executor(
        *plan,
        database
    );

    executor.execute();

    EXPECT_THROW(
        executor.execute(),
        std::invalid_argument
    );
}

TEST_F(CreateTableExecutorTest, CatalogSurvivesRestart) {

    Lexer lexer(
        "CREATE TABLE student ("
        "id INT, "
        "name VARCHAR(50)"
        ");"
    );

    const auto tokens = lexer.tokenize();
    Parser parser(tokens);

    const auto statement = parser.parse();

    Planner planner;

    const auto plan =
        planner.create_plan(statement);

    {
        Database database(
            database_directory.string()
        );

        CreateTableExecutor executor(
            *plan,
            database
        );

        executor.execute();
    }

    // Reopen the database to verify persistent metadata.
    Database reopened_database(
        database_directory.string()
    );

    ASSERT_TRUE(
        reopened_database.catalog().has_table(
            "student"
        )
    );

    EXPECT_EQ(
        reopened_database.catalog().table_count(),
        1u
    );

    const Schema& schema =
        reopened_database.catalog().get_schema(
            "student"
        );

    ASSERT_EQ(
        schema.field_count(),
        2u
    );

    EXPECT_EQ(
        schema.fields()[0].name,
        "id"
    );

    EXPECT_EQ(
        schema.fields()[1].name,
        "name"
    );
}

TEST_F(CreateTableExecutorTest, DataSurvivesRestart) {

    Lexer lexer(
        "CREATE TABLE student ("
        "id INT, "
        "name VARCHAR(50)"
        ");"
    );

    const auto tokens = lexer.tokenize();
    Parser parser(tokens);

    const auto statement = parser.parse();

    Planner planner;

    const auto plan =
        planner.create_plan(statement);

    {
        Database database(
            database_directory.string()
        );

        CreateTableExecutor executor(
            *plan,
            database
        );

        executor.execute();

        // Open the newly created table through the database.
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
    }

    // Reopen the database after the first instance has shut down.
    Database reopened_database(
        database_directory.string()
    );

    auto records =
        reopened_database.open_table("student");

    const auto record_ids =
        records->scan();

    ASSERT_EQ(
        record_ids.size(),
        2
    );

    EXPECT_EQ(
        records->get(record_ids[0], "id"),
        "1"
    );

    EXPECT_EQ(
        records->get(record_ids[0], "name"),
        "Alice"
    );

    EXPECT_EQ(
        records->get(record_ids[1], "id"),
        "2"
    );

    EXPECT_EQ(
        records->get(record_ids[1], "name"),
        "Bob"
    );
}

} // namespace flashdb