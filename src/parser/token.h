#ifndef FLASHDB_PARSER_TOKEN_H
#define FLASHDB_PARSER_TOKEN_H

#include <string>

namespace flashdb {

enum class TokenType {
    KEYWORD,
    IDENTIFIER,
    INTEGER,
    STRING,
    SYMBOL
};

class Token {
public:
    Token(TokenType type, std::string value);

    TokenType type() const;
    const std::string& value() const;

private:
    TokenType type_;
    std::string value_;
};

} // namespace flashdb

#endif