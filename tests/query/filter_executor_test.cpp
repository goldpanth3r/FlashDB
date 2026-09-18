#include <filesystem>
#include <stdexcept>

#include <gtest/gtest.h>

#include "buffer/buffer_manager.h"
#include "file/file_manager.h"
#include "parser/ast/condition.h"
#include "parser/ast/expression.h"
#include "query/filter_executor.h"
#include "query/table_scan_executor.h"
#include "record/layout.h"
#include "record/record_file.h"
#include "record/schema.h"

namespace flashdb {

class FilterExecutorTest : public ::testing::Test {
protected:
    std::filesystem::path test_directory;

    void SetUp() override {
        test_directory =
            std::filesystem::temp_directory_path()
            / "flashdb_filter_executor_test";

        std::filesystem::remove_all(test_directory);
        std::filesystem::create_directories(test_directory);
    }

    void TearDown() override {
        std::filesystem::remove_all(test_directory);
    }
};

TEST_F(FilterExecutorTest, ReturnsOnlyMatchingRecords) {
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

    const RecordId alice =
        records.insert({
            {"id", "1"},
            {"name", "Alice"}
        });

    records.insert({
        {"id", "2"},
        {"name", "Bob"}
    });

    records.insert({
        {"id", "3"},
        {"name", "Charlie"}
    });

    TableScanExecutor table_scan(records);

    Condition condition(
        Expression(
            ExpressionType::IDENTIFIER,
            "id"
        ),
        "=",
        Expression(
            ExpressionType::INTEGER,
            "2"
        )
    );

    FilterExecutor executor(
        table_scan,
        records,
        condition
    );

    executor.open();

    ASSERT_TRUE(executor.has_next());

    const RecordId result = executor.next();

    EXPECT_FALSE(
        result == alice
    );

    EXPECT_EQ(
        records.get(result, "id"),
        "2"
    );

    EXPECT_EQ(
        records.get(result, "name"),
        "Bob"
    );

    EXPECT_FALSE(executor.has_next());

    executor.close();
}

TEST_F(FilterExecutorTest, ReturnsNoRecordsWhenNothingMatches) {
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

    records.insert({
        {"id", "1"},
        {"name", "Alice"}
    });

    records.insert({
        {"id", "2"},
        {"name", "Bob"}
    });

    TableScanExecutor table_scan(records);

    Condition condition(
        Expression(
            ExpressionType::IDENTIFIER,
            "id"
        ),
        "=",
        Expression(
            ExpressionType::INTEGER,
            "99"
        )
    );

    FilterExecutor executor(
        table_scan,
        records,
        condition
    );

    executor.open();

    EXPECT_FALSE(executor.has_next());

    EXPECT_THROW(
        executor.next(),
        std::runtime_error
    );

    executor.close();
}

} // namespace flashdb