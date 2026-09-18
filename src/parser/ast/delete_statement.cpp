#include "delete_statement.h"

#include <stdexcept>
#include <utility>

namespace flashdb {

DeleteStatement::DeleteStatement(
    std::string table_name)
    : table_name_(std::move(table_name)),
      has_condition_(false),
      condition_(nullptr) {
}

DeleteStatement::DeleteStatement(
    std::string table_name,
    Condition condition)
    : table_name_(std::move(table_name)),
      has_condition_(true),
      condition_(
          std::make_unique<Condition>(std::move(condition))) {
}

const std::string&
DeleteStatement::table_name() const {
    return table_name_;
}

bool DeleteStatement::has_condition() const {
    return has_condition_;
}

const Condition&
DeleteStatement::condition() const {
    if (!has_condition_) {
        throw std::runtime_error(
            "DeleteStatement::condition: no condition"
        );
    }

    return *condition_;
}

} // namespace flashdb