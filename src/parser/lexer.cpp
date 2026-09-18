#include "lexer.h"

#include <cctype>
#include <stdexcept>
#include <unordered_set>

namespace flashdb {

Lexer::Lexer(const std::string& input)
    : input_(input),
      position_(0) {
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (position_ < input_.size()) {
        skip_whitespace();

        if (position_ >= input_.size()) {
            break;
        }

        const char current = input_[position_];

        if (std::isalpha(static_cast<unsigned char>(current)) ||
            current == '_') {
            tokens.push_back(read_word());
        }
        else if (std::isdigit(static_cast<unsigned char>(current))) {
            tokens.push_back(read_number());
        }
        else if (current == '\'') {
            tokens.push_back(read_string());
        }
        else if (current == '(' ||
                 current == ')' ||
                 current == ',' ||
                 current == ';' ||
                 current == '=') {
            tokens.push_back(read_symbol());
        }
        else {
            throw std::invalid_argument(
                "Lexer::tokenize: unexpected character"
            );
        }
    }

    return tokens;
}

void Lexer::skip_whitespace() {
    while (position_ < input_.size() &&
           std::isspace(
               static_cast<unsigned char>(input_[position_]))) {
        ++position_;
    }
}

Token Lexer::read_word() {
    const std::size_t start = position_;

    while (position_ < input_.size()) {
        const char current = input_[position_];

        if (!std::isalnum(static_cast<unsigned char>(current)) &&
            current != '_') {
            break;
        }

        ++position_;
    }

    const std::string value =
        input_.substr(start, position_ - start);

    if (is_keyword(value)) {
        return Token(TokenType::KEYWORD, value);
    }

    return Token(TokenType::IDENTIFIER, value);
}

Token Lexer::read_number() {
    const std::size_t start = position_;

    while (position_ < input_.size() &&
           std::isdigit(
               static_cast<unsigned char>(input_[position_]))) {
        ++position_;
    }

    return Token(
        TokenType::INTEGER,
        input_.substr(start, position_ - start)
    );
}

Token Lexer::read_string() {
    ++position_;

    const std::size_t start = position_;

    while (position_ < input_.size() &&
           input_[position_] != '\'') {
        ++position_;
    }

    if (position_ >= input_.size()) {
        throw std::invalid_argument(
            "Lexer::read_string: unterminated string"
        );
    }

    const std::string value =
        input_.substr(start, position_ - start);

    ++position_;

    return Token(TokenType::STRING, value);
}

Token Lexer::read_symbol() {
    const char symbol = input_[position_++];

    return Token(
        TokenType::SYMBOL,
        std::string(1, symbol)
    );
}

bool Lexer::is_keyword(const std::string& value) const {
    static const std::unordered_set<std::string> keywords = {
        "CREATE",
        "TABLE",
        "SELECT",
        "FROM",
        "WHERE",
        "INSERT",
        "UPDATE",
        "DELETE",
        "INTO",
        "VALUES",
        "SET",
        "INT",
        "VARCHAR"
    };

    return keywords.contains(value);
}

} // namespace flashdb