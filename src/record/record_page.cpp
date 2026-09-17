#include "record_page.h"

#include <cstring>
#include <stdexcept>

namespace flashdb {

RecordPage::RecordPage(
    Page& page,
    const Schema& schema)
    : page_(page),
      schema_(schema),
      record_size_(calculate_record_size()) {}

std::size_t RecordPage::calculate_record_size() const {
    std::size_t size = 0;

    for (std::size_t i = 0; i < schema_.field_count(); ++i) {
        if (schema_.is_int(i)) {
            size += sizeof(int32_t);
        } else {
            size += schema_.string_length(i);
        }
    }

    return size;
}

int RecordPage::insert(const std::string& values) {
    if (values.size() > record_size_) {
        throw std::runtime_error(
            "Record is larger than record slot");
    }

    const std::size_t header_size = sizeof(int32_t);
    const std::size_t capacity =
        (Page::PAGE_SIZE - header_size) / record_size_;

    int32_t slot_count = page_.get_int(0);

    if (slot_count < 0 ||
        static_cast<std::size_t>(slot_count) >= capacity) {
        throw std::runtime_error(
            "Record page is full");
    }

    const std::size_t offset =
        header_size +
        static_cast<std::size_t>(slot_count) * record_size_;

    std::memcpy(
        page_.data().data() + offset,
        values.data(),
        values.size());

    ++slot_count;
    page_.set_int(0, slot_count);

    return slot_count - 1;
}

std::string RecordPage::get(int slot) const {
    if (slot < 0) {
        throw std::out_of_range("Invalid record slot");
    }

    const std::size_t header_size = sizeof(int32_t);
    const int32_t slot_count = page_.get_int(0);

    if (slot >= slot_count) {
        throw std::out_of_range("Invalid record slot");
    }

    const std::size_t offset =
        header_size +
        static_cast<std::size_t>(slot) * record_size_;

    return std::string(
        reinterpret_cast<const char*>(
            page_.data().data() + offset),
        record_size_);
}

} // namespace flashdb