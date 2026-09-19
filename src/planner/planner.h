#pragma once

#include <memory>
#include <variant>

#include "parser/ast/create_index_statement.h"
#include "parser/ast/create_table_statement.h"
#include "parser/ast/delete_statement.h"
#include "parser/ast/insert_statement.h"
#include "parser/ast/select_statement.h"
#include "parser/ast/transaction_statement.h"
#include "parser/ast/update_statement.h"
#include "planner/plan.h"

namespace flashdb {

class Planner {
public:
    // Convert a parsed SQL statement into an executable plan.
    std::unique_ptr<Plan> create_plan(
        const std::variant<
            SelectStatement,
            InsertStatement,
            CreateTableStatement,
            CreateIndexStatement,
            UpdateStatement,
            DeleteStatement,
            TransactionStatement
        >& statement
    );
};

} // namespace flashdb