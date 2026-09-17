#include <filesystem>
#include <fstream>
#include <string>

#include <gtest/gtest.h>

#include "buffer/buffer_manager.h"

namespace flashdb {

class BufferManagerTest : public ::testing::Test {
protected:
    std::filesystem::path test_directory =
        std::filesystem::temp_directory_path() / "flashdb_buffer_test";

    void SetUp() override {
        std::filesystem::remove_all(test_directory);
        std::filesystem::create_directories(test_directory);
    }

    void TearDown() override {
        std::filesystem::remove_all(test_directory);
    }
};

TEST_F(BufferManagerTest, LoadsBuffer) {
    FileManager file_manager(test_directory.string());

    const BlockId block = file_manager.append("students.tbl");

    BufferManager buffer_manager(file_manager, 2);

    Buffer* buffer = buffer_manager.get_buffer(block);

    ASSERT_NE(buffer, nullptr);
    EXPECT_EQ(buffer->block(), block);
    EXPECT_EQ(buffer->pin_count(), 1);

    buffer_manager.unpin_buffer(*buffer);

    EXPECT_EQ(buffer->pin_count(), 0);
}

TEST_F(BufferManagerTest, PinsLoadedBuffer) {
    FileManager file_manager(test_directory.string());

    const BlockId block = file_manager.append("students.tbl");

    BufferManager buffer_manager(file_manager, 2);

    Buffer* first = buffer_manager.get_buffer(block);
    ASSERT_NE(first, nullptr);

    Buffer* second = buffer_manager.get_buffer(block);
    ASSERT_NE(second, nullptr);

    EXPECT_EQ(first, second);
    EXPECT_EQ(second->pin_count(), 2);

    buffer_manager.unpin_buffer(*first);
    buffer_manager.unpin_buffer(*second);

    EXPECT_EQ(second->pin_count(), 0);
}

TEST_F(BufferManagerTest, CannotUnpinMoreThanPinned) {
    FileManager file_manager(test_directory.string());

    const BlockId block = file_manager.append("students.tbl");

    BufferManager buffer_manager(file_manager, 1);

    Buffer* buffer = buffer_manager.get_buffer(block);

    ASSERT_NE(buffer, nullptr);

    buffer_manager.unpin_buffer(*buffer);

    EXPECT_THROW(
        buffer_manager.unpin_buffer(*buffer),
        std::runtime_error
    );
}

TEST_F(BufferManagerTest, CannotFlushPinnedDirtyBuffer) {
    FileManager file_manager(test_directory.string());

    const BlockId block = file_manager.append("students.tbl");

    BufferManager buffer_manager(file_manager, 1);

    Buffer* buffer = buffer_manager.get_buffer(block);

    ASSERT_NE(buffer, nullptr);

    buffer->page().set_int(0, 123);
    buffer->mark_dirty();

    EXPECT_THROW(
        buffer_manager.flush_buffer(*buffer),
        std::runtime_error
    );

    buffer_manager.unpin_buffer(*buffer);

    buffer_manager.flush_buffer(*buffer);

    EXPECT_FALSE(buffer->is_dirty());
}

TEST_F(BufferManagerTest, ReplacesUnpinnedBuffer) {
    FileManager file_manager(test_directory.string());

    const BlockId first_block =
        file_manager.append("students.tbl");

    const BlockId second_block =
        file_manager.append("students.tbl");

    BufferManager buffer_manager(file_manager, 1);

    Buffer* first = buffer_manager.get_buffer(first_block);

    ASSERT_NE(first, nullptr);
    EXPECT_EQ(first->block(), first_block);

    buffer_manager.unpin_buffer(*first);

    Buffer* second = buffer_manager.get_buffer(second_block);

    ASSERT_NE(second, nullptr);
    EXPECT_EQ(second->block(), second_block);
    EXPECT_EQ(second->pin_count(), 1);

    buffer_manager.unpin_buffer(*second);
}

TEST_F(BufferManagerTest, WritesDirtyPageBeforeReplacement) {
    FileManager file_manager(test_directory.string());

    const BlockId first_block =
        file_manager.append("students.tbl");

    const BlockId second_block =
        file_manager.append("students.tbl");

    BufferManager buffer_manager(file_manager, 1);

    // Load the first page.
    Buffer* first = buffer_manager.get_buffer(first_block);

    ASSERT_NE(first, nullptr);

    // Modify the page.
    first->page().set_int(0, 12345);
    first->mark_dirty();

    // Release it so it becomes eligible for replacement.
    buffer_manager.unpin_buffer(*first);

    // Request another page.
    //
    // The first page must be written to disk before its
    // buffer frame is reused.
    Buffer* second = buffer_manager.get_buffer(second_block);

    ASSERT_NE(second, nullptr);
    EXPECT_EQ(second->block(), second_block);

    buffer_manager.unpin_buffer(*second);

    // Read the first page directly from disk.
    Page page;

    file_manager.read(first_block, page);

    EXPECT_EQ(page.get_int(0), 12345);
}

TEST_F(BufferManagerTest, DoesNotReplacePinnedBuffer) {
    FileManager file_manager(test_directory.string());

    const BlockId first_block =
        file_manager.append("students.tbl");

    const BlockId second_block =
        file_manager.append("students.tbl");

    BufferManager buffer_manager(file_manager, 1);

    // Keep the first buffer pinned.
    Buffer* first = buffer_manager.get_buffer(first_block);

    ASSERT_NE(first, nullptr);
    EXPECT_EQ(first->pin_count(), 1);

    // There is only one buffer and it is pinned.
    //
    // Therefore the second page cannot be loaded.
    Buffer* second = buffer_manager.get_buffer(second_block);

    EXPECT_EQ(second, nullptr);

    // Clean up the first buffer.
    buffer_manager.unpin_buffer(*first);
}

TEST_F(BufferManagerTest, FlushAllWritesDirtyBuffers) {
    FileManager file_manager(test_directory.string());

    const BlockId first_block =
        file_manager.append("students.tbl");

    const BlockId second_block =
        file_manager.append("students.tbl");

    BufferManager buffer_manager(file_manager, 2);

    Buffer* first = buffer_manager.get_buffer(first_block);
    Buffer* second = buffer_manager.get_buffer(second_block);

    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);

    first->page().set_int(0, 111);
    first->mark_dirty();

    second->page().set_int(0, 222);
    second->mark_dirty();

    buffer_manager.unpin_buffer(*first);
    buffer_manager.unpin_buffer(*second);

    buffer_manager.flush_all();

    EXPECT_FALSE(first->is_dirty());
    EXPECT_FALSE(second->is_dirty());

    Page first_page;
    Page second_page;

    file_manager.read(first_block, first_page);
    file_manager.read(second_block, second_page);

    EXPECT_EQ(first_page.get_int(0), 111);
    EXPECT_EQ(second_page.get_int(0), 222);
}

TEST_F(BufferManagerTest, FlushAllSkipsPinnedDirtyBuffers) {
    FileManager file_manager(test_directory.string());

    const BlockId block =
        file_manager.append("students.tbl");

    BufferManager buffer_manager(file_manager, 1);

    Buffer* buffer = buffer_manager.get_buffer(block);

    ASSERT_NE(buffer, nullptr);

    buffer->page().set_int(0, 999);
    buffer->mark_dirty();

    // The buffer is still pinned.
    buffer_manager.flush_all();

    EXPECT_TRUE(buffer->is_dirty());

    buffer_manager.unpin_buffer(*buffer);

    buffer_manager.flush_all();

    EXPECT_FALSE(buffer->is_dirty());

    Page page;
    file_manager.read(block, page);

    EXPECT_EQ(page.get_int(0), 999);
}

} // namespace flashdb