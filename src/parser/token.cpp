#include "token.h"

#include <utility>

namespace flashdb {

Token::Token(TokenType type, std::string value)
    : type_(type),
      value_(std::move(value)) {
}

TokenType Token::type() const {
    return type_;
}

const std::string& Token::value() const {
    return value_;
}

} // namespace flashdb