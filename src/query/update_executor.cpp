#include "query/update_executor.h"

#include <stdexcept>

#include "query/filter_executor.h"
#include "query/table_scan_executor.h"

namespace flashdb {

// Prepare UPDATE execution against the target table storage.
UpdateExecutor::UpdateExecutor(
    const Plan& plan,
    RecordFile& record_file)
    : plan_(plan),
      record_file_(record_file) {
}

// Apply the planned value to every record selected by the UPDATE predicate.
std::size_t UpdateExecutor::execute() {

    if (plan_.get_name() != "Update") {
        throw std::invalid_argument(
            "UpdateExecutor: expected Update plan"
        );
    }

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
        record_file_.layout().schema();

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

    TableScanExecutor table_scan(record_file_);

    table_scan.open();

    std::size_t updated_count = 0;

    if (plan_.get_condition() == nullptr) {

        // Update every record when no WHERE predicate is present.
        while (table_scan.has_next()) {
            const RecordId rid = table_scan.next();

            record_file_.set(
                rid,
                column.value(),
                value.value()
            );

            ++updated_count;
        }

        table_scan.close();

        return updated_count;
    }

    // Restrict writes to records matching the WHERE predicate.
    FilterExecutor filter(
        table_scan,
        record_file_,
        *plan_.get_condition()
    );

    filter.open();

    while (filter.has_next()) {
        const RecordId rid = filter.next();

        record_file_.set(
            rid,
            column.value(),
            value.value()
        );

        ++updated_count;
    }

    filter.close();

    return updated_count;
}

}