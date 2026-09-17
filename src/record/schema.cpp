#include "schema.h"

#include <stdexcept>

namespace flashdb {

void Schema::add_int_field(const std::string& name) {
    fields_.push_back({
        name,
        FieldType::INT,
        sizeof(int32_t)
    });
}

void Schema::add_string_field(
    const std::string& name,
    std::size_t length) {

    fields_.push_back({
        name,
        FieldType::STRING,
        length
    });
}

std::size_t Schema::field_count() const {
    return fields_.size();
}

const std::string& Schema::field_name(std::size_t index) const {
    if (index >= fields_.size()) {
        throw std::out_of_range("Schema field index");
    }

    return fields_[index].name;
}

bool Schema::is_int(std::size_t index) const {
    if (index >= fields_.size()) {
        throw std::out_of_range("Schema field index");
    }

    return fields_[index].type == FieldType::INT;
}

std::size_t Schema::string_length(std::size_t index) const {
    if (index >= fields_.size()) {
        throw std::out_of_range("Schema field index");
    }

    if (fields_[index].type != FieldType::STRING) {
        throw std::runtime_error(
            "Field is not a string");
    }

    return fields_[index].length;
}

} // namespace flashdb