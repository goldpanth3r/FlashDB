#include "insert_statement.h"

#include <utility>

namespace flashdb {

InsertStatement::InsertStatement(
    std::string table_name,
    std::vector<Expression> values)
    : table_name_(std::move(table_name)),
      values_(std::move(values)) {
}

const std::string&
InsertStatement::table_name() const {
    return table_name_;
}

const std::vector<Expression>&
InsertStatement::values() const {
    return values_;
}

} // namespace flashdb