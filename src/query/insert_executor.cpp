#include "query/insert_executor.h"

#include <stdexcept>
#include <unordered_map>
#include <utility>

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

// Validate the INSERT values and send the write through the selected path.
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

    const auto& fields = schema.fields();

    for (std::size_t i = 0;
         i < fields.size();
         ++i) {

        const Field& field = fields[i];
        const Expression& value = values[i];

        // Convert SQL literals into the storage representation.
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

        record[field.name] = value.value();
    }

    if (transaction_ != nullptr) {
        // Record the write in the transaction so rollback can undo it.
        return transaction_->insert(
            plan_.get_table_name(),
            record
        );
    }

    // Keep direct storage execution available for existing callers.
    return record_file->insert(record);
}

} // namespace flashdb