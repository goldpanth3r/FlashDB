#include "query/create_index_executor.h"

#include <stdexcept>

namespace flashdb {

// Prepare CREATE INDEX execution using the shared database managers.
CreateIndexExecutor::CreateIndexExecutor(
    const Plan& plan,
    Database& database)
    : plan_(plan),
      database_(database) {
}

// Create the index and populate it with existing table records.
void CreateIndexExecutor::execute() {

    if (plan_.get_name() != "CreateIndex") {
        throw std::invalid_argument(
            "CreateIndexExecutor: expected CreateIndex plan"
        );
    }

    const auto& columns =
        plan_.get_columns();

    if (columns.size() != 2) {
        throw std::invalid_argument(
            "CreateIndexExecutor: invalid index plan"
        );
    }

    const std::string& index_name =
        columns[0].value();

    const std::string& column_name =
        columns[1].value();

    const std::string& table_name =
        plan_.get_table_name();

    if (database_.index_manager().has_index(index_name)) {
        throw std::invalid_argument(
            "CreateIndexExecutor: index already exists"
        );
    }

    const Schema& schema =
        database_.catalog().get_schema(table_name);

    bool column_exists = false;

    for (const Field& field : schema.fields()) {
        if (field.name == column_name) {
            column_exists = true;
            break;
        }
    }

    if (!column_exists) {
        throw std::invalid_argument(
            "CreateIndexExecutor: column does not exist"
        );
    }

    database_.index_manager().create_index(
        index_name,
        table_name,
        column_name
    );

    std::unique_ptr<RecordFile> record_file =
        database_.open_table(table_name);

    // Add all existing records to the new index.
    for (const RecordId& rid :
         record_file->scan()) {

        const std::string value =
            record_file->get(
                rid,
                column_name
            );

        int key = 0;

        try {
            key = std::stoi(value);
        }
        catch (const std::exception&) {
            throw std::invalid_argument(
                "CreateIndexExecutor: index column must contain integers"
            );
        }

        database_.index_manager().insert(
            index_name,
            key,
            rid
        );
    }
}

} // namespace flashdb