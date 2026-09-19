#include "query/update_executor.h"

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "query/filter_executor.h"
#include "query/table_scan_executor.h"

namespace flashdb {

namespace {

// Remove an updated row from indexes that reference the changed column.
void remove_old_index_entries(
    Database& database,
    const std::string& table_name,
    const std::string& column_name,
    const RecordId& rid,
    RecordFile& record_file) {

    const std::vector<std::string> index_names =
        database.index_manager().indexes_for_table(
            table_name
        );

    for (const std::string& index_name : index_names) {

        const IndexMetadata& metadata =
            database.index_manager().metadata(
                index_name
            );

        if (metadata.column_name != column_name) {
            continue;
        }

        const std::string old_value =
            record_file.get(
                rid,
                column_name
            );

        int old_key;

        try {
            old_key = std::stoi(old_value);
        }
        catch (const std::exception&) {
            throw std::invalid_argument(
                "UpdateExecutor: indexed column must contain an integer"
            );
        }

        database.index_manager().remove(
            index_name,
            old_key,
            rid
        );
    }
}

// Add an updated row to indexes that reference the changed column.
void add_new_index_entries(
    Database& database,
    const std::string& table_name,
    const std::string& column_name,
    const std::string& new_value,
    const RecordId& rid) {

    const std::vector<std::string> index_names =
        database.index_manager().indexes_for_table(
            table_name
        );

    for (const std::string& index_name : index_names) {

        const IndexMetadata& metadata =
            database.index_manager().metadata(
                index_name
            );

        if (metadata.column_name != column_name) {
            continue;
        }

        int new_key;

        try {
            new_key = std::stoi(new_value);
        }
        catch (const std::exception&) {
            throw std::invalid_argument(
                "UpdateExecutor: indexed column must contain an integer"
            );
        }

        database.index_manager().insert(
            index_name,
            new_key,
            rid
        );
    }
}

} // namespace

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

// Validate the UPDATE, modify rows, and maintain affected indexes.
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

    const Expression& column =
        columns[0];

    const Expression& value =
        values[0];

    if (column.type() != ExpressionType::IDENTIFIER) {
        throw std::invalid_argument(
            "UpdateExecutor: update target must be a column"
        );
    }

    const Schema& schema =
        record_file->layout().schema();

    const auto& fields =
        schema.fields();

    const Field* target_field =
        nullptr;

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

    // Validate the new value against the column type.
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

    TableScanExecutor table_scan(
        *record_file
    );

    table_scan.open();

    std::size_t updated_count = 0;

    // Update every row when there is no WHERE condition.
    if (plan_.get_condition() == nullptr) {

        while (table_scan.has_next()) {

            const RecordId rid =
                table_scan.next();

            /*
             * Only remove an index entry when the
             * updated column is actually indexed.
             */
            remove_old_index_entries(
                database_,
                plan_.get_table_name(),
                column.value(),
                rid,
                *record_file
            );

            if (transaction_ != nullptr) {

                // Record the old value before changing the row.
                transaction_->update(
                    plan_.get_table_name(),
                    rid,
                    column.value(),
                    value.value()
                );

            } else {

                // Update the row directly.
                record_file->set(
                    rid,
                    column.value(),
                    value.value()
                );
            }

            /*
             * Add the row again using its new
             * indexed value.
             */
            add_new_index_entries(
                database_,
                plan_.get_table_name(),
                column.value(),
                value.value(),
                rid
            );

            ++updated_count;
        }

        table_scan.close();

        return updated_count;
    }

    // Restrict the update to rows matching the WHERE condition.
    FilterExecutor filter(
        table_scan,
        *record_file,
        *plan_.get_condition()
    );

    filter.open();

    while (filter.has_next()) {

        const RecordId rid =
            filter.next();

        /*
         * Remove the old index entry before
         * changing the indexed column.
         */
        remove_old_index_entries(
            database_,
            plan_.get_table_name(),
            column.value(),
            rid,
            *record_file
        );

        if (transaction_ != nullptr) {

            // Record the old value before changing the row.
            transaction_->update(
                plan_.get_table_name(),
                rid,
                column.value(),
                value.value()
            );

        } else {

            // Update the row directly.
            record_file->set(
                rid,
                column.value(),
                value.value()
            );
        }

        // Add the new indexed value.
        add_new_index_entries(
            database_,
            plan_.get_table_name(),
            column.value(),
            value.value(),
            rid
        );

        ++updated_count;
    }

    filter.close();

    return updated_count;
}

} // namespace flashdb