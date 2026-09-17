#include "page.h"

#include <cstring>
#include <limits>
#include <stdexcept>

namespace flashdb {

Page::Page()
    : data_(PAGE_SIZE, std::byte{0}) {}

/**
 * get_int reads a 32-bit integer from the page.
 *
 * @param offset Byte position where the integer starts.
 * @return Stored integer value.
 */
int32_t Page::get_int(std::size_t offset) const {
    if (offset > PAGE_SIZE - sizeof(int32_t)) {
        throw std::out_of_range("Page::get_int: offset outside page");
    }

    int32_t value{};

    std::memcpy(
        &value,
        data_.data() + offset,
        sizeof(int32_t)
    );

    return value;
}

/**
 * set_int writes a 32-bit integer into the page.
 *
 * @param offset Byte position where the integer starts.
 * @param value Integer value to store.
 */
void Page::set_int(std::size_t offset, int32_t value) {
    if (offset > PAGE_SIZE - sizeof(int32_t)) {
        throw std::out_of_range("Page::set_int: offset outside page");
    }

    std::memcpy(
        data_.data() + offset,
        &value,
        sizeof(int32_t)
    );
}

/**
 * get_string reads a length-prefixed string.
 *
 * The page stores:
 *
 * [4-byte length][string bytes]
 *
 * @param offset Byte position where the string starts.
 * @return Stored string.
 */
std::string Page::get_string(std::size_t offset) const {
    const int32_t length = get_int(offset);

    if (length < 0) {
        throw std::runtime_error(
            "Page::get_string: invalid negative length"
        );
    }

    const std::size_t string_length =
        static_cast<std::size_t>(length);

    const std::size_t data_offset =
        offset + sizeof(int32_t);

    if (data_offset > PAGE_SIZE ||
        string_length > PAGE_SIZE - data_offset) {
        throw std::out_of_range(
            "Page::get_string: string outside page"
        );
    }

    const char* chars =
        reinterpret_cast<const char*>(
            data_.data() + data_offset
        );

    return std::string(chars, string_length);
}

/**
 * set_string writes a length-prefixed string.
 *
 * @param offset Byte position where the string starts.
 * @param value String to store.
 */
void Page::set_string(
    std::size_t offset,
    const std::string& value) {

    if (value.size() >
        static_cast<std::size_t>(
            std::numeric_limits<int32_t>::max())) {
        throw std::length_error(
            "Page::set_string: string is too large"
        );
    }

    const std::size_t data_offset =
        offset + sizeof(int32_t);

    if (offset > PAGE_SIZE ||
        data_offset > PAGE_SIZE ||
        value.size() > PAGE_SIZE - data_offset) {
        throw std::out_of_range(
            "Page::set_string: string outside page"
        );
    }

    set_int(
        offset,
        static_cast<int32_t>(value.size())
    );

    if (!value.empty()) {
        std::memcpy(
            data_.data() + data_offset,
            value.data(),
            value.size()
        );
    }
}

std::vector<std::byte>& Page::data() {
    return data_;
}

const std::vector<std::byte>& Page::data() const {
    return data_;
}

} // namespace flashdb