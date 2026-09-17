#include "buffer_manager.h"

#include <stdexcept>

namespace flashdb {

/**
 * BufferManager creates a fixed-size pool of memory buffers.
 *
 * @param file_manager File manager used to read and write database pages.
 * @param buffer_count Number of buffers in the pool.
 */
BufferManager::BufferManager(
    FileManager& file_manager,
    std::size_t buffer_count)
    : file_manager_(file_manager),
      buffers_(buffer_count) {
}

/**
 * get_buffer finds a database block in the buffer pool.
 *
 * If the block is already loaded, its pin count is increased.
 *
 * If the block is not loaded, this function first looks for an unused
 * buffer. If no unused buffer exists, it looks for an unpinned buffer
 * that can be replaced.
 *
 * A dirty buffer is written to disk before it is replaced.
 *
 * @param block Database block to load.
 * @return Pointer to the buffer containing the block.
 * @return nullptr if every buffer is currently pinned.
 */
Buffer* BufferManager::get_buffer(const BlockId& block) {

    // Step 1:
    // Check whether the requested block is already in memory.
    for (auto& buffer : buffers_) {

        if (buffer.is_used() &&
            buffer.block() == block) {

            // The caller is now using this buffer.
            buffer.pin();

            return &buffer;
        }
    }

    // Step 2:
    // Look for a completely unused buffer first.
    for (auto& buffer : buffers_) {

        if (!buffer.is_used()) {

            file_manager_.read(
                block,
                buffer.page()
            );

            buffer.set_block(block);

            // The caller owns one pin.
            buffer.pin();

            return &buffer;
        }
    }

    // Step 3:
    // No unused buffer exists.
    //
    // Find an existing buffer that nobody is currently using.
    for (auto& buffer : buffers_) {

        if (buffer.pin_count() != 0) {
            continue;
        }

        // Step 4:
        // If the buffer contains modified data, save it before
        // replacing the buffer.
        if (buffer.is_dirty()) {

            file_manager_.write(
                buffer.block(),
                buffer.page()
            );

            buffer.mark_clean();
        }

        // Step 5:
        // Load the requested block into this buffer.
        file_manager_.read(
            block,
            buffer.page()
        );

        buffer.set_block(block);

        // The caller is now using the newly loaded buffer.
        buffer.pin();

        return &buffer;
    }

    // Every buffer is pinned.
    //
    // We cannot safely replace any of them because another part
    // of the database is still using them.
    return nullptr;
}

/**
 * flush_buffer writes a dirty buffer to disk.
 *
 * A pinned buffer cannot be flushed by the replacement mechanism
 * because another user is currently working with it.
 *
 * @param buffer Buffer to flush.
 */
void BufferManager::flush_buffer(Buffer& buffer) {

    if (!buffer.is_used() || !buffer.is_dirty()) {
        return;
    }

    if (buffer.pin_count() != 0) {
        throw std::runtime_error(
            "BufferManager::flush_buffer: buffer is pinned"
        );
    }

    file_manager_.write(
        buffer.block(),
        buffer.page()
    );

    buffer.mark_clean();
}

/**
 * flush_all writes every dirty, unpinned buffer to disk.
 *
 * Pinned buffers are skipped because they are still being used.
 */
void BufferManager::flush_all() {

    for (auto& buffer : buffers_) {

        if (!buffer.is_used()) {
            continue;
        }

        if (!buffer.is_dirty()) {
            continue;
        }

        if (buffer.pin_count() != 0) {
            continue;
        }

        file_manager_.write(
            buffer.block(),
            buffer.page()
        );

        buffer.mark_clean();
    }
}

/**
 * unpin_buffer releases one user's reference to a buffer.
 *
 * @param buffer Buffer to unpin.
 */
void BufferManager::unpin_buffer(Buffer& buffer) {
    buffer.unpin();
}

/**
 * size returns the number of buffers in the pool.
 *
 * @return Number of buffers managed by this BufferManager.
 */
std::size_t BufferManager::size() const {
    return buffers_.size();
}

} // namespace flashdb