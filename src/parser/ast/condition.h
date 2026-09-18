#ifndef FLASHDB_PARSER_AST_CONDITION_H
#define FLASHDB_PARSER_AST_CONDITION_H

#include <string>

#include "expression.h"

namespace flashdb {

/**
 * Condition represents a simple WHERE condition.
 *
 * Example:
 * WHERE id = 1
 *
 * It contains:
 * - left expression
 * - comparison operator
 * - right expression
 */
class Condition {
public:
    /**
     * Create a condition from two expressions and an operator.
     */
    Condition(
        Expression left,
        std::string operator_,
        Expression right
    );

    /**
     * Return the left expression.
     */
    const Expression& left() const;

    /**
     * Return the comparison operator.
     */
    const std::string& operator_() const;

    /**
     * Return the right expression.
     */
    const Expression& right() const;

private:
    Expression left_;
    std::string operator_value_;
    Expression right_;
};

} // namespace flashdb

#endif