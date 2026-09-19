#pragma once

#include <cstddef>
#include <vector>

#include "query/record_executor.h"

namespace flashdb {

class IndexScanExecutor : public RecordExecutor {
public:
    // Create an executor over records returned by an index lookup.
    // Arguments:
    // record_ids - records found by the index.
    // Returns:
    // A new IndexScanExecutor object.
    explicit IndexScanExecutor(
        std::vector<RecordId> record_ids
    );

    // Start the index scan.
    // Arguments:
    // None.
    // Returns:
    // Nothing.
    void open() override;

    // Check whether another indexed record exists.
    // Arguments:
    // None.
    // Returns:
    // True if another record exists, otherwise false.
    bool has_next() override;

    // Return the next record found by the index.
    // Arguments:
    // None.
    // Returns:
    // The next RecordId.
    RecordId next() override;

    // Stop the index scan.
    // Arguments:
    // None.
    // Returns:
    // Nothing.
    void close() override;

private:
    std::vector<RecordId> record_ids_;
    std::size_t current_index_;
    bool opened_;
};

} // namespace flashdb