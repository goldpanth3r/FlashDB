#include "query/table_scan_executor.h"

#include <stdexcept>

namespace flashdb {

TableScanExecutor::TableScanExecutor(
    RecordFile& record_file)
    : record_file_(record_file),
      current_index_(0),
      opened_(false) {
}

void TableScanExecutor::open() {
    records_ = record_file_.scan();
    current_index_ = 0;
    opened_ = true;
}

bool TableScanExecutor::has_next() {
    return opened_ &&
           current_index_ < records_.size();
}

RecordId TableScanExecutor::next() {
    if (!has_next()) {
        throw std::runtime_error(
            "TableScanExecutor: no more records"
        );
    }

    return records_[current_index_++];
}

void TableScanExecutor::close() {
    records_.clear();
    current_index_ = 0;
    opened_ = false;
}

RecordFile& TableScanExecutor::record_file() {
    return record_file_;
}

} // namespace flashdb