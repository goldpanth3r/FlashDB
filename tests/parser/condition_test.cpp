#include <gtest/gtest.h>

#include "parser/ast/condition.h"

namespace flashdb {

TEST(ConditionTest, StoresComparison) {
    Condition condition(
        Expression(ExpressionType::IDENTIFIER, "id"),
        "=",
        Expression(ExpressionType::INTEGER, "1")
    );

    EXPECT_EQ(
        condition.left().type(),
        ExpressionType::IDENTIFIER
    );

    EXPECT_EQ(
        condition.left().value(),
        "id"
    );

    EXPECT_EQ(
        condition.operator_(),
        "="
    );

    EXPECT_EQ(
        condition.right().type(),
        ExpressionType::INTEGER
    );

    EXPECT_EQ(
        condition.right().value(),
        "1"
    );
}

} // namespace flashdb