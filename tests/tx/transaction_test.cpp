#include <filesystem>
#include <gtest/gtest.h>

#include "tx/transaction.h"

using namespace flashdb;

TEST(TransactionTest, GetsUniqueTransactionId) {
    std::filesystem::remove_all("test_tx_db");

    FileManager file_manager("test_tx_db");
    LogManager log_manager("test_tx_db");
    BufferManager buffer_manager(file_manager, 2);

    Transaction first(
        file_manager,
        log_manager,
        buffer_manager);

    Transaction second(
        file_manager,
        log_manager,
        buffer_manager);

    EXPECT_NE(first.id(), second.id());

    std::filesystem::remove_all("test_tx_db");
}

TEST(TransactionTest, CommitReturnsTransactionId) {
    std::filesystem::remove_all("test_tx_db");

    FileManager file_manager("test_tx_db");
    LogManager log_manager("test_tx_db");
    BufferManager buffer_manager(file_manager, 2);

    Transaction transaction(
        file_manager,
        log_manager,
        buffer_manager);

    EXPECT_EQ(transaction.commit(), transaction.id());

    std::filesystem::remove_all("test_tx_db");
}