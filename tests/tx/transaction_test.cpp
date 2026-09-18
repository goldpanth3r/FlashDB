#include <filesystem>
#include <string>
#include <unordered_map>

#include <gtest/gtest.h>

#include "database.h"
#include "log/log_manager.h"
#include "record/record_file.h"
#include "record/schema.h"
#include "tx/transaction.h"

using namespace flashdb;

namespace {

class TransactionTest : public ::testing::Test {
protected:
    std::filesystem::path database_path;

    // Create a fresh database directory for every transaction test.
    void SetUp() override {
        database_path =
            std::filesystem::temp_directory_path() /
            "flashdb_transaction_test";

        std::filesystem::remove_all(database_path);
        std::filesystem::create_directories(database_path);
    }

    // Remove the temporary database after the test finishes.
    void TearDown() override {
        std::filesystem::remove_all(database_path);
    }

    // Create the table metadata and physical table file used by the tests.
    void create_student_table(Database& database) {
        Schema schema;
        schema.add_int_field("id");
        schema.add_string_field("name", 50);

        database.catalog().create_table(
            "student",
            schema
        );

        database.catalog().save();

        database.file_manager().append(
            "student.tbl"
        );
    }
};

} // namespace

TEST_F(TransactionTest, GetsUniqueTransactionId) {
    std::filesystem::remove_all("test_tx_db");

    FileManager file_manager("test_tx_db");
    LogManager log_manager("test_tx_db");
    BufferManager buffer_manager(file_manager, 2);

    Transaction first(
        file_manager,
        log_manager,
        buffer_manager
    );

    Transaction second(
        file_manager,
        log_manager,
        buffer_manager
    );

    EXPECT_NE(first.id(), second.id());

    std::filesystem::remove_all("test_tx_db");
}

TEST_F(TransactionTest, CommitReturnsTransactionId) {
    std::filesystem::remove_all("test_tx_db");

    FileManager file_manager("test_tx_db");
    LogManager log_manager("test_tx_db");
    BufferManager buffer_manager(file_manager, 2);

    Transaction transaction(
        file_manager,
        log_manager,
        buffer_manager
    );

    EXPECT_EQ(
        transaction.commit(),
        transaction.id()
    );

    std::filesystem::remove_all("test_tx_db");
}

TEST_F(TransactionTest, RollbackInsertRemovesRecord) {
    Database database(database_path.string());
    create_student_table(database);

    LogManager log_manager(database_path.string());

    Transaction transaction(
        database,
        log_manager
    );

    RecordId rid = transaction.insert(
        "student",
        {
            {"id", "1"},
            {"name", "Alice"}
        }
    );

    transaction.rollback();

    auto table = database.open_table("student");

    EXPECT_TRUE(table->scan().empty());
    EXPECT_THROW(
        table->get(rid, "name"),
        std::runtime_error
    );
}

TEST_F(TransactionTest, RollbackUpdateRestoresOldValue) {
    Database database(database_path.string());
    create_student_table(database);

    LogManager log_manager(database_path.string());

    Transaction insert_transaction(
        database,
        log_manager
    );

    RecordId rid = insert_transaction.insert(
        "student",
        {
            {"id", "1"},
            {"name", "Alice"}
        }
    );

    insert_transaction.commit();

    Transaction update_transaction(
        database,
        log_manager
    );

    update_transaction.update(
        "student",
        rid,
        "name",
        "Bob"
    );

    EXPECT_EQ(
        database.open_table("student")->get(rid, "name"),
        "Bob"
    );

    update_transaction.rollback();

    EXPECT_EQ(
        database.open_table("student")->get(rid, "name"),
        "Alice"
    );
}

TEST_F(TransactionTest, RollbackDeleteRestoresRecord) {
    Database database(database_path.string());
    create_student_table(database);

    LogManager log_manager(database_path.string());

    Transaction insert_transaction(
        database,
        log_manager
    );

    RecordId rid = insert_transaction.insert(
        "student",
        {
            {"id", "1"},
            {"name", "Alice"}
        }
    );

    insert_transaction.commit();

    Transaction delete_transaction(
        database,
        log_manager
    );

    delete_transaction.remove(
        "student",
        rid
    );

    EXPECT_THROW(
        database.open_table("student")->get(rid, "name"),
        std::runtime_error
    );

    delete_transaction.rollback();

    EXPECT_EQ(
        database.open_table("student")->get(rid, "name"),
        "Alice"
    );
}
