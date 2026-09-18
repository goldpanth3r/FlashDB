#include "query/delete_executor.h"

#include <stdexcept>

#include "query/filter_executor.h"
#include "query/table_scan_executor.h"

namespace flashdb {

// Prepare DELETE execution against the target table storage.
DeleteExecutor::DeleteExecutor(
    const Plan& plan,
    RecordFile& record_file)
    : plan_(plan),
      record_file_(record_file) {
}

// Remove every record selected by the DELETE predicate.
std::size_t DeleteExecutor::execute() {

    if (plan_.get_name() != "Delete") {
        throw std::invalid_argument(
            "DeleteExecutor: expected Delete plan"
        );
    }

    TableScanExecutor table_scan(record_file_);

    table_scan.open();

    std::size_t deleted_count = 0;

    if (plan_.get_condition() == nullptr) {

        // Remove every stored record when no WHERE predicate exists.
        while (table_scan.has_next()) {
            const RecordId rid = table_scan.next();

            record_file_.remove(rid);

            ++deleted_count;
        }

        table_scan.close();

        return deleted_count;
    }

    // Restrict deletion to records matching the WHERE predicate.
    FilterExecutor filter(
        table_scan,
        record_file_,
        *plan_.get_condition()
    );

    filter.open();

    while (filter.has_next()) {
        const RecordId rid = filter.next();

        record_file_.remove(rid);

        ++deleted_count;
    }

    filter.close();

    return deleted_count;
}

}