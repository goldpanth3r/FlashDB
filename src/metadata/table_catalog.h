#ifndef FLASHDB_METADATA_TABLE_CATALOG_H
#define FLASHDB_METADATA_TABLE_CATALOG_H

#include <string>
#include <unordered_map>

#include "record/schema.h"

namespace flashdb {

class TableCatalog {
public:
    explicit TableCatalog(
        const std::string& catalog_filename = "flashdb.catalog"
    );

    void create_table(
        const std::string& table_name,
        const Schema& schema
    );

    bool has_table(
        const std::string& table_name
    ) const;

    const Schema& get_schema(
        const std::string& table_name
    ) const;

    std::size_t table_count() const;

    void load();
    void save() const;

private:
    std::string catalog_filename_;
    std::unordered_map<std::string, Schema> tables_;
};

} // namespace flashdb

#endif