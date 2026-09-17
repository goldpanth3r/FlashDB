#include "buffer.h"

#include <stdexcept>

namespace flashdb {

Buffer::Buffer()
    : page_(),
      block_("", -1),
      used_(false),
      dirty_(false),
      pin_count_(0) {}

Page& Buffer::page() {
    return page_;
}

const Page& Buffer::page() const {
    return page_;
}

const BlockId& Buffer::block() const {
    return block_;
}

bool Buffer::is_used() const {
    return used_;
}

bool Buffer::is_dirty() const {
    return dirty_;
}

std::size_t Buffer::pin_count() const {
    return pin_count_;
}

void Buffer::set_block(const BlockId& block) {
    block_ = block;
    used_ = true;
    dirty_ = false;
    pin_count_ = 0;
}

void Buffer::mark_dirty() {
    if (!used_) {
        throw std::runtime_error(
            "Buffer::mark_dirty: buffer is unused"
        );
    }

    dirty_ = true;
}

void Buffer::mark_clean() {
    dirty_ = false;
}

void Buffer::pin() {
    if (!used_) {
        throw std::runtime_error(
            "Buffer::pin: buffer is unused"
        );
    }

    ++pin_count_;
}

void Buffer::unpin() {
    if (pin_count_ == 0) {
        throw std::runtime_error(
            "Buffer::unpin: buffer is not pinned"
        );
    }

    --pin_count_;
}

} // namespace flashdb