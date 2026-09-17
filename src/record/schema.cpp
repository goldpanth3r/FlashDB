#include "schema.h"
#include <cstdint>

#include <stdexcept>

namespace flashdb {

/**
 * add_int_field adds an integer column to the schema.
 *
 * @param name Column name.
 */
void Schema::add_int_field(const std::string& name) {

    if (name.empty()) {
        throw std::invalid_argument(
            "Schema::add_int_field: name cannot be empty"
        );
    }

    fields_.push_back({
        name,
        FieldType::INT,
        sizeof(int32_t)
    });
}

/**
 * add_string_field adds a string column to the schema.
 *
 * @param name Column name.
 * @param length Maximum number of string bytes.
 */
void Schema::add_string_field(
    const std::string& name,
    std::size_t length) {

    if (name.empty()) {
        throw std::invalid_argument(
            "Schema::add_string_field: name cannot be empty"
        );
    }

    if (length == 0) {
        throw std::invalid_argument(
            "Schema::add_string_field: length must be greater than zero"
        );
    }

    fields_.push_back({
        name,
        FieldType::STRING,
        length
    });
}

/**
 * fields returns all fields in the schema.
 *
 * @return List of fields.
 */
const std::vector<Field>& Schema::fields() const {
    return fields_;
}

/**
 * field_count returns the number of fields.
 *
 * @return Number of fields.
 */
std::size_t Schema::field_count() const {
    return fields_.size();
}

} // namespace flashdb