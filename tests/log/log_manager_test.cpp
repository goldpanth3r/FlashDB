#include <filesystem>
#include <gtest/gtest.h>

#include "log/log_manager.h"

using namespace flashdb;

TEST(LogManagerTest, AppendsRecords) {
    std::filesystem::remove_all("test_log_db");

    LogManager log("test_log_db");

    std::vector<std::byte> record = {
        std::byte{'A'},
        std::byte{'B'},
        std::byte{'C'}
    };

    EXPECT_EQ(log.append(record), 0);
    EXPECT_EQ(log.append(record), 1);
    EXPECT_EQ(log.size(), 2);

    log.flush();

    EXPECT_TRUE(
        std::filesystem::exists("test_log_db/flashdb.log")
    );

    std::filesystem::remove_all("test_log_db");
}