#ifndef FLASHDB_RECORD_SCHEMA_H
#define FLASHDB_RECORD_SCHEMA_H

#include <cstddef>
#include <string>
#include <vector>

namespace flashdb {

/**
 * Schema describes the fields stored in a database record.
 *
 * Example:
 *
 * id    INT
 * name  TEXT
 */
class Schema {
public:
    /**
     * Add an integer field.
     *
     * @param name Field name.
     */
    void add_int_field(const std::string& name);

    /**
     * Add a string field.
     *
     * @param name Field name.
     * @param length Maximum string length.
     */
    void add_string_field(
        const std::string& name,
        std::size_t length);

    /**
     * Return the number of fields.
     */
    std::size_t field_count() const;

    /**
     * Return a field name.
     *
     * @param index Field position.
     */
    const std::string& field_name(std::size_t index) const;

    /**
     * Return whether a field is an integer.
     *
     * @param index Field position.
     */
    bool is_int(std::size_t index) const;

    /**
     * Return the maximum string length.
     *
     * @param index Field position.
     */
    std::size_t string_length(std::size_t index) const;

private:
    enum class FieldType {
        INT,
        STRING
    };

    struct Field {
        std::string name;
        FieldType type;
        std::size_t length;
    };

    std::vector<Field> fields_;
};

} // namespace flashdb

#endif