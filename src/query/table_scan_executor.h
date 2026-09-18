#pragma once

#include <cstddef>
#include <vector>

#include "query/record_executor.h"
#include "record/record_file.h"

namespace flashdb {

class TableScanExecutor : public RecordExecutor {
public:
    // TableScanExecutor creates an executor for scanning a table.
    // Arguments:
    // record_file - record file containing the table.
    // Returns:
    // A new TableScanExecutor object.
    explicit TableScanExecutor(RecordFile& record_file);

    // open starts the table scan.
    // Arguments:
    // None.
    // Returns:
    // Nothing.
    void open() override;

    // has_next checks whether another record is available.
    // Arguments:
    // None.
    // Returns:
    // True if another record is available, otherwise false.
    bool has_next() override;

    // next returns the next record identifier.
    // Arguments:
    // None.
    // Returns:
    // The next record identifier.
    // Throws:
    // std::runtime_error if no record is available.
    RecordId next() override;

    // close stops the table scan.
    // Arguments:
    // None.
    // Returns:
    // Nothing.
    void close() override;

    // record_file returns the table being scanned.
    // Arguments:
    // None.
    // Returns:
    // The record file used by this executor.
    RecordFile& record_file();

private:
    RecordFile& record_file_;
    std::vector<RecordId> records_;
    std::size_t current_index_;
    bool opened_;
};

} // namespace flashdb