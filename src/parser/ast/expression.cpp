#include "expression.h"

#include <utility>

namespace flashdb {

Expression::Expression(
    ExpressionType type,
    std::string value)
    : type_(type),
      value_(std::move(value)) {
}

ExpressionType Expression::type() const {
    return type_;
}

const std::string& Expression::value() const {
    return value_;
}

} // namespace flashdb