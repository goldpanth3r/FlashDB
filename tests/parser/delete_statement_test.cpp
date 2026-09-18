#include <gtest/gtest.h>

#include "parser/ast/delete_statement.h"

using namespace flashdb;

TEST(DeleteStatementTest, StoresTableName) {
    DeleteStatement statement(
        "student"
    );

    EXPECT_EQ(
        statement.table_name(),
        "student"
    );
}