#include "query/project_executor.h"

#include <stdexcept>

namespace flashdb {

ProjectExecutor::ProjectExecutor(
    RecordExecutor& input,
    RecordFile& record_file,
    const std::vector<Expression>& columns)
    : input_(input),
      record_file_(record_file),
      columns_(columns),
      opened_(false) {
}

void ProjectExecutor::open() {
    input_.open();
    opened_ = true;
}

bool ProjectExecutor::has_next() {
    return opened_ && input_.has_next();
}

std::vector<std::string> ProjectExecutor::next() {
    if (!has_next()) {
        throw std::runtime_error(
            "ProjectExecutor: no more records"
        );
    }

    const RecordId rid = input_.next();

    std::vector<std::string> result;

    for (const Expression& column : columns_) {
        if (column.type() != ExpressionType::IDENTIFIER) {
            throw std::invalid_argument(
                "ProjectExecutor: column must be an identifier"
            );
        }

        result.push_back(
            record_file_.get(
                rid,
                column.value()
            )
        );
    }

    return result;
}

void ProjectExecutor::close() {
    input_.close();
    opened_ = false;
}

} // namespace flashdb