#include "query/delete_executor.h"

#include <memory>
#include <stdexcept>

#include "query/filter_executor.h"
#include "query/table_scan_executor.h"

namespace flashdb {

namespace {

// Remove the row from every index belonging to its table.
void remove_from_indexes(
    Database* database,
    const std::string& table_name,
    const RecordId& rid,
    RecordFile& record_file) {

    if (database == nullptr) {
        return;
    }

    const std::vector<std::string> index_names =
        database->index_manager().indexes_for_table(
            table_name
        );

    for (const std::string& index_name : index_names) {

        const IndexMetadata& metadata =
            database->index_manager().metadata(
                index_name
            );

        const std::string value =
            record_file.get(
                rid,
                metadata.column_name
            );

        int key;

        try {
            key = std::stoi(value);
        }
        catch (const std::exception&) {
            throw std::invalid_argument(
                "DeleteExecutor: indexed column must contain an integer"
            );
        }

        database->index_manager().remove(
            index_name,
            key,
            rid
        );
    }
}

} // namespace

// Prepare DELETE execution against an existing table file.
DeleteExecutor::DeleteExecutor(
    const Plan& plan,
    RecordFile& record_file)
    : plan_(plan),
      database_(nullptr),
      record_file_(&record_file),
      transaction_(nullptr) {
}

// Prepare DELETE execution using the shared database storage.
DeleteExecutor::DeleteExecutor(
    const Plan& plan,
    Database& database)
    : plan_(plan),
      database_(&database),
      record_file_(nullptr),
      transaction_(nullptr) {
}

// Prepare DELETE execution so removed records can be restored.
DeleteExecutor::DeleteExecutor(
    const Plan& plan,
    Database& database,
    Transaction& transaction)
    : plan_(plan),
      database_(&database),
      record_file_(nullptr),
      transaction_(&transaction) {
}

// Select records, remove their indexes, and delete the records.
std::size_t DeleteExecutor::execute() {

    if (plan_.get_name() != "Delete") {
        throw std::invalid_argument(
            "DeleteExecutor: expected Delete plan"
        );
    }

    if (record_file_ == nullptr &&
        database_ == nullptr) {

        throw std::runtime_error(
            "DeleteExecutor: no database storage"
        );
    }

    std::unique_ptr<RecordFile> opened_record_file;

    if (database_ != nullptr) {

        opened_record_file =
            database_->open_table(
                plan_.get_table_name()
            );

        record_file_ =
            opened_record_file.get();
    }

    TableScanExecutor table_scan(
        *record_file_
    );

    table_scan.open();

    std::size_t deleted_count = 0;

    if (plan_.get_condition() == nullptr) {

        // Delete every record when there is no WHERE condition.
        while (table_scan.has_next()) {

            const RecordId rid =
                table_scan.next();

            // Remove the index entry before deleting the row.
            remove_from_indexes(
                database_,
                plan_.get_table_name(),
                rid,
                *record_file_
            );

            if (transaction_ != nullptr) {

                // Save the deleted row so rollback can restore it.
                transaction_->remove(
                    plan_.get_table_name(),
                    rid
                );

            } else {

                // Delete the row directly.
                record_file_->remove(rid);
            }

            ++deleted_count;
        }

        table_scan.close();

        return deleted_count;
    }

    // Restrict deletion to records matching the WHERE condition.
    FilterExecutor filter(
        table_scan,
        *record_file_,
        *plan_.get_condition()
    );

    filter.open();

    while (filter.has_next()) {

        const RecordId rid =
            filter.next();

        // Remove the index entry before deleting the row.
        remove_from_indexes(
            database_,
            plan_.get_table_name(),
            rid,
            *record_file_
        );

        if (transaction_ != nullptr) {

            // Save the deleted row so rollback can restore it.
            transaction_->remove(
                plan_.get_table_name(),
                rid
            );

        } else {

            // Delete the row directly.
            record_file_->remove(rid);
        }

        ++deleted_count;
    }

    filter.close();

    return deleted_count;
}

} // namespace flashdb