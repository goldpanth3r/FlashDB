#include "planner/planner.h"

namespace flashdb {

// Build the execution structure required by each supported SQL operation.
std::unique_ptr<Plan> Planner::create_plan(
    const std::variant<
        SelectStatement,
        InsertStatement,
        CreateTableStatement,
        UpdateStatement,
        DeleteStatement
    >& statement) {

    if (const auto* select =
            std::get_if<SelectStatement>(&statement)) {

        std::unique_ptr<Plan> plan =
            std::make_unique<Plan>(
                "TableScan",
                select->table_name()
            );

        // Add filtering between storage access and projection.
        if (select->has_condition()) {
            plan = std::make_unique<Plan>(
                "Filter",
                select->table_name(),
                std::move(plan),
                select->condition()
            );
        }

        // Finish SELECT planning with the requested result columns.
        return std::make_unique<Plan>(
            "Project",
            select->table_name(),
            std::move(plan),
            std::nullopt,
            select->columns()
        );
    }

    if (const auto* insert =
            std::get_if<InsertStatement>(&statement)) {

        // Carry INSERT values from the SQL layer into execution.
        return std::make_unique<Plan>(
            "Insert",
            insert->table_name(),
            nullptr,
            std::nullopt,
            std::vector<Expression>{},
            insert->values()
        );
    }

    if (const auto* update =
            std::get_if<UpdateStatement>(&statement)) {

        // Carry the target column and new value into UPDATE execution.
        std::vector<Expression> columns;

        columns.emplace_back(
            ExpressionType::IDENTIFIER,
            update->column_name()
        );

        std::vector<Expression> values;

        values.push_back(update->value());

        // Keep the optional WHERE predicate attached to the UPDATE plan.
        std::optional<Condition> condition;

        if (update->has_condition()) {
            condition = update->condition();
        }

        return std::make_unique<Plan>(
            "Update",
            update->table_name(),
            nullptr,
            std::move(condition),
            std::move(columns),
            std::move(values)
        );
    }

    return nullptr;
}

}