#ifndef FLASHDB_PARSER_AST_DELETE_STATEMENT_H
#define FLASHDB_PARSER_AST_DELETE_STATEMENT_H

#include <memory>
#include <string>

#include "condition.h"

namespace flashdb {

/**
 * DeleteStatement represents a SQL DELETE statement.
 *
 * Examples:
 * DELETE FROM student;
 * DELETE FROM student WHERE id = 1;
 *
 * The WHERE condition is optional.
 */
class DeleteStatement {
public:
    /**
     * Create a DELETE statement without a WHERE condition.
     */
    explicit DeleteStatement(
        std::string table_name
    );

    /**
     * Create a DELETE statement with a WHERE condition.
     */
    DeleteStatement(
        std::string table_name,
        Condition condition
    );

    /**
     * Return the table name.
     */
    const std::string& table_name() const;

    /**
     * Check whether the DELETE has a WHERE condition.
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

    bool has_condition_;
    std::unique_ptr<Condition> condition_;
};

} // namespace flashdb

#endif