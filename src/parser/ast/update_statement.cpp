#include "update_statement.h"

#include <stdexcept>
#include <utility>

namespace flashdb {

UpdateStatement::UpdateStatement(
    std::string table_name,
    std::string column_name,
    Expression value)
    : table_name_(std::move(table_name)),
      column_name_(std::move(column_name)),
      value_(std::move(value)),
      has_condition_(false),
      condition_(nullptr) {
}

UpdateStatement::UpdateStatement(
    std::string table_name,
    std::string column_name,
    Expression value,
    Condition condition)
    : table_name_(std::move(table_name)),
      column_name_(std::move(column_name)),
      value_(std::move(value)),
      has_condition_(true),
      condition_(
          std::make_unique<Condition>(std::move(condition))) {
}

const std::string&
UpdateStatement::table_name() const {
    return table_name_;
}

const std::string&
UpdateStatement::column_name() const {
    return column_name_;
}

const Expression&
UpdateStatement::value() const {
    return value_;
}

bool UpdateStatement::has_condition() const {
    return has_condition_;
}

const Condition&
UpdateStatement::condition() const {
    if (!has_condition_) {
        throw std::runtime_error(
            "UpdateStatement::condition: no condition"
        );
    }

    return *condition_;
}

} // namespace flashdb