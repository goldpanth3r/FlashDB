#include "buffer_manager.h"

#include <limits>
#include <stdexcept>

#include "log/log_manager.h"

namespace flashdb {

// Create the buffer pool before connecting it to the database WAL.
BufferManager::BufferManager(
    FileManager& file_manager,
    std::size_t buffer_count)
    : file_manager_(file_manager),
      buffers_(buffer_count),
      log_manager_(nullptr) {
}

// Connect the buffer pool to the database write-ahead log.
void BufferManager::set_log_manager(
    LogManager& log_manager) {

    log_manager_ = &log_manager;
}

// Find an existing page or load it into an available buffer.
Buffer* BufferManager::get_buffer(
    const BlockId& block) {

    // Reuse a page that is already cached.
    for (auto& buffer : buffers_) {

        if (buffer.is_used() &&
            buffer.block() == block) {

            buffer.pin();

            return &buffer;
        }
    }

    // Prefer a completely unused buffer.
    for (auto& buffer : buffers_) {

        if (!buffer.is_used()) {

            file_manager_.read(
                block,
                buffer.page()
            );

            buffer.set_block(block);
            buffer.pin();

            return &buffer;
        }
    }

    // Replace an unpinned buffer when the pool is full.
    for (auto& buffer : buffers_) {

        if (buffer.pin_count() != 0) {
            continue;
        }

        // Apply WAL before allowing modified page data to reach disk.
        if (buffer.is_dirty()) {
            flush_page(buffer);
        }

        file_manager_.read(
            block,
            buffer.page()
        );

        buffer.set_block(block);
        buffer.pin();

        return &buffer;
    }

    return nullptr;
}

// Flush one modified page while respecting write-ahead logging.
void BufferManager::flush_buffer(
    Buffer& buffer) {

    if (!buffer.is_used() ||
        !buffer.is_dirty()) {
        return;
    }

    if (buffer.pin_count() != 0) {
        throw std::runtime_error(
            "BufferManager::flush_buffer: buffer is pinned"
        );
    }

    flush_page(buffer);
}

// Flush all modified pages after their required WAL records are durable.
void BufferManager::flush_all() {

    for (auto& buffer : buffers_) {

        if (!buffer.is_used() ||
            !buffer.is_dirty() ||
            buffer.pin_count() != 0) {
            continue;
        }

        flush_page(buffer);
    }
}

// Release one active user of a cached page.
void BufferManager::unpin_buffer(
    Buffer& buffer) {

    buffer.unpin();
}

// Associate a modified page with the log record that describes it.
void BufferManager::set_log_sequence_number(
    Buffer& buffer,
    std::size_t lsn) {

    buffer.set_log_sequence_number(lsn);
}

// Return the number of buffers in the pool.
std::size_t BufferManager::size() const {
    return buffers_.size();
}

// Flush a page only after its WAL record has reached stable storage.
void BufferManager::flush_page(
    Buffer& buffer) {

    if (log_manager_ != nullptr) {

        const std::size_t lsn =
            buffer.log_sequence_number();

        if (lsn !=
            std::numeric_limits<std::size_t>::max()) {

            log_manager_->flush();
        }
    }

    file_manager_.write(
        buffer.block(),
        buffer.page()
    );

    buffer.mark_clean();
}

} // namespace flashdb