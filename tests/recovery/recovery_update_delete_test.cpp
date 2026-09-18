#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

#include <gtest/gtest.h>

#include "database.h"
#include "log/log_manager.h"
#include "query/create_table_executor.h"
#include "recovery/recovery_manager.h"
#include "tx/transaction.h"

namespace flashdb {
namespace {

class RecoveryUpdateDeleteTest : public ::testing::Test {
protected:
    std::filesystem::path database_directory =
        "test_recovery_update_delete_database";

    void SetUp() override {
        std::filesystem::remove_all(
            database_directory
        );

        Database database(
            database_directory.string()
        );

        LogManager log_manager(
            database_directory.string()
        );

        Schema schema;
        schema.add_int_field("id");
        schema.add_string_field("name", 50);

        Plan plan(
            "CreateTable",
            "student",
            nullptr,
            std::nullopt,
            std::vector<Expression>{},
            std::vector<Expression>{},
            std::move(schema)
        );

        CreateTableExecutor executor(
            plan,
            database
        );

        executor.execute();

        Transaction transaction(
            database,
            log_manager
        );

        std::unordered_map<std::string, std::string> values{
            {"id", "1"},
            {"name", "Alice"}
        };

        transaction.insert(
            "student",
            values
        );

        transaction.commit();
    }

    void TearDown() override {
        std::filesystem::remove_all(
            database_directory
        );
    }
};

TEST_F(
    RecoveryUpdateDeleteTest,
    UndoesIncompleteUpdate
) {
    {
        Database database(
            database_directory.string()
        );

        LogManager log_manager(
            database_directory.string()
        );

        auto table =
            database.open_table("student");

        const auto records = table->scan();

        ASSERT_EQ(
            records.size(),
            1
        );

        Transaction transaction(
            database,
            log_manager
        );

        transaction.update(
            "student",
            records[0],
            "name",
            "Bob"
        );

        // Simulate a crash without COMMIT.
    }

    {
        Database database(
            database_directory.string()
        );

        LogManager log_manager(
            database_directory.string()
        );

        RecoveryManager recovery(
            database,
            log_manager
        );

        recovery.recover();

        auto table =
            database.open_table("student");

        const auto records = table->scan();

        ASSERT_EQ(
            records.size(),
            1
        );

        EXPECT_EQ(
            table->get(
                records[0],
                "name"
            ),
            "Alice"
        );
    }
}

TEST_F(
    RecoveryUpdateDeleteTest,
    UndoesIncompleteDelete
) {
    {
        Database database(
            database_directory.string()
        );

        LogManager log_manager(
            database_directory.string()
        );

        auto table =
            database.open_table("student");

        const auto records = table->scan();

        ASSERT_EQ(
            records.size(),
            1
        );

        Transaction transaction(
            database,
            log_manager
        );

        transaction.remove(
            "student",
            records[0]
        );

        // Simulate a crash without COMMIT.
    }

    {
        Database database(
            database_directory.string()
        );

        LogManager log_manager(
            database_directory.string()
        );

        RecoveryManager recovery(
            database,
            log_manager
        );

        recovery.recover();

        auto table =
            database.open_table("student");

        const auto records = table->scan();

        ASSERT_EQ(
            records.size(),
            1
        );

        EXPECT_EQ(
            table->get(
                records[0],
                "id"
            ),
            "1"
        );

        EXPECT_EQ(
            table->get(
                records[0],
                "name"
            ),
            "Alice"
        );
    }
}

} // namespace
} // namespace flashdb