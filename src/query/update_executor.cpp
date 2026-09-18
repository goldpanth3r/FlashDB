#include "query/update_executor.h"

#include <memory>
#include <stdexcept>

#include "query/filter_executor.h"
#include "query/table_scan_executor.h"

namespace flashdb {

// Prepare UPDATE execution using the shared database storage.
UpdateExecutor::UpdateExecutor(
    const Plan& plan,
    Database& database)
    : plan_(plan),
      database_(database),
      transaction_(nullptr) {
}

// Prepare UPDATE execution so every write can be rolled back.
UpdateExecutor::UpdateExecutor(
    const Plan& plan,
    Database& database,
    Transaction& transaction)
    : plan_(plan),
      database_(database),
      transaction_(&transaction) {
}

// Validate the UPDATE and apply it through the selected transaction path.
std::size_t UpdateExecutor::execute() {

    if (plan_.get_name() != "Update") {
        throw std::invalid_argument(
            "UpdateExecutor: expected Update plan"
        );
    }

    std::unique_ptr<RecordFile> record_file =
        database_.open_table(
            plan_.get_table_name()
        );

    const std::vector<Expression>& columns =
        plan_.get_columns();

    const std::vector<Expression>& values =
        plan_.get_values();

    if (columns.size() != 1 ||
        values.size() != 1) {
        throw std::invalid_argument(
            "UpdateExecutor: invalid update plan"
        );
    }

    const Expression& column = columns[0];
    const Expression& value = values[0];

    if (column.type() != ExpressionType::IDENTIFIER) {
        throw std::invalid_argument(
            "UpdateExecutor: update target must be a column"
        );
    }

    const Schema& schema =
        record_file->layout().schema();

    const auto& fields = schema.fields();

    const Field* target_field = nullptr;

    for (const Field& field : fields) {
        if (field.name == column.value()) {
            target_field = &field;
            break;
        }
    }

    if (target_field == nullptr) {
        throw std::invalid_argument(
            "UpdateExecutor: unknown column"
        );
    }

    // Validate the SQL value against the physical column type.
    if (target_field->type == FieldType::INT) {
        if (value.type() != ExpressionType::INTEGER) {
            throw std::invalid_argument(
                "UpdateExecutor: expected integer value"
            );
        }
    }

    if (target_field->type == FieldType::STRING) {
        if (value.type() != ExpressionType::STRING) {
            throw std::invalid_argument(
                "UpdateExecutor: expected string value"
            );
        }
    }

    TableScanExecutor table_scan(*record_file);

    table_scan.open();

    std::size_t updated_count = 0;

    if (plan_.get_condition() == nullptr) {

        // Update every record when no WHERE predicate is present.
        while (table_scan.has_next()) {
            const RecordId rid = table_scan.next();

            if (transaction_ != nullptr) {
                // Record the old value before changing the row.
                transaction_->update(
                    plan_.get_table_name(),
                    rid,
                    column.value(),
                    value.value()
                );
            } else {
                record_file->set(
                    rid,
                    column.value(),
                    value.value()
                );
            }

            ++updated_count;
        }

        table_scan.close();

        return updated_count;
    }

    // Restrict writes to records matching the WHERE predicate.
    FilterExecutor filter(
        table_scan,
        *record_file,
        *plan_.get_condition()
    );

    filter.open();

    while (filter.has_next()) {
        const RecordId rid = filter.next();

        if (transaction_ != nullptr) {
            // Record the old value before changing the matching row.
            transaction_->update(
                plan_.get_table_name(),
                rid,
                column.value(),
                value.value()
            );
        } else {
            record_file->set(
                rid,
                column.value(),
                value.value()
            );
        }

        ++updated_count;
    }

    filter.close();

    return updated_count;
}

} // namespace flashdb