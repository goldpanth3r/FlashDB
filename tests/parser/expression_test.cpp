#include <gtest/gtest.h>

#include "parser/ast/expression.h"

namespace flashdb {

TEST(ExpressionTest, StoresIdentifier) {
    Expression expression(
        ExpressionType::IDENTIFIER,
        "student"
    );

    EXPECT_EQ(
        expression.type(),
        ExpressionType::IDENTIFIER
    );

    EXPECT_EQ(expression.value(), "student");
}

TEST(ExpressionTest, StoresInteger) {
    Expression expression(
        ExpressionType::INTEGER,
        "123"
    );

    EXPECT_EQ(
        expression.type(),
        ExpressionType::INTEGER
    );

    EXPECT_EQ(expression.value(), "123");
}

TEST(ExpressionTest, StoresString) {
    Expression expression(
        ExpressionType::STRING,
        "Alice"
    );

    EXPECT_EQ(
        expression.type(),
        ExpressionType::STRING
    );

    EXPECT_EQ(expression.value(), "Alice");
}

} // namespace flashdb