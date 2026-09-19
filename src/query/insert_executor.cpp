#include "query/insert_executor.h"

#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

namespace flashdb {

// Prepare INSERT execution using the shared database storage.
InsertExecutor::InsertExecutor(
    const Plan& plan,
    Database& database)
    : plan_(plan),
      database_(database),
      transaction_(nullptr) {
}

// Prepare INSERT execution so the write can be rolled back.
InsertExecutor::InsertExecutor(
    const Plan& plan,
    Database& database,
    Transaction& transaction)
    : plan_(plan),
      database_(database),
      transaction_(&transaction) {
}

// Validate the INSERT, store the row, and update its indexes.
RecordId InsertExecutor::execute() {

    if (plan_.get_name() != "Insert") {
        throw std::invalid_argument(
            "InsertExecutor: expected Insert plan"
        );
    }

    std::unique_ptr<RecordFile> record_file =
        database_.open_table(
            plan_.get_table_name()
        );

    const std::vector<Expression>& values =
        plan_.get_values();

    const Schema& schema =
        record_file->layout().schema();

    if (values.size() != schema.field_count()) {
        throw std::invalid_argument(
            "InsertExecutor: value count does not match table columns"
        );
    }

    std::unordered_map<std::string, std::string> record;

    const auto& fields =
        schema.fields();

    for (std::size_t i = 0;
         i < fields.size();
         ++i) {

        const Field& field =
            fields[i];

        const Expression& value =
            values[i];

        // Validate the SQL value against the column type.
        if (field.type == FieldType::INT) {

            if (value.type() != ExpressionType::INTEGER) {
                throw std::invalid_argument(
                    "InsertExecutor: expected integer value"
                );
            }
        }

        if (field.type == FieldType::STRING) {

            if (value.type() != ExpressionType::STRING) {
                throw std::invalid_argument(
                    "InsertExecutor: expected string value"
                );
            }
        }

        record[field.name] =
            value.value();
    }

    RecordId rid =
        transaction_ != nullptr
            ? transaction_->insert(
                  plan_.get_table_name(),
                  record
              )
            : record_file->insert(record);

    /*
     * Add the new row to every index belonging
     * to this table.
     */
    const std::vector<std::string> index_names =
        database_.index_manager().indexes_for_table(
            plan_.get_table_name()
        );

    for (const std::string& index_name :
         index_names) {

        const IndexMetadata& metadata =
            database_.index_manager().metadata(
                index_name
            );

        const auto value_it =
            record.find(
                metadata.column_name
            );

        if (value_it == record.end()) {
            throw std::runtime_error(
                "InsertExecutor: indexed column not found"
            );
        }

        int key;

        try {
            key =
                std::stoi(
                    value_it->second
                );
        }
        catch (const std::exception&) {
            throw std::invalid_argument(
                "InsertExecutor: indexed column must contain an integer"
            );
        }

        database_.index_manager().insert(
            index_name,
            key,
            rid
        );
    }

    return rid;
}

} // namespace flashdb