#include <gtest/gtest.h>

#include "parser/ast/insert_statement.h"

namespace flashdb {

TEST(InsertStatementTest, StoresTableAndValues) {
    std::vector<Expression> values;

    values.emplace_back(
        ExpressionType::INTEGER,
        "1"
    );

    values.emplace_back(
        ExpressionType::STRING,
        "Alice"
    );

    InsertStatement statement(
        "student",
        std::move(values)
    );

    EXPECT_EQ(
        statement.table_name(),
        "student"
    );

    ASSERT_EQ(
        statement.values().size(),
        2
    );

    EXPECT_EQ(
        statement.values()[0].type(),
        ExpressionType::INTEGER
    );

    EXPECT_EQ(
        statement.values()[0].value(),
        "1"
    );

    EXPECT_EQ(
        statement.values()[1].type(),
        ExpressionType::STRING
    );

    EXPECT_EQ(
        statement.values()[1].value(),
        "Alice"
    );
}

} // namespace flashdb