#include "table_catalog.h"

#include <stdexcept>

namespace flashdb {

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

bool TableCatalog::has_table(
    const std::string& table_name) const {

    return tables_.contains(table_name);
}

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

std::size_t TableCatalog::table_count() const {
    return tables_.size();
}

} // namespace flashdb