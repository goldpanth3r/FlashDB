#pragma once

#include <memory>
#include <variant>

#include "parser/parser.h"
#include "planner/plan.h"

namespace flashdb {

class Planner {
public:
    // create_plan converts a parsed SQL statement into a query plan.
    // Arguments:
    // statement - parsed SQL statement stored in a variant.
    // Returns:
    // A query plan.
    std::unique_ptr<Plan> create_plan(
        const std::variant<
            SelectStatement,
            InsertStatement,
            CreateTableStatement,
            UpdateStatement,
            DeleteStatement
        >& statement);
};

} // namespace flashdb