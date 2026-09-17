#include <filesystem>
#include <gtest/gtest.h>

#include "buffer/buffer_manager.h"

using namespace flashdb;

TEST(BufferManagerTest, LoadsPageIntoBuffer) {
    std::filesystem::remove_all("test_buffer_db");

    FileManager file_manager("test_buffer_db");

    BlockId block = file_manager.append("student.tbl");

    Page page;
    page.set_int(0, 1234);
    page.set_string(100, "FlashDB");

    file_manager.write(block, page);

    BufferManager buffer_manager(file_manager, 2);

    Buffer* buffer = buffer_manager.get_buffer(block);

    ASSERT_NE(buffer, nullptr);

    EXPECT_EQ(buffer->page().get_int(0), 1234);
    EXPECT_EQ(buffer->page().get_string(100), "FlashDB");

    std::filesystem::remove_all("test_buffer_db");
}

TEST(BufferManagerTest, ReusesCachedBuffer) {
    std::filesystem::remove_all("test_buffer_db");

    FileManager file_manager("test_buffer_db");

    BlockId block = file_manager.append("student.tbl");

    Page page;
    page.set_int(0, 99);

    file_manager.write(block, page);

    BufferManager buffer_manager(file_manager, 1);

    Buffer* first = buffer_manager.get_buffer(block);
    Buffer* second = buffer_manager.get_buffer(block);

    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);

    EXPECT_EQ(first, second);

    std::filesystem::remove_all("test_buffer_db");
}

TEST(BufferManagerTest, ReturnsNullWhenFull) {
    std::filesystem::remove_all("test_buffer_db");

    FileManager file_manager("test_buffer_db");

    BlockId first_block = file_manager.append("student.tbl");
    BlockId second_block = file_manager.append("student.tbl");

    BufferManager buffer_manager(file_manager, 1);

    Buffer* first = buffer_manager.get_buffer(first_block);
    Buffer* second = buffer_manager.get_buffer(second_block);

    ASSERT_NE(first, nullptr);
    EXPECT_EQ(second, nullptr);

    std::filesystem::remove_all("test_buffer_db");
}