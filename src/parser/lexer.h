#ifndef FLASHDB_PARSER_LEXER_H
#define FLASHDB_PARSER_LEXER_H

#include <string>
#include <vector>

#include "token.h"

namespace flashdb {

class Lexer {
public:
    explicit Lexer(const std::string& input);

    std::vector<Token> tokenize();

private:
    std::string input_;
    std::size_t position_;

    void skip_whitespace();

    Token read_word();
    Token read_number();
    Token read_string();
    Token read_symbol();

    bool is_keyword(const std::string& value) const;
};

} // namespace flashdb

#endif