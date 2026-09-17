#include "buffer_manager.h"

namespace flashdb {

BufferManager::BufferManager(
    FileManager& file_manager,
    std::size_t buffer_count)
    : file_manager_(file_manager),
      buffers_(buffer_count) {}

Buffer* BufferManager::get_buffer(const BlockId& block) {
    // First check whether the block is already cached.
    for (auto& buffer : buffers_) {
        if (buffer.is_used() && buffer.block() == block) {
            return &buffer;
        }
    }

    // Find an unused buffer.
    for (auto& buffer : buffers_) {
        if (!buffer.is_used()) {
            file_manager_.read(block, buffer.page());
            buffer.set_block(block);
            return &buffer;
        }
    }

    // No free buffer is available.
    return nullptr;
}

std::size_t BufferManager::size() const {
    return buffers_.size();
}

} // namespace flashdb