#include "table_catalog.h"

#include <cstdint>
#include <fstream>
#include <stdexcept>
#include <utility>

namespace flashdb {

namespace {

constexpr std::uint32_t kCatalogVersion = 1;
constexpr std::uint32_t kIntField = 0;
constexpr std::uint32_t kStringField = 1;

// Store a fixed-width integer in the catalog file.
void write_uint32(
    std::ofstream& output,
    std::uint32_t value) {

    output.write(
        reinterpret_cast<const char*>(&value),
        sizeof(value)
    );

    if (!output) {
        throw std::runtime_error(
            "TableCatalog: failed to write catalog"
        );
    }
}

// Read a fixed-width integer from the catalog file.
std::uint32_t read_uint32(
    std::ifstream& input) {

    std::uint32_t value = 0;

    input.read(
        reinterpret_cast<char*>(&value),
        sizeof(value)
    );

    if (!input) {
        throw std::runtime_error(
            "TableCatalog: invalid catalog file"
        );
    }

    return value;
}

// Store a length-prefixed string in the catalog file.
void write_string(
    std::ofstream& output,
    const std::string& value) {

    write_uint32(
        output,
        static_cast<std::uint32_t>(value.size())
    );

    output.write(
        value.data(),
        static_cast<std::streamsize>(value.size())
    );

    if (!output) {
        throw std::runtime_error(
            "TableCatalog: failed to write string"
        );
    }
}

// Read a length-prefixed string from the catalog file.
std::string read_string(
    std::ifstream& input) {

    const std::uint32_t length =
        read_uint32(input);

    std::string value(length, '\0');

    input.read(
        value.data(),
        static_cast<std::streamsize>(length)
    );

    if (!input) {
        throw std::runtime_error(
            "TableCatalog: invalid string in catalog"
        );
    }

    return value;
}

} // namespace

// Keep the catalog file location with the metadata manager.
TableCatalog::TableCatalog(
    const std::string& catalog_filename)
    : catalog_filename_(catalog_filename) {
}

// Register a table schema in memory.
void TableCatalog::create_table(
    const std::string& table_name,
    const Schema& schema) {

    if (table_name.empty()) {
        throw std::invalid_argument(
            "TableCatalog::create_table: table name cannot be empty"
        );
    }

    if (tables_.contains(table_name)) {
        throw std::invalid_argument(
            "TableCatalog::create_table: table already exists"
        );
    }

    tables_.emplace(table_name, schema);
}

// Check whether a table exists in the catalog.
bool TableCatalog::has_table(
    const std::string& table_name) const {

    return tables_.contains(table_name);
}

// Return the schema registered for a table.
const Schema& TableCatalog::get_schema(
    const std::string& table_name) const {

    const auto it = tables_.find(table_name);

    if (it == tables_.end()) {
        throw std::out_of_range(
            "TableCatalog::get_schema: table does not exist"
        );
    }

    return it->second;
}

// Count the tables currently stored in the catalog.
std::size_t TableCatalog::table_count() const {
    return tables_.size();
}

// Restore table metadata from disk.
void TableCatalog::load() {

    tables_.clear();

    std::ifstream input(
        catalog_filename_,
        std::ios::binary
    );

    // A missing catalog means this is a new database.
    if (!input.is_open()) {
        return;
    }

    const std::uint32_t version =
        read_uint32(input);

    if (version != kCatalogVersion) {
        throw std::runtime_error(
            "TableCatalog: unsupported catalog version"
        );
    }

    const std::uint32_t table_count =
        read_uint32(input);

    for (std::uint32_t table_index = 0;
         table_index < table_count;
         ++table_index) {

        const std::string table_name =
            read_string(input);

        Schema schema;

        const std::uint32_t field_count =
            read_uint32(input);

        for (std::uint32_t field_index = 0;
             field_index < field_count;
             ++field_index) {

            const std::string field_name =
                read_string(input);

            const std::uint32_t field_type =
                read_uint32(input);

            const std::uint32_t field_length =
                read_uint32(input);

            if (field_type == kIntField) {
                schema.add_int_field(field_name);
                continue;
            }

            if (field_type == kStringField) {
                schema.add_string_field(
                    field_name,
                    static_cast<std::size_t>(field_length)
                );
                continue;
            }

            throw std::runtime_error(
                "TableCatalog: unknown field type"
            );
        }

        if (tables_.contains(table_name)) {
            throw std::runtime_error(
                "TableCatalog: duplicate table in catalog"
            );
        }

        tables_.emplace(
            table_name,
            std::move(schema)
        );
    }
}

// Persist all table metadata to disk.
void TableCatalog::save() const {

    std::ofstream output(
        catalog_filename_,
        std::ios::binary |
        std::ios::trunc
    );

    if (!output.is_open()) {
        throw std::runtime_error(
            "TableCatalog: failed to open catalog for writing"
        );
    }

    write_uint32(
        output,
        kCatalogVersion
    );

    write_uint32(
        output,
        static_cast<std::uint32_t>(tables_.size())
    );

    for (const auto& [table_name, schema] :
         tables_) {

        write_string(
            output,
            table_name
        );

        const auto& fields =
            schema.fields();

        write_uint32(
            output,
            static_cast<std::uint32_t>(fields.size())
        );

        for (const Field& field : fields) {

            write_string(
                output,
                field.name
            );

            if (field.type == FieldType::INT) {

                write_uint32(
                    output,
                    kIntField
                );

                write_uint32(
                    output,
                    0
                );

                continue;
            }

            if (field.type == FieldType::STRING) {

                write_uint32(
                    output,
                    kStringField
                );

                write_uint32(
                    output,
                    static_cast<std::uint32_t>(
                        field.length
                    )
                );

                continue;
            }

            throw std::runtime_error(
                "TableCatalog: unknown field type"
            );
        }
    }

    output.flush();

    if (!output) {
        throw std::runtime_error(
            "TableCatalog: failed to flush catalog"
        );
    }
}

} // namespace flashdb