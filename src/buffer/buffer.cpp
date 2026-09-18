#include "buffer.h"

#include <stdexcept>

namespace flashdb {

// Start with an unused buffer that has no associated WAL record.
Buffer::Buffer()
    : page_(),
      block_("", -1),
      used_(false),
      dirty_(false),
      pin_count_(0),
      log_sequence_number_(
          std::numeric_limits<std::size_t>::max()) {
}

// Provide access to the in-memory database page.
Page& Buffer::page() {
    return page_;
}

// Provide read-only access to the in-memory database page.
const Page& Buffer::page() const {
    return page_;
}

// Identify the database block currently held by the buffer.
const BlockId& Buffer::block() const {
    return block_;
}

// Check whether this buffer currently owns a database block.
bool Buffer::is_used() const {
    return used_;
}

// Check whether the in-memory page differs from disk.
bool Buffer::is_dirty() const {
    return dirty_;
}

// Return the number of active users of this buffer.
std::size_t Buffer::pin_count() const {
    return pin_count_;
}

// Return the WAL record required before this page can be flushed.
std::size_t Buffer::log_sequence_number() const {
    return log_sequence_number_;
}

// Associate the buffer with a database block and reset its WAL state.
void Buffer::set_block(const BlockId& block) {
    block_ = block;
    used_ = true;
    dirty_ = false;
    pin_count_ = 0;
    log_sequence_number_ =
        std::numeric_limits<std::size_t>::max();
}

// Associate the page with the log record describing its modification.
void Buffer::set_log_sequence_number(
    std::size_t lsn) {

    if (!used_) {
        throw std::runtime_error(
            "Buffer::set_log_sequence_number: buffer is unused"
        );
    }

    log_sequence_number_ = lsn;
}

// Mark the page as modified in memory.
void Buffer::mark_dirty() {
    if (!used_) {
        throw std::runtime_error(
            "Buffer::mark_dirty: buffer is unused"
        );
    }

    dirty_ = true;
}

// Mark the page as synchronized with its durable state.
void Buffer::mark_clean() {
    dirty_ = false;
}

// Add one active user to the buffer.
void Buffer::pin() {
    if (!used_) {
        throw std::runtime_error(
            "Buffer::pin: buffer is unused"
        );
    }

    ++pin_count_;
}

// Remove one active user from the buffer.
void Buffer::unpin() {
    if (pin_count_ == 0) {
        throw std::runtime_error(
            "Buffer::unpin: buffer is not pinned"
        );
    }

    --pin_count_;
}

} // namespace flashdb