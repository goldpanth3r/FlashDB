#ifndef FLASHDB_PARSER_PARSER_H
#define FLASHDB_PARSER_PARSER_H

#include <cstddef>
#include <string>
#include <variant>
#include <vector>

#include "ast/create_table_statement.h"
#include "ast/insert_statement.h"
#include "ast/select_statement.h"
#include "ast/update_statement.h"
#include "ast/delete_statement.h"
#include "token.h"

namespace flashdb {

/**
 * Parser converts SQL tokens into an AST.
 *
 * Currently supported:
 *
 * SELECT name FROM student;
 * SELECT name, age FROM student;
 * SELECT name FROM student WHERE id = 1;
 * INSERT INTO student VALUES (1, 'Alice');
 * CREATE TABLE student(id INT, name VARCHAR(100));
 * UPDATE student SET name = 'Alice';
 */
class Parser {
public:
    /**
     * Create a parser from a list of tokens.
     */
    explicit Parser(const std::vector<Token>& tokens);

    /**
     * Parse a SELECT statement.
     */
    SelectStatement parse_select();

    /**
     * Parse an INSERT statement.
     */
    InsertStatement parse_insert();

    /**
     * Parse a CREATE TABLE statement.
     */
    CreateTableStatement parse_create_table();

    /**
     * Parse an UPDATE statement.
     */
    UpdateStatement parse_update();

    /**
    * Parse a DELETE statement.
    */
    DeleteStatement parse_delete();

    /**
     * Parse the next SQL statement.
     *
     * Returns either a SELECT, INSERT, CREATE TABLE,
     * or UPDATE AST.
     */
    std::variant<
        SelectStatement,
        InsertStatement,
        CreateTableStatement,
        UpdateStatement,
        DeleteStatement
    > parse();

private:
    const std::vector<Token>& tokens_;
    std::size_t position_;

    /**
     * Return the current token.
     */
    const Token& current() const;

    /**
     * Consume and return the current token.
     */
    const Token& consume();

    /**
     * Check whether all tokens have been consumed.
     */
    bool is_end() const;

    /**
     * Check whether the current token matches the given type and value.
     */
    bool match(
        TokenType type,
        const std::string& value
    ) const;

    /**
     * Require the current token to match the given type and value.
     */
    void expect(
        TokenType type,
        const std::string& value
    );

    /**
     * Check that no tokens remain after the statement.
     */
    void expect_end();

    /**
     * Convert the current token into an Expression.
     */
    Expression parse_expression();
};

} // namespace flashdb

#endif