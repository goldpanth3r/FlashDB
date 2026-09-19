#include "parser/ast/create_index_statement.h"

#include <utility>

namespace flashdb {

// Store the CREATE INDEX information for later planning.
CreateIndexStatement::CreateIndexStatement(
    std::string index_name,
    std::string table_name,
    std::string column_name)
    : index_name_(std::move(index_name)),
      table_name_(std::move(table_name)),
      column_name_(std::move(column_name)) {
}

// Return the name assigned to the index.
const std::string& CreateIndexStatement::index_name() const {
    return index_name_;
}

// Return the table on which the index is created.
const std::string& CreateIndexStatement::table_name() const {
    return table_name_;
}

// Return the column stored in the index.
const std::string& CreateIndexStatement::column_name() const {
    return column_name_;
}

} // namespace flashdb
