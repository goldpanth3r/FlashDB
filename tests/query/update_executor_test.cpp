#include <filesystem>
#include <stdexcept>

#include <gtest/gtest.h>

#include "database.h"
#include "parser/lexer.h"
#include "parser/parser.h"
#include "planner/planner.h"
#include "query/update_executor.h"

namespace flashdb {

class UpdateExecutorTest : public ::testing::Test {
protected:
    std::filesystem::path database_directory;

    void SetUp() override {
        database_directory =
            std::filesystem::temp_directory_path()
            / "flashdb_update_executor_test";

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

TEST_F(UpdateExecutorTest, UpdatesMatchingRecord) {
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
        "UPDATE student SET name = 'Charlie' WHERE id = 1;"
    );

    const auto tokens = lexer.tokenize();
    Parser parser(tokens);

    const auto statement = parser.parse();

    Planner planner;

    const auto plan =
        planner.create_plan(statement);

    ASSERT_NE(plan, nullptr);

    // Update only the record selected by the WHERE condition.
    UpdateExecutor executor(
        *plan,
        database
    );

    const std::size_t updated =
        executor.execute();

    EXPECT_EQ(updated, 1u);

    EXPECT_EQ(
        records->get(
            records->scan()[0],
            "name"
        ),
        "Charlie"
    );

    EXPECT_EQ(
        records->get(
            records->scan()[1],
            "name"
        ),
        "Bob"
    );
}

TEST_F(UpdateExecutorTest, UpdatesAllRecordsWithoutWhere) {
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
        "UPDATE student SET name = 'Updated';"
    );

    const auto tokens = lexer.tokenize();
    Parser parser(tokens);

    const auto statement = parser.parse();

    Planner planner;

    const auto plan =
        planner.create_plan(statement);

    ASSERT_NE(plan, nullptr);

    // Update every record when no WHERE condition is present.
    UpdateExecutor executor(
        *plan,
        database
    );

    const std::size_t updated =
        executor.execute();

    EXPECT_EQ(updated, 2u);

    const auto record_ids =
        records->scan();

    ASSERT_EQ(record_ids.size(), 2);

    EXPECT_EQ(
        records->get(record_ids[0], "name"),
        "Updated"
    );

    EXPECT_EQ(
        records->get(record_ids[1], "name"),
        "Updated"
    );
}

TEST_F(UpdateExecutorTest, UpdatesMultipleMatchingRecords) {
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
        {"name", "Alice"}
    });

    records->insert({
        {"id", "3"},
        {"name", "Bob"}
    });

    Lexer lexer(
        "UPDATE student SET name = 'Updated' "
        "WHERE name = 'Alice';"
    );

    const auto tokens = lexer.tokenize();
    Parser parser(tokens);

    const auto statement = parser.parse();

    Planner planner;

    const auto plan =
        planner.create_plan(statement);

    ASSERT_NE(plan, nullptr);

    // Update every record matching the WHERE condition.
    UpdateExecutor executor(
        *plan,
        database
    );

    const std::size_t updated =
        executor.execute();

    EXPECT_EQ(updated, 2u);

    const auto record_ids =
        records->scan();

    ASSERT_EQ(record_ids.size(), 3);

    EXPECT_EQ(
        records->get(record_ids[0], "name"),
        "Updated"
    );

    EXPECT_EQ(
        records->get(record_ids[1], "name"),
        "Updated"
    );

    EXPECT_EQ(
        records->get(record_ids[2], "name"),
        "Bob"
    );
}

TEST_F(UpdateExecutorTest, RejectsWrongValueType) {
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
        "UPDATE student SET id = 'wrong';"
    );

    const auto tokens = lexer.tokenize();
    Parser parser(tokens);

    const auto statement = parser.parse();

    Planner planner;

    const auto plan =
        planner.create_plan(statement);

    ASSERT_NE(plan, nullptr);

    // Reject values that do not match the target column type.
    UpdateExecutor executor(
        *plan,
        database
    );

    EXPECT_THROW(
        executor.execute(),
        std::invalid_argument
    );
}

TEST_F(UpdateExecutorTest, RejectsUnknownColumn) {
    Database database(
        database_directory.string()
    );

    create_student_table(database);

    Lexer lexer(
        "UPDATE student SET unknown = 'value';"
    );

    const auto tokens = lexer.tokenize();
    Parser parser(tokens);

    const auto statement = parser.parse();

    Planner planner;

    const auto plan =
        planner.create_plan(statement);

    ASSERT_NE(plan, nullptr);

    // Reject updates that reference a column not in the schema.
    UpdateExecutor executor(
        *plan,
        database
    );

    EXPECT_THROW(
        executor.execute(),
        std::invalid_argument
    );
}

TEST_F(UpdateExecutorTest, UpdatesIntegerColumn) {
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
        "UPDATE student SET id = 10 WHERE name = 'Alice';"
    );

    const auto tokens = lexer.tokenize();
    Parser parser(tokens);

    const auto statement = parser.parse();

    Planner planner;

    const auto plan =
        planner.create_plan(statement);

    ASSERT_NE(plan, nullptr);

    // Update an integer column using an integer SQL literal.
    UpdateExecutor executor(
        *plan,
        database
    );

    const std::size_t updated =
        executor.execute();

    EXPECT_EQ(updated, 1u);

    const auto record_ids =
        records->scan();

    ASSERT_EQ(record_ids.size(), 1);

    EXPECT_EQ(
        records->get(record_ids[0], "id"),
        "10"
    );
}

} // namespace flashdb