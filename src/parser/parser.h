#ifndef FLASHDB_PARSER_PARSER_H
#define FLASHDB_PARSER_PARSER_H

#include <cstddef>
#include <string>
#include <variant>
#include <vector>

#include "ast/create_table_statement.h"
#include "ast/delete_statement.h"
#include "ast/insert_statement.h"
#include "ast/select_statement.h"
#include "ast/transaction_statement.h"
#include "ast/update_statement.h"
#include "token.h"

namespace flashdb {

/**
 * Parser converts SQL tokens into an AST.
 *
 * Currently supported:
 *
 * SELECT name FROM student;
 * INSERT INTO student VALUES (1, 'Alice');
 * CREATE TABLE student(id INT, name VARCHAR(100));
 * UPDATE student SET name = 'Alice';
 * DELETE FROM student WHERE id = 1;
 * BEGIN;
 * COMMIT;
 * ROLLBACK;
 */
class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens);

    SelectStatement parse_select();

    InsertStatement parse_insert();

    CreateTableStatement parse_create_table();

    UpdateStatement parse_update();

    DeleteStatement parse_delete();

    TransactionStatement parse_transaction();

    std::variant<
        SelectStatement,
        InsertStatement,
        CreateTableStatement,
        UpdateStatement,
        DeleteStatement,
        TransactionStatement
    > parse();

private:
    const std::vector<Token>& tokens_;
    std::size_t position_;

    const Token& current() const;

    const Token& consume();

    bool is_end() const;

    bool match(
        TokenType type,
        const std::string& value
    ) const;

    void expect(
        TokenType type,
        const std::string& value
    );

    void expect_end();

    Expression parse_expression();
};

} // namespace flashdb

#endif