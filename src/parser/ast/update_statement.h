#ifndef FLASHDB_PARSER_AST_UPDATE_STATEMENT_H
#define FLASHDB_PARSER_AST_UPDATE_STATEMENT_H

#include <memory>
#include <string>

#include "condition.h"
#include "expression.h"

namespace flashdb {

/**
 * UpdateStatement represents a SQL UPDATE statement.
 *
 * Examples:
 * UPDATE student SET name = 'Alice';
 * UPDATE student SET name = 'Alice' WHERE id = 1;
 *
 * The WHERE condition is optional.
 */
class UpdateStatement {
public:
    /**
     * Create an UPDATE statement without a WHERE condition.
     */
    UpdateStatement(
        std::string table_name,
        std::string column_name,
        Expression value
    );

    /**
     * Create an UPDATE statement with a WHERE condition.
     */
    UpdateStatement(
        std::string table_name,
        std::string column_name,
        Expression value,
        Condition condition
    );

    /**
     * Return the table name.
     */
    const std::string& table_name() const;

    /**
     * Return the column being updated.
     */
    const std::string& column_name() const;

    /**
     * Return the new value.
     */
    const Expression& value() const;

    /**
     * Check whether the UPDATE has a WHERE condition.
     */
    bool has_condition() const;

    /**
     * Return the WHERE condition.
     *
     * Throws std::runtime_error if no condition exists.
     */
    const Condition& condition() const;

private:
    std::string table_name_;
    std::string column_name_;
    Expression value_;

    bool has_condition_;
    std::unique_ptr<Condition> condition_;
};

} // namespace flashdb

#endif