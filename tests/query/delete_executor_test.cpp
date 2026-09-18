#include <filesystem>
#include <memory>
#include <string>

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
    std::filesystem::path test_directory;

    std::unique_ptr<FileManager> file_manager;
    std::unique_ptr<BufferManager> buffer_manager;
    std::unique_ptr<Schema> schema;
    std::unique_ptr<Layout> layout;
    std::unique_ptr<RecordFile> record_file;

    void SetUp() override {
        test_directory =
            std::filesystem::temp_directory_path()
            / "flashdb_delete_executor_test";

        std::filesystem::remove_all(test_directory);
        std::filesystem::create_directories(test_directory);

        file_manager =
            std::make_unique<FileManager>(test_directory);

        buffer_manager =
            std::make_unique<BufferManager>(
                *file_manager,
                10
            );

        schema = std::make_unique<Schema>();

        schema->add_int_field("id");
        schema->add_string_field("name", 50);

        layout = std::make_unique<Layout>(*schema);

        record_file =
            std::make_unique<RecordFile>(
                *file_manager,
                *buffer_manager,
                "student",
                *layout
            );
    }

    void TearDown() override {
        record_file.reset();
        layout.reset();
        schema.reset();
        buffer_manager.reset();
        file_manager.reset();

        std::filesystem::remove_all(test_directory);
    }

    void insert_student(
        const std::string& id,
        const std::string& name) {

        std::unordered_map<std::string, std::string> values;

        values["id"] = id;
        values["name"] = name;

        record_file->insert(values);
    }

    std::unique_ptr<Plan> create_plan(
        const std::string& sql) {

        Lexer lexer(sql);

        const auto tokens = lexer.tokenize();
        Parser parser(tokens);

        const auto statement = parser.parse();

        Planner planner;

        return planner.create_plan(statement);
    }
};

// Delete only the record selected by the WHERE predicate.
TEST_F(DeleteExecutorTest, DeletesMatchingRecord) {
    insert_student("1", "Alice");
    insert_student("2", "Bob");

    auto plan =
        create_plan(
            "DELETE FROM student WHERE id = 1;"
        );

    DeleteExecutor executor(
        *plan,
        *record_file
    );

    EXPECT_EQ(executor.execute(), 1u);
    EXPECT_EQ(record_file->scan().size(), 1u);

    const auto remaining = record_file->scan();

    EXPECT_EQ(
        record_file->get(remaining[0], "id"),
        "2"
    );

    EXPECT_EQ(
        record_file->get(remaining[0], "name"),
        "Bob"
    );
}

// Delete every record when DELETE has no WHERE predicate.
TEST_F(DeleteExecutorTest, DeletesAllRecordsWithoutWhere) {
    insert_student("1", "Alice");
    insert_student("2", "Bob");
    insert_student("3", "Charlie");

    auto plan =
        create_plan(
            "DELETE FROM student;"
        );

    DeleteExecutor executor(
        *plan,
        *record_file
    );

    EXPECT_EQ(executor.execute(), 3u);
    EXPECT_TRUE(record_file->scan().empty());
}

// Delete every record matching the same predicate.
TEST_F(DeleteExecutorTest, DeletesMultipleMatchingRecords) {
    insert_student("1", "Alice");
    insert_student("1", "Alex");
    insert_student("2", "Bob");

    auto plan =
        create_plan(
            "DELETE FROM student WHERE id = 1;"
        );

    DeleteExecutor executor(
        *plan,
        *record_file
    );

    EXPECT_EQ(executor.execute(), 2u);
    EXPECT_EQ(record_file->scan().size(), 1u);

    const auto remaining = record_file->scan();

    EXPECT_EQ(
        record_file->get(remaining[0], "id"),
        "2"
    );
}

// Return zero when the DELETE predicate matches nothing.
TEST_F(DeleteExecutorTest, DeletesNoMatchingRecords) {
    insert_student("1", "Alice");
    insert_student("2", "Bob");

    auto plan =
        create_plan(
            "DELETE FROM student WHERE id = 99;"
        );

    DeleteExecutor executor(
        *plan,
        *record_file
    );

    EXPECT_EQ(executor.execute(), 0u);
    EXPECT_EQ(record_file->scan().size(), 2u);
}

// Keep records that do not satisfy the DELETE predicate.
TEST_F(DeleteExecutorTest, PreservesNonMatchingRecords) {
    insert_student("1", "Alice");
    insert_student("2", "Bob");
    insert_student("3", "Charlie");

    auto plan =
        create_plan(
            "DELETE FROM student WHERE id = 2;"
        );

    DeleteExecutor executor(
        *plan,
        *record_file
    );

    EXPECT_EQ(executor.execute(), 1u);

    const auto remaining = record_file->scan();

    ASSERT_EQ(remaining.size(), 2u);

    EXPECT_EQ(
        record_file->get(remaining[0], "id"),
        "1"
    );

    EXPECT_EQ(
        record_file->get(remaining[1], "id"),
        "3"
    );
}

// Reject plans that belong to another execution operation.
TEST_F(DeleteExecutorTest, RejectsWrongPlanType) {
    auto plan =
        create_plan(
            "DELETE FROM student;"
        );

    Plan wrong_plan(
        "Update",
        "student"
    );

    DeleteExecutor executor(
        wrong_plan,
        *record_file
    );

    EXPECT_THROW(
        executor.execute(),
        std::invalid_argument
    );
}

}