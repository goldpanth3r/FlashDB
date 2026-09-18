#include <gtest/gtest.h>

#include "parser/ast/select_statement.h"

namespace flashdb {

TEST(SelectStatementTest, StoresColumnsAndTable) {
    std::vector<Expression> columns;

    columns.emplace_back(
        ExpressionType::IDENTIFIER,
        "name"
    );

    columns.emplace_back(
        ExpressionType::IDENTIFIER,
        "age"
    );

    SelectStatement statement(
        std::move(columns),
        "student"
    );

    ASSERT_EQ(statement.columns().size(), 2);

    EXPECT_EQ(
        statement.columns()[0].value(),
        "name"
    );

    EXPECT_EQ(
        statement.columns()[1].value(),
        "age"
    );

    EXPECT_EQ(
        statement.table_name(),
        "student"
    );

    EXPECT_FALSE(statement.has_condition());
}

TEST(SelectStatementTest, StoresWhereCondition) {
    std::vector<Expression> columns;

    columns.emplace_back(
        ExpressionType::IDENTIFIER,
        "name"
    );

    Condition condition(
        Expression(
            ExpressionType::IDENTIFIER,
            "id"
        ),
        "=",
        Expression(
            ExpressionType::INTEGER,
            "1"
        )
    );

    SelectStatement statement(
        std::move(columns),
        "student",
        std::move(condition)
    );

    EXPECT_TRUE(statement.has_condition());

    EXPECT_EQ(
        statement.condition().left().value(),
        "id"
    );

    EXPECT_EQ(
        statement.condition().operator_(),
        "="
    );

    EXPECT_EQ(
        statement.condition().right().value(),
        "1"
    );
}

TEST(SelectStatementTest, ThrowsWhenConditionDoesNotExist) {
    SelectStatement statement(
        std::vector<Expression>{
            Expression(
                ExpressionType::IDENTIFIER,
                "name"
            )
        },
        "student"
    );

    EXPECT_THROW(
        statement.condition(),
        std::runtime_error
    );
}

} // namespace flashdb