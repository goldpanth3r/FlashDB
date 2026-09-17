#include "block_id.h"

#include <utility>

namespace flashdb {

// Store the file name and block number.
BlockId::BlockId(std::string filename, int block_number)
    : filename_(std::move(filename)),
      block_number_(block_number) {}

// Return the database file name.
const std::string& BlockId::filename() const {
    return filename_;
}

// Return the block number.
int BlockId::number() const {
    return block_number_;
}

// Check whether two BlockIds refer to the same block.
bool BlockId::operator==(const BlockId& other) const {
    return filename_ == other.filename_
        && block_number_ == other.block_number_;
}

// Create a readable representation.
std::string BlockId::to_string() const {
    return filename_ + "[" + std::to_string(block_number_) + "]";
}

} // namespace flashdb