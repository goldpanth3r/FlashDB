#include <filesystem>

#include <gtest/gtest.h>

#include "buffer/buffer_manager.h"
#include "file/file_manager.h"
#include "parser/lexer.h"
#include "parser/parser.h"
#include "planner/planner.h"
#include "query/delete_executor.h"
#include "record/layout.h"
#include "record/record_file.h"
#include "record/schema.h"

namespace flashdb {

class DeleteExecutorTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_directory =
            std::filesystem::temp_directory_path()
            / "flashdb_delete_executor_test";

        std::filesystem::remove_all(test_directory);
        std::filesystem::create_directories(test_directory);

        FileManager file_manager(
            test_directory.string()
        );

        BufferManager buffer_manager(
            file_manager,
            10
        );

        file_manager_ =
            std::make_unique<FileManager>(
                test_directory.string()
            );

        buffer_manager_ =
            std::make_unique<BufferManager>(
                *file_manager_,
                10
            );

        schema_.add_int_field("id");
        schema_.add_string_field("name", 50);

        layout_ = std::make_unique<Layout>(schema_);

        record_file_ =
            std::make_unique<RecordFile>(
                *file_manager_,
                *buffer_manager_,
                "student",
                *layout_
            );
    }

    void TearDown() override {
        record_file_.reset();
        layout_.reset();
        buffer_manager_.reset();
        file_manager_.reset();

        std::filesystem::remove_all(test_directory);
    }

    std::unique_ptr<FileManager> file_manager_;
    std::unique_ptr<BufferManager> buffer_manager_;
    std::unique_ptr<Layout> layout_;
    std::unique_ptr<RecordFile> record_file_;

    Schema schema_;
    std::filesystem::path test_directory;
};

TEST_F(DeleteExecutorTest, DeletesMatchingRecord) {
    record_file_->insert({
        {"id", "1"},
        {"name", "Alice"}
    });

    record_file_->insert({
        {"id", "2"},
        {"name", "Bob"}
    });

    Lexer lexer(
        "DELETE FROM student WHERE id = 1;"
    );

    const auto tokens = lexer.tokenize();
    Parser parser(tokens);

    const auto statement = parser.parse();

    Planner planner;

    const auto plan = planner.create_plan(statement);

    DeleteExecutor executor(
        *plan,
        *record_file_
    );

    EXPECT_EQ(executor.execute(), 1u);

    const auto records = record_file_->scan();

    ASSERT_EQ(records.size(), 1u);
    EXPECT_EQ(
        record_file_->get(records[0], "id"),
        "2"
    );
}

TEST_F(DeleteExecutorTest, DeletesAllRecordsWithoutWhere) {
    record_file_->insert({
        {"id", "1"},
        {"name", "Alice"}
    });

    record_file_->insert({
        {"id", "2"},
        {"name", "Bob"}
    });

    record_file_->insert({
        {"id", "3"},
        {"name", "Charlie"}
    });

    Lexer lexer(
        "DELETE FROM student;"
    );

    const auto tokens = lexer.tokenize();
    Parser parser(tokens);

    const auto statement = parser.parse();

    Planner planner;

    const auto plan = planner.create_plan(statement);

    DeleteExecutor executor(
        *plan,
        *record_file_
    );

    EXPECT_EQ(executor.execute(), 3u);
    EXPECT_TRUE(record_file_->scan().empty());
}

TEST_F(DeleteExecutorTest, DeletesMultipleMatchingRecords) {
    record_file_->insert({
        {"id", "1"},
        {"name", "Alice"}
    });

    record_file_->insert({
        {"id", "1"},
        {"name", "Bob"}
    });

    record_file_->insert({
        {"id", "2"},
        {"name", "Charlie"}
    });

    Lexer lexer(
        "DELETE FROM student WHERE id = 1;"
    );

    const auto tokens = lexer.tokenize();
    Parser parser(tokens);

    const auto statement = parser.parse();

    Planner planner;

    const auto plan = planner.create_plan(statement);

    DeleteExecutor executor(
        *plan,
        *record_file_
    );

    EXPECT_EQ(executor.execute(), 2u);

    const auto records = record_file_->scan();

    ASSERT_EQ(records.size(), 1u);
    EXPECT_EQ(
        record_file_->get(records[0], "id"),
        "2"
    );
}

TEST_F(DeleteExecutorTest, DeletesNoMatchingRecords) {
    record_file_->insert({
        {"id", "1"},
        {"name", "Alice"}
    });

    record_file_->insert({
        {"id", "2"},
        {"name", "Bob"}
    });

    Lexer lexer(
        "DELETE FROM student WHERE id = 99;"
    );

    const auto tokens = lexer.tokenize();
    Parser parser(tokens);

    const auto statement = parser.parse();

    Planner planner;

    const auto plan = planner.create_plan(statement);

    DeleteExecutor executor(
        *plan,
        *record_file_
    );

    EXPECT_EQ(executor.execute(), 0u);

    EXPECT_EQ(record_file_->scan().size(), 2u);
}

TEST_F(DeleteExecutorTest, PreservesNonMatchingRecords) {
    record_file_->insert({
        {"id", "1"},
        {"name", "Alice"}
    });

    record_file_->insert({
        {"id", "2"},
        {"name", "Bob"}
    });

    Lexer lexer(
        "DELETE FROM student WHERE id = 1;"
    );

    const auto tokens = lexer.tokenize();
    Parser parser(tokens);

    const auto statement = parser.parse();

    Planner planner;

    const auto plan = planner.create_plan(statement);

    DeleteExecutor executor(
        *plan,
        *record_file_
    );

    EXPECT_EQ(executor.execute(), 1u);

    const auto records = record_file_->scan();

    ASSERT_EQ(records.size(), 1u);
    EXPECT_EQ(
        record_file_->get(records[0], "name"),
        "Bob"
    );
}

}