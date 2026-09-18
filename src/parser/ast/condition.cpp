#include "condition.h"

#include <utility>

namespace flashdb {

Condition::Condition(
    Expression left,
    std::string operator_,
    Expression right)
    : left_(std::move(left)),
      operator_value_(std::move(operator_)),
      right_(std::move(right)) {
}

const Expression& Condition::left() const {
    return left_;
}

const std::string& Condition::operator_() const {
    return operator_value_;
}

const Expression& Condition::right() const {
    return right_;
}

} // namespace flashdb