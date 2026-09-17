#include <filesystem>
#include <gtest/gtest.h>

#include "log/log_manager.h"

using namespace flashdb;

TEST(LogManagerTest, AppendsAndReadsRecords) {
    std::filesystem::remove_all("test_log_db");

    LogManager log("test_log_db");

    std::vector<std::byte> record = {
        std::byte{0x41},
        std::byte{0x42},
        std::byte{0x43}
    };

    EXPECT_EQ(log.append(record), 0);
    EXPECT_EQ(log.append(record), 1);
    EXPECT_EQ(log.size(), 2);

    log.flush();

    EXPECT_EQ(log.read(0), record);
    EXPECT_EQ(log.read(1), record);

    std::filesystem::remove_all("test_log_db");
}

TEST(LogManagerTest, RecoversRecordCountAfterRestart) {
    std::filesystem::remove_all("test_log_db");

    {
        LogManager log("test_log_db");

        std::vector<std::byte> record = {
            std::byte{0x01},
            std::byte{0x02}
        };

        EXPECT_EQ(log.append(record), 0);
        EXPECT_EQ(log.append(record), 1);

        log.flush();
    }

    {
        LogManager log("test_log_db");

        EXPECT_EQ(log.size(), 2);

        std::vector<std::byte> record = {
            std::byte{0x03}
        };

        EXPECT_EQ(log.append(record), 2);
        EXPECT_EQ(log.size(), 3);
    }

    std::filesystem::remove_all("test_log_db");
}

TEST(LogManagerTest, RejectsInvalidLsn) {
    std::filesystem::remove_all("test_log_db");

    LogManager log("test_log_db");

    EXPECT_THROW(
        log.read(0),
        std::out_of_range
    );

    std::filesystem::remove_all("test_log_db");
}