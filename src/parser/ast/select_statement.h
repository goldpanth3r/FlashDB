#ifndef FLASHDB_PARSER_AST_SELECT_STATEMENT_H
#define FLASHDB_PARSER_AST_SELECT_STATEMENT_H

#include <memory>
#include <string>
#include <vector>

#include "condition.h"
#include "expression.h"

namespace flashdb {

/**
 * SelectStatement represents a SQL SELECT query.
 *
 * Example:
 * SELECT name
 * FROM student
 * WHERE id = 1;
 *
 * The WHERE condition is optional.
 */
class SelectStatement {
public:
    /**
     * Create a SELECT statement without a WHERE condition.
     */
    SelectStatement(
        std::vector<Expression> columns,
        std::string table_name
    );

    /**
     * Create a SELECT statement with a WHERE condition.
     */
    SelectStatement(
        std::vector<Expression> columns,
        std::string table_name,
        Condition condition
    );

    /**
     * Return the selected columns.
     */
    const std::vector<Expression>& columns() const;

    /**
     * Return the table name.
     */
    const std::string& table_name() const;

    /**
     * Check whether the query has a WHERE condition.
     */
    bool has_condition() const;

    /**
     * Return the WHERE condition.
     *
     * Throws std::runtime_error if no condition exists.
     */
    const Condition& condition() const;

private:
    std::vector<Expression> columns_;
    std::string table_name_;

    bool has_condition_;
    std::unique_ptr<Condition> condition_;
};

} // namespace flashdb

#endif