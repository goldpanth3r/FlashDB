#include <filesystem>
#include <stdexcept>
#include <vector>

#include <gtest/gtest.h>

#include "buffer/buffer_manager.h"
#include "file/file_manager.h"
#include "parser/ast/expression.h"
#include "query/project_executor.h"
#include "query/table_scan_executor.h"
#include "record/layout.h"
#include "record/record_file.h"
#include "record/schema.h"

namespace flashdb {

class ProjectExecutorTest : public ::testing::Test {
protected:
    std::filesystem::path test_directory;

    void SetUp() override {
        test_directory =
            std::filesystem::temp_directory_path()
            / "flashdb_project_executor_test";

        std::filesystem::remove_all(test_directory);
        std::filesystem::create_directories(test_directory);
    }

    void TearDown() override {
        std::filesystem::remove_all(test_directory);
    }
};

TEST_F(ProjectExecutorTest, ReturnsSelectedColumns) {
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

    const std::vector<Expression> columns = {
        Expression(
            ExpressionType::IDENTIFIER,
            "name"
        )
    };

    ProjectExecutor executor(
        table_scan,
        records,
        columns
    );

    executor.open();

    ASSERT_TRUE(executor.has_next());

    const std::vector<std::string> first =
        executor.next();

    ASSERT_EQ(first.size(), 1);
    EXPECT_EQ(first[0], "Alice");

    ASSERT_TRUE(executor.has_next());

    const std::vector<std::string> second =
        executor.next();

    ASSERT_EQ(second.size(), 1);
    EXPECT_EQ(second[0], "Bob");

    EXPECT_FALSE(executor.has_next());

    executor.close();

    EXPECT_FALSE(executor.has_next());
}

TEST_F(ProjectExecutorTest, ReturnsMultipleSelectedColumns) {
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

    TableScanExecutor table_scan(records);

    const std::vector<Expression> columns = {
        Expression(
            ExpressionType::IDENTIFIER,
            "id"
        ),
        Expression(
            ExpressionType::IDENTIFIER,
            "name"
        )
    };

    ProjectExecutor executor(
        table_scan,
        records,
        columns
    );

    executor.open();

    ASSERT_TRUE(executor.has_next());

    const std::vector<std::string> result =
        executor.next();

    ASSERT_EQ(result.size(), 2);

    EXPECT_EQ(result[0], "1");
    EXPECT_EQ(result[1], "Alice");

    EXPECT_FALSE(executor.has_next());

    executor.close();
}

TEST_F(ProjectExecutorTest, ThrowsWhenNoRecordsRemain) {
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

    TableScanExecutor table_scan(records);

    const std::vector<Expression> columns = {
        Expression(
            ExpressionType::IDENTIFIER,
            "name"
        )
    };

    ProjectExecutor executor(
        table_scan,
        records,
        columns
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