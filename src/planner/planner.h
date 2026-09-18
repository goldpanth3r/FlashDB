#pragma once

#include <memory>
#include <variant>

#include "parser/parser.h"
#include "planner/plan.h"

namespace flashdb {

class Planner {
public:
    // Convert a parsed SQL statement into a plan for execution.
    std::unique_ptr<Plan> create_plan(
        const std::variant<
            SelectStatement,
            InsertStatement,
            CreateTableStatement,
            UpdateStatement,
            DeleteStatement,
            TransactionStatement
        >& statement);
};

} // namespace flashdb