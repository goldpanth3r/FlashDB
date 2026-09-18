#include <filesystem>

#include <gtest/gtest.h>

#include "buffer/buffer_manager.h"
#include "file/file_manager.h"
#include "parser/lexer.h"
#include "parser/parser.h"
#include "planner/planner.h"
#include "query/update_executor.h"
#include "record/layout.h"
#include "record/record_file.h"
#include "record/schema.h"

namespace flashdb {

class UpdateExecutorTest : public ::testing::Test {
protected:
    std::filesystem::path test_directory;

    void SetUp() override {
        test_directory =
            std::filesystem::temp_directory_path()
            / "flashdb_update_executor_test";

        std::filesystem::remove_all(test_directory);
        std::filesystem::create_directories(test_directory);
    }

    void TearDown() override {
        std::filesystem::remove_all(test_directory);
    }
};

TEST_F(UpdateExecutorTest, UpdatesMatchingRecord) {
    FileManager file_manager(
        test_directory.string()
    );

    BufferManager buffer_manager(
        file_manager,
        10
    );

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

    const RecordId alice =
        records.insert({
            {"id", "1"},
            {"name", "Alice"}
        });

    const RecordId bob =
        records.insert({
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
    EXPECT_EQ(plan->get_name(), "Update");

    // Execute the targeted UPDATE through the query layer.
    UpdateExecutor executor(
        *plan,
        records
    );

    EXPECT_EQ(
        executor.execute(),
        1
    );

    EXPECT_EQ(
        records.get(alice, "name"),
        "Charlie"
    );

    EXPECT_EQ(
        records.get(bob, "name"),
        "Bob"
    );
}

TEST_F(UpdateExecutorTest, UpdatesAllRecordsWithoutWhere) {
    FileManager file_manager(
        test_directory.string()
    );

    BufferManager buffer_manager(
        file_manager,
        10
    );

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

    const RecordId alice =
        records.insert({
            {"id", "1"},
            {"name", "Alice"}
        });

    const RecordId bob =
        records.insert({
            {"id", "2"},
            {"name", "Bob"}
        });

    Lexer lexer(
        "UPDATE student SET name = 'Updated';"
    );

    const auto tokens = lexer.tokenize();

    Parser parser(tokens);

    const auto statement = parser.parse();

    const auto plan =
        Planner().create_plan(statement);

    ASSERT_NE(plan, nullptr);

    // Apply the UPDATE to every record when no predicate is supplied.
    UpdateExecutor executor(
        *plan,
        records
    );

    EXPECT_EQ(
        executor.execute(),
        2
    );

    EXPECT_EQ(
        records.get(alice, "name"),
        "Updated"
    );

    EXPECT_EQ(
        records.get(bob, "name"),
        "Updated"
    );
}

TEST_F(UpdateExecutorTest, UpdatesMultipleMatchingRecords) {
    FileManager file_manager(
        test_directory.string()
    );

    BufferManager buffer_manager(
        file_manager,
        10
    );

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

    const RecordId first =
        records.insert({
            {"id", "1"},
            {"name", "Alice"}
        });

    const RecordId second =
        records.insert({
            {"id", "1"},
            {"name", "Bob"}
        });

    const RecordId third =
        records.insert({
            {"id", "2"},
            {"name", "Charlie"}
        });

    Lexer lexer(
        "UPDATE student SET name = 'Updated' WHERE id = 1;"
    );

    const auto tokens = lexer.tokenize();

    Parser parser(tokens);

    const auto statement = parser.parse();

    const auto plan =
        Planner().create_plan(statement);

    ASSERT_NE(plan, nullptr);

    // Verify that UPDATE operates on every matching record.
    UpdateExecutor executor(
        *plan,
        records
    );

    EXPECT_EQ(
        executor.execute(),
        2
    );

    EXPECT_EQ(
        records.get(first, "name"),
        "Updated"
    );

    EXPECT_EQ(
        records.get(second, "name"),
        "Updated"
    );

    EXPECT_EQ(
        records.get(third, "name"),
        "Charlie"
    );
}

TEST_F(UpdateExecutorTest, RejectsWrongValueType) {
    FileManager file_manager(
        test_directory.string()
    );

    BufferManager buffer_manager(
        file_manager,
        10
    );

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
        "UPDATE student SET id = 'wrong';"
    );

    const auto tokens = lexer.tokenize();

    Parser parser(tokens);

    const auto statement = parser.parse();

    const auto plan =
        Planner().create_plan(statement);

    ASSERT_NE(plan, nullptr);

    // Reject values that cannot be represented by the target column.
    UpdateExecutor executor(
        *plan,
        records
    );

    EXPECT_THROW(
        executor.execute(),
        std::invalid_argument
    );
}

TEST_F(UpdateExecutorTest, RejectsUnknownColumn) {
    FileManager file_manager(
        test_directory.string()
    );

    BufferManager buffer_manager(
        file_manager,
        10
    );

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
        "UPDATE student SET unknown = 'value';"
    );

    const auto tokens = lexer.tokenize();

    Parser parser(tokens);

    const auto statement = parser.parse();

    const auto plan =
        Planner().create_plan(statement);

    ASSERT_NE(plan, nullptr);

    // Protect the storage layer from writes to nonexistent columns.
    UpdateExecutor executor(
        *plan,
        records
    );

    EXPECT_THROW(
        executor.execute(),
        std::invalid_argument
    );
}

TEST_F(UpdateExecutorTest, UpdatesIntegerColumn) {
    FileManager file_manager(
        test_directory.string()
    );

    BufferManager buffer_manager(
        file_manager,
        10
    );

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

    const RecordId rid =
        records.insert({
            {"id", "1"},
            {"name", "Alice"}
        });

    Lexer lexer(
        "UPDATE student SET id = 10 WHERE id = 1;"
    );

    const auto tokens = lexer.tokenize();

    Parser parser(tokens);

    const auto statement = parser.parse();

    const auto plan =
        Planner().create_plan(statement);

    ASSERT_NE(plan, nullptr);

    // Verify that numeric SQL values reach integer storage correctly.
    UpdateExecutor executor(
        *plan,
        records
    );

    EXPECT_EQ(
        executor.execute(),
        1
    );

    EXPECT_EQ(
        records.get(rid, "id"),
        "10"
    );
}

} // namespace flashdb