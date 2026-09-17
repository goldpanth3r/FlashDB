#ifndef FLASHDB_FILE_BLOCK_ID_H
#define FLASHDB_FILE_BLOCK_ID_H

#include <string>

namespace flashdb {

/**
 * BlockId identifies one fixed-size block inside a database file.
 *
 * Example:
 *   student.tbl, block 0
 *   student.tbl, block 15
 */
class BlockId {
public:
    // Create a block identifier from a file name and block number.
    BlockId(std::string filename, int block_number);

    // Return the database file name.
    const std::string& filename() const;

    // Return the block number.
    int number() const;

    // Compare two block identifiers.
    bool operator==(const BlockId& other) const;

    // Convert to a readable string.
    std::string to_string() const;

private:
    std::string filename_;
    int block_number_;
};

} // namespace flashdb

#endif