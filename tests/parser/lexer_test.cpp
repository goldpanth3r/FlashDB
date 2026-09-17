#include <gtest/gtest.h>

#include "parser/lexer.h"

namespace flashdb {

TEST(LexerTest, TokenizesCreateTable) {
    Lexer lexer(
        "CREATE TABLE student (id INT);"
    );

    const auto tokens = lexer.tokenize();

    ASSERT_EQ(tokens.size(), 8);

    EXPECT_EQ(tokens[0].type(), TokenType::KEYWORD);
    EXPECT_EQ(tokens[0].value(), "CREATE");

    EXPECT_EQ(tokens[1].type(), TokenType::KEYWORD);
    EXPECT_EQ(tokens[1].value(), "TABLE");

    EXPECT_EQ(tokens[2].type(), TokenType::IDENTIFIER);
    EXPECT_EQ(tokens[2].value(), "student");

    EXPECT_EQ(tokens[3].value(), "(");
    EXPECT_EQ(tokens[4].value(), "id");

    EXPECT_EQ(tokens[5].value(), "INT");
    EXPECT_EQ(tokens[6].value(), ")");
    EXPECT_EQ(tokens[7].value(), ";");
}

TEST(LexerTest, TokenizesInsert) {
    Lexer lexer(
        "INSERT INTO student VALUES (1, 'Alice');"
    );

    const auto tokens = lexer.tokenize();

    ASSERT_EQ(tokens.size(), 10);

    EXPECT_EQ(tokens[0].value(), "INSERT");
    EXPECT_EQ(tokens[1].value(), "INTO");
    EXPECT_EQ(tokens[2].value(), "student");
    EXPECT_EQ(tokens[3].value(), "VALUES");
    EXPECT_EQ(tokens[4].value(), "(");
    EXPECT_EQ(tokens[5].value(), "1");
    EXPECT_EQ(tokens[6].value(), ",");
    EXPECT_EQ(tokens[7].value(), "Alice");
    EXPECT_EQ(tokens[8].value(), ")");
    EXPECT_EQ(tokens[9].value(), ";");
}

TEST(LexerTest, IgnoresWhitespace) {
    Lexer lexer(
        "  SELECT   name   FROM   student ; "
    );

    const auto tokens = lexer.tokenize();

    ASSERT_EQ(tokens.size(), 5);

    EXPECT_EQ(tokens[0].value(), "SELECT");
    EXPECT_EQ(tokens[1].value(), "name");
    EXPECT_EQ(tokens[2].value(), "FROM");
    EXPECT_EQ(tokens[3].value(), "student");
    EXPECT_EQ(tokens[4].value(), ";");
}

TEST(LexerTest, RejectsUnexpectedCharacter) {
    Lexer lexer(
        "SELECT @ FROM student;"
    );

    EXPECT_THROW(
        lexer.tokenize(),
        std::invalid_argument
    );
}

TEST(LexerTest, RejectsUnterminatedString) {
    Lexer lexer(
        "INSERT INTO student VALUES ('Alice);"
    );

    EXPECT_THROW(
        lexer.tokenize(),
        std::invalid_argument
    );
}

} // namespace flashdb