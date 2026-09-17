#ifndef FLASHDB_FILE_PAGE_H
#define FLASHDB_FILE_PAGE_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace flashdb {

/**
 * Page represents one fixed-size database page in memory.
 *
 * A page is exactly 4096 bytes and stores raw binary data.
 * Higher layers (Record, Log, Index) will use this class to
 * read and write integers, strings, and records.
 */
class Page {
public:
    // Size of one database page (4 KB).
    static constexpr std::size_t PAGE_SIZE = 4096;

    // Create an empty page filled with zeros.
    Page();

    // Read a 4-byte integer from an offset.
    int32_t get_int(std::size_t offset) const;

    // Write a 4-byte integer at an offset.
    void set_int(std::size_t offset, int32_t value);

    // Read a string from an offset.
    std::string get_string(std::size_t offset) const;

    // Write a string at an offset.
    void set_string(std::size_t offset, const std::string& value);

    // Return the raw page bytes.
    std::vector<std::byte>& data();

    // Return the raw page bytes (read only).
    const std::vector<std::byte>& data() const;

private:
    std::vector<std::byte> data_;
};

} // namespace flashdb

#endif