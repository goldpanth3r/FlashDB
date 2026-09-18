#include "select_statement.h"

#include <memory>
#include <stdexcept>
#include <utility>

namespace flashdb {

SelectStatement::SelectStatement(
    std::vector<Expression> columns,
    std::string table_name)
    : columns_(std::move(columns)),
      table_name_(std::move(table_name)),
      has_condition_(false),
      condition_(nullptr) {
}

SelectStatement::SelectStatement(
    std::vector<Expression> columns,
    std::string table_name,
    Condition condition)
    : columns_(std::move(columns)),
      table_name_(std::move(table_name)),
      has_condition_(true),
      condition_(
          std::make_unique<Condition>(
              std::move(condition)
          )
      ) {
}

const std::vector<Expression>&
SelectStatement::columns() const {
    return columns_;
}

const std::string&
SelectStatement::table_name() const {
    return table_name_;
}

bool SelectStatement::has_condition() const {
    return has_condition_;
}

const Condition&
SelectStatement::condition() const {
    if (!has_condition_ || condition_ == nullptr) {
        throw std::runtime_error(
            "SelectStatement::condition: no condition"
        );
    }

    return *condition_;
}

} // namespace flashdb