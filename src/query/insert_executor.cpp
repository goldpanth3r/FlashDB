#include "query/insert_executor.h"

#include <stdexcept>
#include <unordered_map>

namespace flashdb {

// Prepare INSERT execution against the target table storage.
InsertExecutor::InsertExecutor(
    const Plan& plan,
    RecordFile& record_file)
    : plan_(plan),
      record_file_(record_file) {
}

// Convert the planned INSERT values into a physical table record.
RecordId InsertExecutor::execute() {

    if (plan_.get_name() != "Insert") {
        throw std::invalid_argument(
            "InsertExecutor: expected Insert plan"
        );
    }

    const std::vector<Expression>& values =
        plan_.get_values();

    const Schema& schema =
        record_file_.layout().schema();

    if (values.size() != schema.field_count()) {
        throw std::invalid_argument(
            "InsertExecutor: value count does not match table columns"
        );
    }

    std::unordered_map<std::string, std::string> record;

    const auto& fields = schema.fields();

    for (std::size_t i = 0; i < fields.size(); ++i) {
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

    // Send the completed record into the record storage layer.
    return record_file_.insert(record);
}

}