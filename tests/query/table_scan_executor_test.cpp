#include <filesystem>

#include <gtest/gtest.h>

#include "buffer/buffer_manager.h"
#include "file/file_manager.h"
#include "query/table_scan_executor.h"
#include "record/layout.h"
#include "record/record_file.h"
#include "record/schema.h"

namespace flashdb {

class TableScanExecutorTest : public ::testing::Test {
protected:
    std::filesystem::path test_directory;

    void SetUp() override {
        test_directory =
            std::filesystem::temp_directory_path()
            / "flashdb_table_scan_executor_test";

        std::filesystem::remove_all(test_directory);
        std::filesystem::create_directories(test_directory);
    }

    void TearDown() override {
        std::filesystem::remove_all(test_directory);
    }
};

TEST_F(TableScanExecutorTest, ScansRecords) {
    FileManager file_manager(
        test_directory.string()
    );

    BufferManager buffer_manager(
        file_manager,
        3
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
            {"id", "2"},
            {"name", "Bob"}
        });

    TableScanExecutor executor(records);

    executor.open();

    ASSERT_TRUE(executor.has_next());

    const RecordId first_result =
        executor.next();

    EXPECT_EQ(
        first_result.page_number(),
        first.page_number()
    );

    EXPECT_EQ(
        first_result.slot_number(),
        first.slot_number()
    );

    ASSERT_TRUE(executor.has_next());

    const RecordId second_result =
        executor.next();

    EXPECT_EQ(
        second_result.page_number(),
        second.page_number()
    );

    EXPECT_EQ(
        second_result.slot_number(),
        second.slot_number()
    );

    EXPECT_FALSE(executor.has_next());

    executor.close();

    EXPECT_FALSE(executor.has_next());
}

TEST_F(TableScanExecutorTest, ThrowsWhenNoRecordsRemain) {
    FileManager file_manager(
        test_directory.string()
    );

    BufferManager buffer_manager(
        file_manager,
        3
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

    TableScanExecutor executor(records);

    executor.open();

    EXPECT_THROW(
        executor.next(),
        std::runtime_error
    );
}

} // namespace flashdb