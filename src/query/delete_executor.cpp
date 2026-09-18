#include "query/delete_executor.h"

#include <memory>
#include <stdexcept>

#include "query/filter_executor.h"
#include "query/table_scan_executor.h"

namespace flashdb {

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

// Select records and remove them through the selected execution path.
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

        record_file_ = opened_record_file.get();
    }

    TableScanExecutor table_scan(*record_file_);

    table_scan.open();

    std::size_t deleted_count = 0;

    if (plan_.get_condition() == nullptr) {

        // Delete every record when no WHERE predicate is present.
        while (table_scan.has_next()) {
            const RecordId rid = table_scan.next();

            if (transaction_ != nullptr) {
                // Save the deleted row so rollback can restore it.
                transaction_->remove(
                    plan_.get_table_name(),
                    rid
                );
            } else {
                record_file_->remove(rid);
            }

            ++deleted_count;
        }

        table_scan.close();

        return deleted_count;
    }

    // Restrict deletion to records matching the WHERE predicate.
    FilterExecutor filter(
        table_scan,
        *record_file_,
        *plan_.get_condition()
    );

    filter.open();

    while (filter.has_next()) {
        const RecordId rid = filter.next();

        if (transaction_ != nullptr) {
            // Save the deleted row so rollback can restore it.
            transaction_->remove(
                plan_.get_table_name(),
                rid
            );
        } else {
            record_file_->remove(rid);
        }

        ++deleted_count;
    }

    filter.close();

    return deleted_count;
}

} // namespace flashdb