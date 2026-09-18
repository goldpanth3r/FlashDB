#include <gtest/gtest.h>

#include "parser/ast/update_statement.h"

using namespace flashdb;

TEST(UpdateStatementTest, StoresUpdateInformation) {
    UpdateStatement statement(
        "student",
        "name",
        Expression(
            ExpressionType::STRING,
            "Alice"
        )
    );

    EXPECT_EQ(
        statement.table_name(),
        "student"
    );

    EXPECT_EQ(
        statement.column_name(),
        "name"
    );

    EXPECT_EQ(
        statement.value().type(),
        ExpressionType::STRING
    );

    EXPECT_EQ(
        statement.value().value(),
        "Alice"
    );
}

TEST(UpdateStatementTest, StoresIntegerValue) {
    UpdateStatement statement(
        "student",
        "age",
        Expression(
            ExpressionType::INTEGER,
            "20"
        )
    );

    EXPECT_EQ(
        statement.table_name(),
        "student"
    );

    EXPECT_EQ(
        statement.column_name(),
        "age"
    );

    EXPECT_EQ(
        statement.value().type(),
        ExpressionType::INTEGER
    );

    EXPECT_EQ(
        statement.value().value(),
        "20"
    );
}