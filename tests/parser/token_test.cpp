#include <gtest/gtest.h>

#include "parser/token.h"

namespace flashdb {

TEST(TokenTest, StoresKeyword) {
    Token token(TokenType::KEYWORD, "SELECT");

    EXPECT_EQ(token.type(), TokenType::KEYWORD);
    EXPECT_EQ(token.value(), "SELECT");
}

TEST(TokenTest, StoresIdentifier) {
    Token token(TokenType::IDENTIFIER, "student");

    EXPECT_EQ(token.type(), TokenType::IDENTIFIER);
    EXPECT_EQ(token.value(), "student");
}

TEST(TokenTest, StoresInteger) {
    Token token(TokenType::INTEGER, "123");

    EXPECT_EQ(token.type(), TokenType::INTEGER);
    EXPECT_EQ(token.value(), "123");
}

TEST(TokenTest, StoresString) {
    Token token(TokenType::STRING, "Alice");

    EXPECT_EQ(token.type(), TokenType::STRING);
    EXPECT_EQ(token.value(), "Alice");
}

TEST(TokenTest, StoresSymbol) {
    Token token(TokenType::SYMBOL, "(");

    EXPECT_EQ(token.type(), TokenType::SYMBOL);
    EXPECT_EQ(token.value(), "(");
}

} // namespace flashdb