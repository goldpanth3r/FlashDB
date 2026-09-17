#include "buffer.h"

namespace flashdb {

Buffer::Buffer()
    : page_(),
      block_("", -1),
      used_(false) {}

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

void Buffer::set_block(const BlockId& block) {
    block_ = block;
    used_ = true;
}

} // namespace flashdb