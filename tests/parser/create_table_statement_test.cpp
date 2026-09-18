#include <gtest/gtest.h>

#include "parser/ast/create_table_statement.h"

using namespace flashdb;

TEST(CreateTableStatementTest, StoresTableName) {
    CreateTableStatement statement(
        "student",
        {}
    );

    EXPECT_EQ(
        statement.table_name(),
        "student"
    );
}

TEST(CreateTableStatementTest, StoresColumns) {
    std::vector<ColumnDefinition> columns = {
        {"id", "INT", 0},
        {"name", "VARCHAR", 100}
    };

    CreateTableStatement statement(
        "student",
        columns
    );

    ASSERT_EQ(
        statement.columns().size(),
        2
    );

    EXPECT_EQ(
        statement.columns()[0].name,
        "id"
    );

    EXPECT_EQ(
        statement.columns()[0].type,
        "INT"
    );

    EXPECT_EQ(
        statement.columns()[1].name,
        "name"
    );

    EXPECT_EQ(
        statement.columns()[1].type,
        "VARCHAR"
    );

    EXPECT_EQ(
        statement.columns()[1].length,
        100
    );
}