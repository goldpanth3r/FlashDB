#include "page.h"

#include <cstring>
#include <stdexcept>

namespace flashdb {

// Create a page filled with zeros.
Page::Page()
    : data_(PAGE_SIZE, std::byte{0}) {}

// Read a 32-bit integer.
int32_t Page::get_int(std::size_t offset) const {
    if (offset + sizeof(int32_t) > PAGE_SIZE)
        throw std::out_of_range("Page::get_int");

    int32_t value;
    std::memcpy(&value, data_.data() + offset, sizeof(int32_t));
    return value;
}

// Write a 32-bit integer.
void Page::set_int(std::size_t offset, int32_t value) {
    if (offset + sizeof(int32_t) > PAGE_SIZE)
        throw std::out_of_range("Page::set_int");

    std::memcpy(data_.data() + offset, &value, sizeof(int32_t));
}

// Read a string.
// Layout:
// [length][characters]
std::string Page::get_string(std::size_t offset) const {
    int32_t length = get_int(offset);

    if (offset + sizeof(int32_t) + length > PAGE_SIZE)
        throw std::out_of_range("Page::get_string");

    const char* chars =
        reinterpret_cast<const char*>(data_.data() + offset + sizeof(int32_t));

    return std::string(chars, length);
}

// Write a string.
// Layout:
// [length][characters]
void Page::set_string(std::size_t offset, const std::string& value) {
    int32_t length = static_cast<int32_t>(value.size());

    if (offset + sizeof(int32_t) + length > PAGE_SIZE)
        throw std::out_of_range("Page::set_string");

    set_int(offset, length);

    std::memcpy(
        data_.data() + offset + sizeof(int32_t),
        value.data(),
        length
    );
}

// Return mutable page bytes.
std::vector<std::byte>& Page::data() {
    return data_;
}

// Return read-only page bytes.
const std::vector<std::byte>& Page::data() const {
    return data_;
}

} // namespace flashdb