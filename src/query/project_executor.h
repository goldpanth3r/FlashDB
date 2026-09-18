#pragma once

#include <string>
#include <vector>

#include "parser/ast/expression.h"
#include "query/record_executor.h"
#include "record/record_file.h"

namespace flashdb {

class ProjectExecutor {
public:
    // ProjectExecutor creates an executor for selected columns.
    // Arguments:
    // input - executor providing records.
    // record_file - table containing the records.
    // columns - columns that should be returned.
    // Returns:
    // A new ProjectExecutor object.
    ProjectExecutor(
        RecordExecutor& input,
        RecordFile& record_file,
        const std::vector<Expression>& columns
    );

    // open starts the projection.
    // Arguments:
    // None.
    // Returns:
    // Nothing.
    void open();

    // has_next checks whether another projected row exists.
    // Arguments:
    // None.
    // Returns:
    // True if another row exists, otherwise false.
    bool has_next();

    // next returns the next projected row.
    // Arguments:
    // None.
    // Returns:
    // The selected column values.
    std::vector<std::string> next();

    // close stops the projection.
    // Arguments:
    // None.
    // Returns:
    // Nothing.
    void close();

private:
    RecordExecutor& input_;
    RecordFile& record_file_;
    const std::vector<Expression>& columns_;
    bool opened_;
};

} // namespace flashdb