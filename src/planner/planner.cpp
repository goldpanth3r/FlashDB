#include "planner/planner.h"

namespace flashdb {

// Build the execution structure required by each supported SQL operation.
std::unique_ptr<Plan> Planner::create_plan(
    const std::variant<
            SelectStatement,
            InsertStatement,
            CreateTableStatement,
            CreateIndexStatement,
            UpdateStatement,
            DeleteStatement,
            TransactionStatement
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

    if (const auto* create_table =
            std::get_if<CreateTableStatement>(&statement)) {

        Schema schema;

        // Convert SQL column definitions into the storage schema.
        for (const ColumnDefinition& column :
             create_table->columns()) {

            if (column.type == "INT") {
                schema.add_int_field(column.name);
                continue;
            }

            if (column.type == "VARCHAR") {
                schema.add_string_field(
                    column.name,
                    static_cast<std::size_t>(column.length)
                );
                continue;
            }
        }

        return std::make_unique<Plan>(
            "CreateTable",
            create_table->table_name(),
            nullptr,
            std::nullopt,
            std::vector<Expression>{},
            std::vector<Expression>{},
            std::move(schema)
        );
    }

    if (const auto* create_index =
            std::get_if<CreateIndexStatement>(&statement)) {

        // Carry CREATE INDEX information into the execution layer.
        return std::make_unique<Plan>(
            "CreateIndex",
            create_index->table_name(),
            nullptr,
            std::nullopt,
            std::vector<Expression>{
                Expression(
                    ExpressionType::IDENTIFIER,
                    create_index->index_name()
                ),
                Expression(
                    ExpressionType::IDENTIFIER,
                    create_index->column_name()
                )
            }
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

    if (const auto* delete_statement =
            std::get_if<DeleteStatement>(&statement)) {

        std::optional<Condition> condition;

        if (delete_statement->has_condition()) {
            condition = delete_statement->condition();
        }

        // Carry the DELETE predicate into the execution layer.
        return std::make_unique<Plan>(
            "Delete",
            delete_statement->table_name(),
            nullptr,
            std::move(condition)
        );
    }

    if (const auto* transaction =
            std::get_if<TransactionStatement>(&statement)) {

        if (transaction->command() ==
            TransactionCommand::BEGIN) {

            return std::make_unique<Plan>(
                "BeginTransaction",
                ""
            );
        }

        if (transaction->command() ==
            TransactionCommand::COMMIT) {

            return std::make_unique<Plan>(
                "CommitTransaction",
                ""
            );
        }

        if (transaction->command() ==
            TransactionCommand::ROLLBACK) {

            return std::make_unique<Plan>(
                "RollbackTransaction",
                ""
            );
        }
    }

    return nullptr;
}

} // namespace flashdb