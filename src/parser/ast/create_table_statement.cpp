#include "create_table_statement.h"

#include <utility>

namespace flashdb {

CreateTableStatement::CreateTableStatement(
    std::string table_name,
    std::vector<ColumnDefinition> columns)
    : table_name_(std::move(table_name)),
      columns_(std::move(columns)) {
}

const std::string&
CreateTableStatement::table_name() const {
    return table_name_;
}

const std::vector<ColumnDefinition>&
CreateTableStatement::columns() const {
    return columns_;
}

} // namespace flashdb