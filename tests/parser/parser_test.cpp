#include <gtest/gtest.h>

#include <stdexcept>
#include <variant>

#include "parser/lexer.h"
#include "parser/parser.h"

namespace flashdb {

TEST(ParserTest, ParsesSimpleSelect) {
    Lexer lexer(
        "SELECT name FROM student;"
    );

    const auto tokens = lexer.tokenize();

    Parser parser(tokens);

    SelectStatement statement =
        parser.parse_select();

    ASSERT_EQ(statement.columns().size(), 1);

    EXPECT_EQ(
        statement.columns()[0].type(),
        ExpressionType::IDENTIFIER
    );

    EXPECT_EQ(
        statement.columns()[0].value(),
        "name"
    );

    EXPECT_EQ(
        statement.table_name(),
        "student"
    );

    EXPECT_FALSE(
        statement.has_condition()
    );
}

TEST(ParserTest, ParsesMultipleColumns) {
    Lexer lexer(
        "SELECT name, age FROM student;"
    );

    const auto tokens = lexer.tokenize();

    Parser parser(tokens);

    SelectStatement statement =
        parser.parse_select();

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
}

TEST(ParserTest, RejectsMissingFrom) {
    Lexer lexer(
        "SELECT name student;"
    );

    const auto tokens = lexer.tokenize();

    Parser parser(tokens);

    EXPECT_THROW(
        parser.parse_select(),
        std::invalid_argument
    );
}

TEST(ParserTest, RejectsMissingSemicolon) {
    Lexer lexer(
        "SELECT name FROM student"
    );

    const auto tokens = lexer.tokenize();

    Parser parser(tokens);

    EXPECT_THROW(
        parser.parse_select(),
        std::invalid_argument
    );
}

TEST(ParserTest, RejectsInvalidColumn) {
    Lexer lexer(
        "SELECT 123 FROM student;"
    );

    const auto tokens = lexer.tokenize();

    Parser parser(tokens);

    EXPECT_THROW(
        parser.parse_select(),
        std::invalid_argument
    );
}

TEST(ParserTest, ParsesWhereCondition) {
    Lexer lexer(
        "SELECT name FROM student WHERE id = 1;"
    );

    const auto tokens = lexer.tokenize();

    Parser parser(tokens);

    SelectStatement statement =
        parser.parse_select();

    EXPECT_EQ(statement.table_name(), "student");
    EXPECT_TRUE(statement.has_condition());

    EXPECT_EQ(
        statement.condition().left().type(),
        ExpressionType::IDENTIFIER
    );

    EXPECT_EQ(
        statement.condition().left().value(),
        "id"
    );

    EXPECT_EQ(
        statement.condition().operator_(),
        "="
    );

    EXPECT_EQ(
        statement.condition().right().type(),
        ExpressionType::INTEGER
    );

    EXPECT_EQ(
        statement.condition().right().value(),
        "1"
    );
}

TEST(ParserTest, ParsesStringWhereCondition) {
    Lexer lexer(
        "SELECT name FROM student WHERE name = 'Alice';"
    );

    const auto tokens = lexer.tokenize();

    Parser parser(tokens);

    SelectStatement statement =
        parser.parse_select();

    EXPECT_TRUE(statement.has_condition());

    EXPECT_EQ(
        statement.condition().left().value(),
        "name"
    );

    EXPECT_EQ(
        statement.condition().operator_(),
        "="
    );

    EXPECT_EQ(
        statement.condition().right().type(),
        ExpressionType::STRING
    );

    EXPECT_EQ(
        statement.condition().right().value(),
        "Alice"
    );
}

TEST(ParserTest, RejectsInvalidWhereOperator) {
    Lexer lexer(
        "SELECT name FROM student WHERE id , 1;"
    );

    const auto tokens = lexer.tokenize();

    Parser parser(tokens);

    EXPECT_THROW(
        parser.parse_select(),
        std::invalid_argument
    );
}

TEST(ParserTest, ParsesInsert) {
    Lexer lexer(
        "INSERT INTO student VALUES (1, 'Alice');"
    );

    const auto tokens = lexer.tokenize();

    Parser parser(tokens);

    InsertStatement statement =
        parser.parse_insert();

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

TEST(ParserTest, ParsesInsertWithMultipleValues) {
    Lexer lexer(
        "INSERT INTO student VALUES (1, 'Alice', 20);"
    );

    const auto tokens = lexer.tokenize();

    Parser parser(tokens);

    InsertStatement statement =
        parser.parse_insert();

    ASSERT_EQ(
        statement.values().size(),
        3
    );

    EXPECT_EQ(
        statement.values()[0].value(),
        "1"
    );

    EXPECT_EQ(
        statement.values()[1].value(),
        "Alice"
    );

    EXPECT_EQ(
        statement.values()[2].value(),
        "20"
    );
}

TEST(ParserTest, RejectsInsertWithoutInto) {
    Lexer lexer(
        "INSERT student VALUES (1);"
    );

    const auto tokens = lexer.tokenize();

    Parser parser(tokens);

    EXPECT_THROW(
        parser.parse_insert(),
        std::invalid_argument
    );
}

TEST(ParserTest, RejectsInsertWithoutValues) {
    Lexer lexer(
        "INSERT INTO student (1);"
    );

    const auto tokens = lexer.tokenize();

    Parser parser(tokens);

    EXPECT_THROW(
        parser.parse_insert(),
        std::invalid_argument
    );
}

TEST(ParserTest, RejectsInsertWithoutClosingParenthesis) {
    Lexer lexer(
        "INSERT INTO student VALUES (1, 'Alice';"
    );

    const auto tokens = lexer.tokenize();

    Parser parser(tokens);

    EXPECT_THROW(
        parser.parse_insert(),
        std::invalid_argument
    );
}

TEST(ParserTest, DispatcherParsesSelect) {
    Lexer lexer(
        "SELECT name FROM student;"
    );

    const auto tokens = lexer.tokenize();

    Parser parser(tokens);

    auto statement = parser.parse();

    ASSERT_TRUE(
        std::holds_alternative<SelectStatement>(statement)
    );

    const auto& select =
        std::get<SelectStatement>(statement);

    EXPECT_EQ(
        select.table_name(),
        "student"
    );

    ASSERT_EQ(
        select.columns().size(),
        1
    );

    EXPECT_EQ(
        select.columns()[0].value(),
        "name"
    );
}

TEST(ParserTest, DispatcherParsesInsert) {
    Lexer lexer(
        "INSERT INTO student VALUES (1, 'Alice');"
    );

    const auto tokens = lexer.tokenize();

    Parser parser(tokens);

    auto statement = parser.parse();

    ASSERT_TRUE(
        std::holds_alternative<InsertStatement>(statement)
    );

    const auto& insert =
        std::get<InsertStatement>(statement);

    EXPECT_EQ(
        insert.table_name(),
        "student"
    );

    ASSERT_EQ(
        insert.values().size(),
        2
    );

    EXPECT_EQ(
        insert.values()[0].value(),
        "1"
    );

    EXPECT_EQ(
        insert.values()[1].value(),
        "Alice"
    );
}

TEST(ParserTest, DispatcherRejectsUnsupportedStatement) {
    Lexer lexer(
        "DROP TABLE student;"
    );

    const auto tokens = lexer.tokenize();

    Parser parser(tokens);

    EXPECT_THROW(
        parser.parse(),
        std::invalid_argument
    );
}

TEST(ParserTest, ParsesCreateTable) {
    Lexer lexer(
        "CREATE TABLE student(id INT, name VARCHAR(100));"
    );

    const auto tokens = lexer.tokenize();

    Parser parser(tokens);

    CreateTableStatement statement =
        parser.parse_create_table();

    EXPECT_EQ(
        statement.table_name(),
        "student"
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
        statement.columns()[0].length,
        0
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

TEST(ParserTest, ParsesCreateTableWithMultipleColumns) {
    Lexer lexer(
        "CREATE TABLE users("
        "id INT,"
        "name VARCHAR(50),"
        "email VARCHAR(100)"
        ");"
    );

    const auto tokens = lexer.tokenize();

    Parser parser(tokens);

    CreateTableStatement statement =
        parser.parse_create_table();

    ASSERT_EQ(
        statement.columns().size(),
        3
    );

    EXPECT_EQ(statement.columns()[0].name, "id");
    EXPECT_EQ(statement.columns()[0].type, "INT");

    EXPECT_EQ(statement.columns()[1].name, "name");
    EXPECT_EQ(statement.columns()[1].type, "VARCHAR");
    EXPECT_EQ(statement.columns()[1].length, 50);

    EXPECT_EQ(statement.columns()[2].name, "email");
    EXPECT_EQ(statement.columns()[2].type, "VARCHAR");
    EXPECT_EQ(statement.columns()[2].length, 100);
}

TEST(ParserTest, RejectsCreateTableWithoutTableName) {
    Lexer lexer(
        "CREATE TABLE (id INT);"
    );

    const auto tokens = lexer.tokenize();

    Parser parser(tokens);

    EXPECT_THROW(
        parser.parse_create_table(),
        std::invalid_argument
    );
}

TEST(ParserTest, RejectsCreateTableWithoutColumns) {
    Lexer lexer(
        "CREATE TABLE student();"
    );

    const auto tokens = lexer.tokenize();

    Parser parser(tokens);

    EXPECT_THROW(
        parser.parse_create_table(),
        std::invalid_argument
    );
}

TEST(ParserTest, RejectsCreateTableWithInvalidColumnType) {
    Lexer lexer(
        "CREATE TABLE student(id TEXT);"
    );

    const auto tokens = lexer.tokenize();

    Parser parser(tokens);

    EXPECT_THROW(
        parser.parse_create_table(),
        std::invalid_argument
    );
}

TEST(ParserTest, RejectsCreateTableWithInvalidVarcharLength) {
    Lexer lexer(
        "CREATE TABLE student(name VARCHAR(0));"
    );

    const auto tokens = lexer.tokenize();

    Parser parser(tokens);

    EXPECT_THROW(
        parser.parse_create_table(),
        std::invalid_argument
    );
}

TEST(ParserTest, DispatcherParsesCreateTable) {
    Lexer lexer(
        "CREATE TABLE student(id INT, name VARCHAR(100));"
    );

    const auto tokens = lexer.tokenize();

    Parser parser(tokens);

    auto statement = parser.parse();

    ASSERT_TRUE(
        std::holds_alternative<CreateTableStatement>(statement)
    );

    const auto& create_table =
        std::get<CreateTableStatement>(statement);

    EXPECT_EQ(
        create_table.table_name(),
        "student"
    );

    ASSERT_EQ(
        create_table.columns().size(),
        2
    );

    EXPECT_EQ(
        create_table.columns()[0].name,
        "id"
    );

    EXPECT_EQ(
        create_table.columns()[0].type,
        "INT"
    );

    EXPECT_EQ(
        create_table.columns()[1].name,
        "name"
    );

    EXPECT_EQ(
        create_table.columns()[1].type,
        "VARCHAR"
    );

    EXPECT_EQ(
        create_table.columns()[1].length,
        100
    );
}

TEST(ParserTest, RejectsSelectWithTrailingTokens) {
    Lexer lexer(
        "SELECT name FROM student; garbage"
    );

    const auto tokens = lexer.tokenize();

    Parser parser(tokens);

    EXPECT_THROW(
        parser.parse_select(),
        std::invalid_argument
    );
}

TEST(ParserTest, RejectsInsertWithTrailingTokens) {
    Lexer lexer(
        "INSERT INTO student VALUES (1, 'Alice'); garbage"
    );

    const auto tokens = lexer.tokenize();

    Parser parser(tokens);

    EXPECT_THROW(
        parser.parse_insert(),
        std::invalid_argument
    );
}

TEST(ParserTest, RejectsCreateTableWithTrailingTokens) {
    Lexer lexer(
        "CREATE TABLE student(id INT); garbage"
    );

    const auto tokens = lexer.tokenize();

    Parser parser(tokens);

    EXPECT_THROW(
        parser.parse_create_table(),
        std::invalid_argument
    );
}

TEST(ParserTest, ParsesUpdate) {
    Lexer lexer(
        "UPDATE student SET name = 'Alice';"
    );

    const auto tokens = lexer.tokenize();

    Parser parser(tokens);

    UpdateStatement statement =
        parser.parse_update();

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

TEST(ParserTest, ParsesUpdateWithInteger) {
    Lexer lexer(
        "UPDATE student SET age = 20;"
    );

    const auto tokens = lexer.tokenize();

    Parser parser(tokens);

    UpdateStatement statement =
        parser.parse_update();

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

TEST(ParserTest, RejectsUpdateWithoutTableName) {
    Lexer lexer(
        "UPDATE SET name = 'Alice';"
    );

    const auto tokens = lexer.tokenize();

    Parser parser(tokens);

    EXPECT_THROW(
        parser.parse_update(),
        std::invalid_argument
    );
}

TEST(ParserTest, RejectsUpdateWithoutSet) {
    Lexer lexer(
        "UPDATE student name = 'Alice';"
    );

    const auto tokens = lexer.tokenize();

    Parser parser(tokens);

    EXPECT_THROW(
        parser.parse_update(),
        std::invalid_argument
    );
}

TEST(ParserTest, RejectsUpdateWithoutColumnName) {
    Lexer lexer(
        "UPDATE student SET = 'Alice';"
    );

    const auto tokens = lexer.tokenize();

    Parser parser(tokens);

    EXPECT_THROW(
        parser.parse_update(),
        std::invalid_argument
    );
}

TEST(ParserTest, RejectsUpdateWithoutValue) {
    Lexer lexer(
        "UPDATE student SET name =;"
    );

    const auto tokens = lexer.tokenize();

    Parser parser(tokens);

    EXPECT_THROW(
        parser.parse_update(),
        std::invalid_argument
    );
}

TEST(ParserTest, RejectsUpdateWithTrailingTokens) {
    Lexer lexer(
        "UPDATE student SET name = 'Alice'; garbage"
    );

    const auto tokens = lexer.tokenize();

    Parser parser(tokens);

    EXPECT_THROW(
        parser.parse_update(),
        std::invalid_argument
    );
}

TEST(ParserTest, DispatcherParsesUpdate) {
    Lexer lexer(
        "UPDATE student SET name = 'Alice';"
    );

    const auto tokens = lexer.tokenize();

    Parser parser(tokens);

    auto statement = parser.parse();

    ASSERT_TRUE(
        std::holds_alternative<UpdateStatement>(statement)
    );

    const auto& update =
        std::get<UpdateStatement>(statement);

    EXPECT_EQ(
        update.table_name(),
        "student"
    );

    EXPECT_EQ(
        update.column_name(),
        "name"
    );

    EXPECT_EQ(
        update.value().type(),
        ExpressionType::STRING
    );

    EXPECT_EQ(
        update.value().value(),
        "Alice"
    );
}

TEST(ParserTest, ParsesDelete) {
    Lexer lexer(
        "DELETE FROM student;"
    );

    const auto tokens = lexer.tokenize();

    Parser parser(tokens);

    DeleteStatement statement = parser.parse_delete();

    EXPECT_EQ(statement.table_name(), "student");
}

TEST(ParserTest, DispatcherParsesDelete) {
    Lexer lexer(
        "DELETE FROM student;"
    );

    const auto tokens = lexer.tokenize();

    Parser parser(tokens);

    auto statement = parser.parse();

    ASSERT_TRUE(
        std::holds_alternative<DeleteStatement>(statement)
    );

    const auto& delete_statement =
        std::get<DeleteStatement>(statement);

    EXPECT_EQ(
        delete_statement.table_name(),
        "student"
    );
}

TEST(ParserTest, RejectsDeleteWithoutFrom) {
    Lexer lexer(
        "DELETE student;"
    );

    const auto tokens = lexer.tokenize();

    Parser parser(tokens);

    EXPECT_THROW(
        parser.parse_delete(),
        std::invalid_argument
    );
}

TEST(ParserTest, RejectsDeleteWithoutTableName) {
    Lexer lexer(
        "DELETE FROM;"
    );

    const auto tokens = lexer.tokenize();

    Parser parser(tokens);

    EXPECT_THROW(
        parser.parse_delete(),
        std::invalid_argument
    );
}

TEST(ParserTest, RejectsDeleteWithoutSemicolon) {
    Lexer lexer(
        "DELETE FROM student"
    );

    const auto tokens = lexer.tokenize();

    Parser parser(tokens);

    EXPECT_THROW(
        parser.parse_delete(),
        std::invalid_argument
    );
}

TEST(ParserTest, RejectsDeleteWithTrailingTokens) {
    Lexer lexer(
        "DELETE FROM student; garbage"
    );

    const auto tokens = lexer.tokenize();

    Parser parser(tokens);

    EXPECT_THROW(
        parser.parse_delete(),
        std::invalid_argument
    );
}

TEST(ParserTest, ParsesDeleteWithWhere) {
    Lexer lexer(
        "DELETE FROM student WHERE id = 1;"
    );

    const auto tokens = lexer.tokenize();

    Parser parser(tokens);

    DeleteStatement statement = parser.parse_delete();

    EXPECT_EQ(statement.table_name(), "student");
    ASSERT_TRUE(statement.has_condition());

    const Condition& condition = statement.condition();

    EXPECT_EQ(condition.left().type(), ExpressionType::IDENTIFIER);
    EXPECT_EQ(condition.left().value(), "id");

    EXPECT_EQ(condition.operator_(), "=");

    EXPECT_EQ(condition.right().type(), ExpressionType::INTEGER);
    EXPECT_EQ(condition.right().value(), "1");
}

TEST(ParserTest, DispatcherParsesDeleteWithWhere) {
    Lexer lexer(
        "DELETE FROM student WHERE id = 1;"
    );

    const auto tokens = lexer.tokenize();

    Parser parser(tokens);

    auto statement = parser.parse();

    ASSERT_TRUE(
        std::holds_alternative<DeleteStatement>(statement)
    );

    const auto& delete_statement =
        std::get<DeleteStatement>(statement);

    EXPECT_EQ(
        delete_statement.table_name(),
        "student"
    );

    ASSERT_TRUE(delete_statement.has_condition());

    EXPECT_EQ(
        delete_statement.condition().left().value(),
        "id"
    );

    EXPECT_EQ(
        delete_statement.condition().right().value(),
        "1"
    );
}

TEST(ParserTest, RejectsDeleteWhereWithoutLeftExpression) {
    Lexer lexer(
        "DELETE FROM student WHERE = 1;"
    );

    const auto tokens = lexer.tokenize();

    Parser parser(tokens);

    EXPECT_THROW(
        parser.parse_delete(),
        std::invalid_argument
    );
}

TEST(ParserTest, RejectsDeleteWhereWithoutOperator) {
    Lexer lexer(
        "DELETE FROM student WHERE id 1;"
    );

    const auto tokens = lexer.tokenize();

    Parser parser(tokens);

    EXPECT_THROW(
        parser.parse_delete(),
        std::invalid_argument
    );
}

TEST(ParserTest, RejectsDeleteWhereWithoutRightExpression) {
    Lexer lexer(
        "DELETE FROM student WHERE id =;"
    );

    const auto tokens = lexer.tokenize();

    Parser parser(tokens);

    EXPECT_THROW(
        parser.parse_delete(),
        std::invalid_argument
    );
}

TEST(ParserTest, RejectsDeleteWhereWithoutSemicolon) {
    Lexer lexer(
        "DELETE FROM student WHERE id = 1"
    );

    const auto tokens = lexer.tokenize();

    Parser parser(tokens);

    EXPECT_THROW(
        parser.parse_delete(),
        std::invalid_argument
    );
}

TEST(ParserTest, ParsesUpdateWithWhere) {
    Lexer lexer(
        "UPDATE student SET name = 'Alice' WHERE id = 1;"
    );

    const auto tokens = lexer.tokenize();
    Parser parser(tokens);

    UpdateStatement statement = parser.parse_update();

    EXPECT_EQ(statement.table_name(), "student");
    EXPECT_EQ(statement.column_name(), "name");
    EXPECT_EQ(statement.value().value(), "Alice");

    ASSERT_TRUE(statement.has_condition());
    EXPECT_EQ(statement.condition().left().value(), "id");
    EXPECT_EQ(statement.condition().operator_(), "=");
    EXPECT_EQ(statement.condition().right().value(), "1");
}

TEST(ParserTest, DispatcherParsesUpdateWithWhere) {
    Lexer lexer(
        "UPDATE student SET name = 'Alice' WHERE id = 1;"
    );

    const auto tokens = lexer.tokenize();
    Parser parser(tokens);

    auto statement = parser.parse();

    ASSERT_TRUE(std::holds_alternative<UpdateStatement>(statement));

    const auto& update = std::get<UpdateStatement>(statement);

    EXPECT_EQ(update.table_name(), "student");

    ASSERT_TRUE(update.has_condition());
    EXPECT_EQ(update.condition().left().value(), "id");
    EXPECT_EQ(update.condition().operator_(), "=");
    EXPECT_EQ(update.condition().right().value(), "1");
}

} // namespace flashdb