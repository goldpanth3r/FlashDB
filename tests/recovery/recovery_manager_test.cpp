#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>

#include <gtest/gtest.h>

#include "database.h"
#include "log/log_manager.h"
#include "query/create_table_executor.h"
#include "recovery/recovery_manager.h"
#include "tx/transaction.h"

namespace flashdb {
namespace {

class RecoveryManagerTest : public ::testing::Test {
protected:
    std::filesystem::path database_directory =
        "test_recovery_database";

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
    }

    void TearDown() override {
        std::filesystem::remove_all(
            database_directory
        );
    }
};

TEST_F(
    RecoveryManagerTest,
    UndoesIncompleteInsert
) {
    {
        Database database(
            database_directory.string()
        );

        LogManager log_manager(
            database_directory.string()
        );

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

        // Simulate a crash by destroying the transaction
        // without committing or rolling back.
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

        EXPECT_EQ(
            table->page_count(),
            1
        );

        EXPECT_TRUE(
            table->scan().empty()
        );
    }
}

} // namespace
} // namespace flashdb