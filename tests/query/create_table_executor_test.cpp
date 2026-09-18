#include <stdexcept>

#include <gtest/gtest.h>

#include "metadata/table_catalog.h"
#include "parser/lexer.h"
#include "parser/parser.h"
#include "planner/planner.h"
#include "query/create_table_executor.h"

namespace flashdb {

TEST(CreateTableExecutorTest, CreatesTable) {
    Lexer lexer(
        "CREATE TABLE student ("
        "id INT, "
        "name VARCHAR(50)"
        ");"
    );

    const auto tokens = lexer.tokenize();
    Parser parser(tokens);

    const auto statement = parser.parse();

    Planner planner;

    const auto plan = planner.create_plan(statement);

    TableCatalog catalog;

    CreateTableExecutor executor(
        *plan,
        catalog
    );

    executor.execute();

    ASSERT_TRUE(
        catalog.has_table("student")
    );

    EXPECT_EQ(
        catalog.table_count(),
        1u
    );

    const Schema& schema =
        catalog.get_schema("student");

    ASSERT_EQ(
        schema.field_count(),
        2u
    );

    EXPECT_EQ(
        schema.fields()[0].name,
        "id"
    );

    EXPECT_EQ(
        schema.fields()[0].type,
        FieldType::INT
    );

    EXPECT_EQ(
        schema.fields()[1].name,
        "name"
    );

    EXPECT_EQ(
        schema.fields()[1].type,
        FieldType::STRING
    );

    EXPECT_EQ(
        schema.fields()[1].length,
        50u
    );
}

TEST(CreateTableExecutorTest, RejectsDuplicateTable) {
    Lexer lexer(
        "CREATE TABLE student ("
        "id INT"
        ");"
    );

    const auto tokens = lexer.tokenize();
    Parser parser(tokens);

    const auto statement = parser.parse();

    Planner planner;

    const auto plan = planner.create_plan(statement);

    TableCatalog catalog;

    CreateTableExecutor executor(
        *plan,
        catalog
    );

    executor.execute();

    EXPECT_THROW(
        executor.execute(),
        std::invalid_argument
    );
}

}