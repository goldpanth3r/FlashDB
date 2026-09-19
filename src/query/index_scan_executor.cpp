#include "query/index_scan_executor.h"

#include <stdexcept>
#include <utility>

namespace flashdb {

// Store the RecordIds returned by an index lookup.
IndexScanExecutor::IndexScanExecutor(
    std::vector<RecordId> record_ids)
    : record_ids_(std::move(record_ids)),
      current_index_(0),
      opened_(false) {
}

// Start reading records from the beginning of the index results.
void IndexScanExecutor::open() {
    current_index_ = 0;
    opened_ = true;
}

// Check whether another indexed record is available.
bool IndexScanExecutor::has_next() {
    return opened_ &&
           current_index_ < record_ids_.size();
}

// Return the next RecordId from the index results.
RecordId IndexScanExecutor::next() {

    if (!has_next()) {
        throw std::runtime_error(
            "IndexScanExecutor: no more records"
        );
    }

    return record_ids_[current_index_++];
}

// Stop the index scan.
void IndexScanExecutor::close() {
    opened_ = false;
}

} // namespace flashdb