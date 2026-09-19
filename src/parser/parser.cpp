#include "parser.h"

#include <stdexcept>
#include <utility>

namespace flashdb {

Parser::Parser(const std::vector<Token>& tokens)
    : tokens_(tokens),
      position_(0) {
}

const Token& Parser::current() const {
    if (is_end()) {
        throw std::runtime_error(
            "Parser::current: unexpected end of input"
        );
    }

    return tokens_[position_];
}

const Token& Parser::consume() {
    const Token& token = current();
    ++position_;
    return token;
}

bool Parser::is_end() const {
    return position_ >= tokens_.size();
}

bool Parser::match(
    TokenType type,
    const std::string& value) const {

    if (is_end()) {
        return false;
    }

    return current().type() == type &&
           current().value() == value;
}

void Parser::expect(
    TokenType type,
    const std::string& value) {

    if (is_end()) {
        throw std::invalid_argument(
            "Parser::expect: unexpected end of input"
        );
    }

    if (!match(type, value)) {
        throw std::invalid_argument(
            "Parser::expect: unexpected token"
        );
    }

    consume();
}

void Parser::expect_end() {
    if (!is_end()) {
        throw std::invalid_argument(
            "Parser::expect_end: unexpected tokens after statement"
        );
    }
}

Expression Parser::parse_expression() {
    const Token& token = consume();

    switch (token.type()) {
        case TokenType::IDENTIFIER:
            return Expression(
                ExpressionType::IDENTIFIER,
                token.value()
            );

        case TokenType::INTEGER:
            return Expression(
                ExpressionType::INTEGER,
                token.value()
            );

        case TokenType::STRING:
            return Expression(
                ExpressionType::STRING,
                token.value()
            );

        default:
            throw std::invalid_argument(
                "Parser::parse_expression: invalid expression"
            );
    }
}

SelectStatement Parser::parse_select() {
    expect(TokenType::KEYWORD, "SELECT");

    std::vector<Expression> columns;

    if (is_end() ||
        current().type() != TokenType::IDENTIFIER) {
        throw std::invalid_argument(
            "Parser::parse_select: expected column name"
        );
    }

    columns.push_back(parse_expression());

    while (match(TokenType::SYMBOL, ",")) {
        consume();

        if (is_end() ||
            current().type() != TokenType::IDENTIFIER) {
            throw std::invalid_argument(
                "Parser::parse_select: expected column name"
            );
        }

        columns.push_back(parse_expression());
    }

    expect(TokenType::KEYWORD, "FROM");

    if (is_end() ||
        current().type() != TokenType::IDENTIFIER) {
        throw std::invalid_argument(
            "Parser::parse_select: expected table name"
        );
    }

    const std::string table_name = consume().value();

    if (match(TokenType::KEYWORD, "WHERE")) {
        consume();

        Expression left = parse_expression();

        expect(TokenType::SYMBOL, "=");

        Expression right = parse_expression();

        Condition condition(
            std::move(left),
            "=",
            std::move(right)
        );

        expect(TokenType::SYMBOL, ";");
        expect_end();

        return SelectStatement(
            std::move(columns),
            table_name,
            std::move(condition)
        );
    }

    expect(TokenType::SYMBOL, ";");
    expect_end();

    return SelectStatement(
        std::move(columns),
        table_name
    );
}

InsertStatement Parser::parse_insert() {
    expect(TokenType::KEYWORD, "INSERT");
    expect(TokenType::KEYWORD, "INTO");

    if (is_end() ||
        current().type() != TokenType::IDENTIFIER) {
        throw std::invalid_argument(
            "Parser::parse_insert: expected table name"
        );
    }

    const std::string table_name = consume().value();

    expect(TokenType::KEYWORD, "VALUES");
    expect(TokenType::SYMBOL, "(");

    std::vector<Expression> values;

    values.push_back(parse_expression());

    while (match(TokenType::SYMBOL, ",")) {
        consume();
        values.push_back(parse_expression());
    }

    expect(TokenType::SYMBOL, ")");
    expect(TokenType::SYMBOL, ";");

    expect_end();

    return InsertStatement(
        table_name,
        std::move(values)
    );
}

CreateTableStatement Parser::parse_create_table() {
    expect(TokenType::KEYWORD, "CREATE");
    expect(TokenType::KEYWORD, "TABLE");

    if (is_end() ||
        current().type() != TokenType::IDENTIFIER) {
        throw std::invalid_argument(
            "Parser::parse_create_table: expected table name"
        );
    }

    const std::string table_name = consume().value();

    expect(TokenType::SYMBOL, "(");

    std::vector<ColumnDefinition> columns;

    while (!match(TokenType::SYMBOL, ")")) {
        if (is_end() ||
            current().type() != TokenType::IDENTIFIER) {
            throw std::invalid_argument(
                "Parser::parse_create_table: expected column name"
            );
        }

        const std::string column_name = consume().value();

        if (match(TokenType::KEYWORD, "INT")) {
            consume();

            columns.push_back({
                column_name,
                "INT",
                0
            });
        }
        else if (match(TokenType::KEYWORD, "VARCHAR")) {
            consume();

            expect(TokenType::SYMBOL, "(");

            if (is_end() ||
                current().type() != TokenType::INTEGER) {
                throw std::invalid_argument(
                    "Parser::parse_create_table: expected VARCHAR length"
                );
            }

            const int length =
                std::stoi(consume().value());

            if (length <= 0) {
                throw std::invalid_argument(
                    "Parser::parse_create_table: invalid VARCHAR length"
                );
            }

            expect(TokenType::SYMBOL, ")");

            columns.push_back({
                column_name,
                "VARCHAR",
                length
            });
        }
        else {
            throw std::invalid_argument(
                "Parser::parse_create_table: expected column type"
            );
        }

        if (match(TokenType::SYMBOL, ",")) {
            consume();
            continue;
        }

        break;
    }

    expect(TokenType::SYMBOL, ")");

    if (columns.empty()) {
        throw std::invalid_argument(
            "Parser::parse_create_table: table must have columns"
        );
    }

    expect(TokenType::SYMBOL, ";");

    expect_end();

    return CreateTableStatement(
        table_name,
        std::move(columns)
    );
}

CreateIndexStatement Parser::parse_create_index() {
    expect(TokenType::KEYWORD, "CREATE");
    expect(TokenType::KEYWORD, "INDEX");

    if (is_end() ||
        current().type() != TokenType::IDENTIFIER) {
        throw std::invalid_argument(
            "Parser::parse_create_index: expected index name"
        );
    }

    const std::string index_name =
        consume().value();

    expect(TokenType::KEYWORD, "ON");

    if (is_end() ||
        current().type() != TokenType::IDENTIFIER) {
        throw std::invalid_argument(
            "Parser::parse_create_index: expected table name"
        );
    }

    const std::string table_name =
        consume().value();

    expect(TokenType::SYMBOL, "(");

    if (is_end() ||
        current().type() != TokenType::IDENTIFIER) {
        throw std::invalid_argument(
            "Parser::parse_create_index: expected column name"
        );
    }

    const std::string column_name =
        consume().value();

    expect(TokenType::SYMBOL, ")");
    expect(TokenType::SYMBOL, ";");

    expect_end();

    return CreateIndexStatement(
        index_name,
        table_name,
        column_name
    );
}

UpdateStatement Parser::parse_update() {
    expect(TokenType::KEYWORD, "UPDATE");

    const std::string table_name = consume().value();

    expect(TokenType::KEYWORD, "SET");

    const std::string column_name = consume().value();

    expect(TokenType::SYMBOL, "=");

    Expression value = parse_expression();

    if (match(TokenType::KEYWORD, "WHERE")) {
        consume();

        Expression left = parse_expression();

        const std::string operator_value = consume().value();

        Expression right = parse_expression();

        Condition condition(
            std::move(left),
            operator_value,
            std::move(right)
        );

        expect(TokenType::SYMBOL, ";");
        expect_end();

        return UpdateStatement(
            table_name,
            column_name,
            std::move(value),
            std::move(condition)
        );
    }

    expect(TokenType::SYMBOL, ";");
    expect_end();

    return UpdateStatement(
        table_name,
        column_name,
        std::move(value)
    );
}

DeleteStatement Parser::parse_delete() {
    expect(TokenType::KEYWORD, "DELETE");
    expect(TokenType::KEYWORD, "FROM");

    if (is_end() ||
        current().type() != TokenType::IDENTIFIER) {
        throw std::invalid_argument(
            "Parser::parse_delete: expected table name"
        );
    }

    const std::string table_name = consume().value();

    if (match(TokenType::KEYWORD, "WHERE")) {
        consume();

        Expression left = parse_expression();

        expect(TokenType::SYMBOL, "=");

        Expression right = parse_expression();

        Condition condition(
            std::move(left),
            "=",
            std::move(right)
        );

        expect(TokenType::SYMBOL, ";");

        expect_end();

        return DeleteStatement(
            table_name,
            std::move(condition)
        );
    }

    expect(TokenType::SYMBOL, ";");

    expect_end();

    return DeleteStatement(
        table_name
    );
}

TransactionStatement Parser::parse_transaction() {
    if (match(TokenType::KEYWORD, "BEGIN")) {
        consume();

        expect(TokenType::SYMBOL, ";");
        expect_end();

        return TransactionStatement(
            TransactionCommand::BEGIN
        );
    }

    if (match(TokenType::KEYWORD, "COMMIT")) {
        consume();

        expect(TokenType::SYMBOL, ";");
        expect_end();

        return TransactionStatement(
            TransactionCommand::COMMIT
        );
    }

    if (match(TokenType::KEYWORD, "ROLLBACK")) {
        consume();

        expect(TokenType::SYMBOL, ";");
        expect_end();

        return TransactionStatement(
            TransactionCommand::ROLLBACK
        );
    }

    throw std::invalid_argument(
        "Parser::parse_transaction: unsupported transaction command"
    );
}

// Route SQL statements to their matching parser.
std::variant<
    SelectStatement,
    InsertStatement,
    CreateTableStatement,
    CreateIndexStatement,
    UpdateStatement,
    DeleteStatement,
    TransactionStatement
> Parser::parse() {

    if (match(TokenType::KEYWORD, "SELECT")) {
        return parse_select();
    }

    if (match(TokenType::KEYWORD, "INSERT")) {
        return parse_insert();
    }

    if (match(TokenType::KEYWORD, "CREATE")) {

        // CREATE INDEX and CREATE TABLE share the CREATE keyword.
        if (position_ + 1 < tokens_.size() &&
            tokens_[position_ + 1].type() == TokenType::KEYWORD &&
            tokens_[position_ + 1].value() == "INDEX") {

            return parse_create_index();
        }

        return parse_create_table();
    }

    if (match(TokenType::KEYWORD, "UPDATE")) {
        return parse_update();
    }

    if (match(TokenType::KEYWORD, "DELETE")) {
        return parse_delete();
    }

    if (match(TokenType::KEYWORD, "BEGIN") ||
        match(TokenType::KEYWORD, "COMMIT") ||
        match(TokenType::KEYWORD, "ROLLBACK")) {

        return parse_transaction();
    }

    throw std::invalid_argument(
        "Parser::parse: unsupported statement"
    );
}

} // namespace flashdb