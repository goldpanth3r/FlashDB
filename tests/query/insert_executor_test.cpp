#include <filesystem>

#include <gtest/gtest.h>

#include "buffer/buffer_manager.h"
#include "file/file_manager.h"
#include "parser/lexer.h"
#include "parser/parser.h"
#include "planner/planner.h"
#include "query/insert_executor.h"
#include "record/layout.h"
#include "record/record_file.h"
#include "record/schema.h"

namespace flashdb {

class InsertExecutorTest : public ::testing::Test {
protected:
    std::filesystem::path test_directory;

    void SetUp() override {
        test_directory =
            std::filesystem::temp_directory_path()
            / "flashdb_insert_executor_test";

        std::filesystem::remove_all(test_directory);
        std::filesystem::create_directories(test_directory);
    }

    void TearDown() override {
        std::filesystem::remove_all(test_directory);
    }
};

TEST_F(InsertExecutorTest, InsertsRecord) {
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

    Lexer lexer(
        "INSERT INTO student VALUES (1, 'Alice');"
    );

    const auto tokens = lexer.tokenize();

    Parser parser(tokens);

    const auto statement = parser.parse();

    Planner planner;

    const auto plan =
        planner.create_plan(statement);

    ASSERT_NE(plan, nullptr);
    EXPECT_EQ(plan->get_name(), "Insert");

    // Execute the planned INSERT operation.
    InsertExecutor executor(
        *plan,
        records
    );

    const RecordId rid =
        executor.execute();

    EXPECT_EQ(
        records.get(rid, "id"),
        "1"
    );

    EXPECT_EQ(
        records.get(rid, "name"),
        "Alice"
    );
}

TEST_F(InsertExecutorTest, InsertsMultipleRecords) {
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

    Planner planner;

    Lexer lexer1(
        "INSERT INTO student VALUES (1, 'Alice');"
    );

    const auto tokens1 = lexer1.tokenize();

    Parser parser1(tokens1);

    const auto plan1 =
        planner.create_plan(parser1.parse());

    ASSERT_NE(plan1, nullptr);

    InsertExecutor executor1(
        *plan1,
        records
    );

    executor1.execute();

    Lexer lexer2(
        "INSERT INTO student VALUES (2, 'Bob');"
    );

    const auto tokens2 = lexer2.tokenize();

    Parser parser2(tokens2);

    const auto plan2 =
        planner.create_plan(parser2.parse());

    ASSERT_NE(plan2, nullptr);

    InsertExecutor executor2(
        *plan2,
        records
    );

    executor2.execute();

    const auto record_ids =
        records.scan();

    ASSERT_EQ(record_ids.size(), 2);

    EXPECT_EQ(
        records.get(record_ids[0], "name"),
        "Alice"
    );

    EXPECT_EQ(
        records.get(record_ids[1], "name"),
        "Bob"
    );
}

TEST_F(InsertExecutorTest, RejectsWrongValueCount) {
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

    Lexer lexer(
        "INSERT INTO student VALUES (1);"
    );

    const auto tokens = lexer.tokenize();

    Parser parser(tokens);

    const auto plan =
        Planner().create_plan(parser.parse());

    ASSERT_NE(plan, nullptr);

    // Reject INSERT statements whose value count does not match the schema.
    InsertExecutor executor(
        *plan,
        records
    );

    EXPECT_THROW(
        executor.execute(),
        std::invalid_argument
    );
}

TEST_F(InsertExecutorTest, RejectsWrongValueType) {
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

    Lexer lexer(
        "INSERT INTO student VALUES ('wrong', 'Alice');"
    );

    const auto tokens = lexer.tokenize();

    Parser parser(tokens);

    const auto plan =
        Planner().create_plan(parser.parse());

    ASSERT_NE(plan, nullptr);

    // Validate column types before writing to storage.
    InsertExecutor executor(
        *plan,
        records
    );

    EXPECT_THROW(
        executor.execute(),
        std::invalid_argument
    );
}

} // namespace flashdb