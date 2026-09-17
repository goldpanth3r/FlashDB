#include <filesystem>
#include <gtest/gtest.h>

#include "file/block_id.h"
#include "file/file_manager.h"
#include "file/page.h"

using namespace flashdb;

TEST(FileManagerTest, WriteAndReadPage) {

    std::filesystem::remove_all("testdb");

    FileManager fm("testdb");

    Page page;

    page.set_int(0, 2026);
    page.set_string(100, "FlashDB");

    BlockId block = fm.append("student.tbl");

    fm.write(block, page);

    Page result;

    fm.read(block, result);

    EXPECT_EQ(result.get_int(0), 2026);
    EXPECT_EQ(result.get_string(100), "FlashDB");
}

TEST(FileManagerTest, RejectsInvalidBlockNumber) {
    std::filesystem::remove_all("testdb");

    FileManager fm("testdb");

    Page page;
    BlockId invalid("student.tbl", -1);

    EXPECT_THROW(
        fm.read(invalid, page),
        std::invalid_argument
    );

    EXPECT_THROW(
        fm.write(invalid, page),
        std::invalid_argument
    );

    std::filesystem::remove_all("testdb");
}

TEST(FileManagerTest, TracksPageCount) {
    std::filesystem::remove_all("testdb");

    FileManager fm("testdb");

    EXPECT_EQ(fm.length("student.tbl"), 0);

    fm.append("student.tbl");
    EXPECT_EQ(fm.length("student.tbl"), 1);

    fm.append("student.tbl");
    EXPECT_EQ(fm.length("student.tbl"), 2);

    std::filesystem::remove_all("testdb");
}