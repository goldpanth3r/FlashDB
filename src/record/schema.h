#ifndef FLASHDB_RECORD_SCHEMA_H
#define FLASHDB_RECORD_SCHEMA_H

#include <cstddef>
#include <string>
#include <vector>

namespace flashdb {

/**
 * FieldType describes the type of one table column.
 */
enum class FieldType {
    INT,
    STRING
};

/**
 * Field describes one column in a table.
 */
struct Field {
    std::string name;
    FieldType type;
    std::size_t length;
};

/**
 * Schema describes the columns of a table.
 *
 * Example:
 *
 * id   INT
 * name STRING(50)
 */
class Schema {
public:
    /**
     * Add an integer field.
     *
     * @param name Column name.
     */
    void add_int_field(const std::string& name);

    /**
     * Add a string field.
     *
     * @param name Column name.
     * @param length Maximum string length.
     */
    void add_string_field(
        const std::string& name,
        std::size_t length
    );

    /**
     * Return all fields in the schema.
     *
     * @return List of table fields.
     */
    const std::vector<Field>& fields() const;

    /**
     * Return the number of fields.
     *
     * @return Number of columns.
     */
    std::size_t field_count() const;

private:
    std::vector<Field> fields_;
};

} // namespace flashdb

#endif