#include "layout.h"

#include <stdexcept>

namespace flashdb {

/**
 * Layout builds byte offsets for every field.
 *
 * Fields are stored sequentially:
 *
 * [field 1][field 2][field 3]...
 *
 * @param schema Schema describing the table.
 */
Layout::Layout(const Schema& schema)
    : schema_(schema),
      record_size_(0) {

    for (const Field& field : schema.fields()) {

        if (offsets_.contains(field.name)) {
            throw std::invalid_argument(
                "Layout: duplicate field name: " + field.name
            );
        }

        offsets_[field.name] = record_size_;

        std::size_t size = 0;

        if (field.type == FieldType::INT) {
            size = sizeof(int32_t);
        } else if (field.type == FieldType::STRING) {
            size = field.length;
        } else {
            throw std::runtime_error(
                "Layout: unsupported field type"
            );
        }

        field_sizes_[field.name] = size;

        record_size_ += size;
    }
}

/**
 * offset returns the byte position of a field.
 *
 * @param field_name Name of the field.
 * @return Byte offset of the field.
 */
std::size_t Layout::offset(
    const std::string& field_name) const {

    const auto it = offsets_.find(field_name);

    if (it == offsets_.end()) {
        throw std::out_of_range(
            "Layout::offset: unknown field: " + field_name
        );
    }

    return it->second;
}

/**
 * field_size returns the number of bytes allocated to a field.
 *
 * @param field_name Name of the field.
 * @return Field storage size.
 */
std::size_t Layout::field_size(
    const std::string& field_name) const {

    const auto it = field_sizes_.find(field_name);

    if (it == field_sizes_.end()) {
        throw std::out_of_range(
            "Layout::field_size: unknown field: " + field_name
        );
    }

    return it->second;
}

/**
 * record_size returns the total storage size of one record.
 *
 * @return Number of bytes in one record.
 */
std::size_t Layout::record_size() const {
    return record_size_;
}

/**
 * schema returns the original table schema.
 *
 * @return Schema used to create this layout.
 */
const Schema& Layout::schema() const {
    return schema_;
}

} // namespace flashdb