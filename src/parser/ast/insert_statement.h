#ifndef FLASHDB_PARSER_AST_INSERT_STATEMENT_H
#define FLASHDB_PARSER_AST_INSERT_STATEMENT_H

#include <string>
#include <vector>

#include "expression.h"

namespace flashdb {

/**
 * InsertStatement represents a SQL INSERT query.
 *
 * Example:
 * INSERT INTO student VALUES (1, 'Alice');
 */
class InsertStatement {
public:
    /**
     * Create an INSERT statement.
     */
    InsertStatement(
        std::string table_name,
        std::vector<Expression> values
    );

    /**
     * Return the table name.
     */
    const std::string& table_name() const;

    /**
     * Return the values being inserted.
     */
    const std::vector<Expression>& values() const;

private:
    std::string table_name_;
    std::vector<Expression> values_;
};

} // namespace flashdb

#endif