#ifndef FLASHDB_PARSER_AST_EXPRESSION_H
#define FLASHDB_PARSER_AST_EXPRESSION_H

#include <string>

namespace flashdb {

/**
 * Expression represents one value used by a SQL statement.
 *
 * Examples:
 *   id
 *   123
 *   'Alice'
 */
enum class ExpressionType {
    IDENTIFIER,
    INTEGER,
    STRING
};

class Expression {
public:
    Expression(
        ExpressionType type,
        std::string value
    );

    ExpressionType type() const;

    const std::string& value() const;

private:
    ExpressionType type_;
    std::string value_;
};

} // namespace flashdb

#endif