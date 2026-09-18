#pragma once

#include <optional>

#include "parser/ast/condition.h"
#include "query/record_executor.h"
#include "record/record_file.h"

namespace flashdb {

class FilterExecutor : public RecordExecutor {
public:
    // FilterExecutor creates an executor that filters table records.
    // Arguments:
    // input - executor providing records to filter.
    // record_file - table containing the records.
    // condition - condition used to filter records.
    // Returns:
    // A new FilterExecutor object.
    FilterExecutor(
        RecordExecutor& input,
        RecordFile& record_file,
        const Condition& condition
    );

    // open starts the filtered scan.
    // Arguments:
    // None.
    // Returns:
    // Nothing.
    void open() override;

    // has_next checks whether another matching record exists.
    // Arguments:
    // None.
    // Returns:
    // True if another matching record exists, otherwise false.
    bool has_next() override;

    // next returns the next matching record.
    // Arguments:
    // None.
    // Returns:
    // The next matching record identifier.
    // Throws:
    // std::runtime_error if no matching record exists.
    RecordId next() override;

    // close stops the filtered scan.
    // Arguments:
    // None.
    // Returns:
    // Nothing.
    void close() override;

private:
    // find_next_matching_record searches for the next matching record.
    // Arguments:
    // None.
    // Returns:
    // Nothing.
    void find_next_matching_record();

    // matches checks whether one record satisfies the condition.
    // Arguments:
    // rid - record being checked.
    // Returns:
    // True if the record satisfies the condition, otherwise false.
    bool matches(const RecordId& rid) const;

    RecordExecutor& input_;
    RecordFile& record_file_;
    const Condition& condition_;

    std::optional<RecordId> next_record_;
    bool opened_;
};

} // namespace flashdb