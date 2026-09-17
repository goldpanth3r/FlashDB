#ifndef FLASHDB_RECORD_LAYOUT_H
#define FLASHDB_RECORD_LAYOUT_H

#include <cstddef>
#include <string>
#include <unordered_map>

#include "schema.h"

namespace flashdb {

/**
 * Layout describes how a record is stored inside a database page.
 *
 * Example:
 *
 * id   INT
 * name STRING(50)
 *
 * Layout:
 *
 * id   -> offset 0
 * name -> offset 4
 */
class Layout {
public:
    /**
     * Build a layout from a table schema.
     *
     * @param schema Schema describing the table fields.
     */
    explicit Layout(const Schema& schema);

    /**
     * Return the byte offset of a field.
     *
     * @param field_name Name of the field.
     * @return Byte offset of the field.
     */
    std::size_t offset(const std::string& field_name) const;

    /**
     * Return the storage size of a field.
     *
     * @param field_name Name of the field.
     * @return Number of bytes used by the field.
     */
    std::size_t field_size(const std::string& field_name) const;

    /**
     * Return the complete size of one record.
     *
     * @return Number of bytes required for one record.
     */
    std::size_t record_size() const;

    /**
     * Return the schema used by this layout.
     *
     * @return Table schema.
     */
    const Schema& schema() const;

private:
    const Schema& schema_;
    std::unordered_map<std::string, std::size_t> offsets_;
    std::unordered_map<std::string, std::size_t> field_sizes_;
    std::size_t record_size_;
};

} // namespace flashdb

#endif