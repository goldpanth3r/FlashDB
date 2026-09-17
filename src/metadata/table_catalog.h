#ifndef FLASHDB_METADATA_TABLE_CATALOG_H
#define FLASHDB_METADATA_TABLE_CATALOG_H

#include <string>
#include <unordered_map>

#include "record/schema.h"

namespace flashdb {

/**
 * TableCatalog stores the schema of each table.
 *
 * It provides a simple in-memory catalog for now.
 */
class TableCatalog {
public:
    /**
     * Add a table to the catalog.
     */
    void create_table(
        const std::string& table_name,
        const Schema& schema
    );

    /**
     * Check whether a table exists.
     */
    bool has_table(
        const std::string& table_name
    ) const;

    /**
     * Return the schema of a table.
     */
    const Schema& get_schema(
        const std::string& table_name
    ) const;

    /**
     * Return the number of tables in the catalog.
     */
    std::size_t table_count() const;

private:
    std::unordered_map<std::string, Schema> tables_;
};

} // namespace flashdb

#endif