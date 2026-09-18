#include "query/filter_executor.h"

#include <stdexcept>

namespace flashdb {

FilterExecutor::FilterExecutor(
    RecordExecutor& input,
    RecordFile& record_file,
    const Condition& condition)
    : input_(input),
      record_file_(record_file),
      condition_(condition),
      next_record_(std::nullopt),
      opened_(false) {
}

void FilterExecutor::open() {
    input_.open();

    next_record_ = std::nullopt;
    opened_ = true;

    find_next_matching_record();
}

bool FilterExecutor::has_next() {
    if (!opened_) {
        return false;
    }

    if (!next_record_.has_value()) {
        find_next_matching_record();
    }

    return next_record_.has_value();
}

RecordId FilterExecutor::next() {
    if (!has_next()) {
        throw std::runtime_error(
            "FilterExecutor: no more matching records"
        );
    }

    const RecordId result = next_record_.value();

    next_record_ = std::nullopt;

    return result;
}

void FilterExecutor::close() {
    input_.close();

    next_record_ = std::nullopt;
    opened_ = false;
}

void FilterExecutor::find_next_matching_record() {
    next_record_ = std::nullopt;

    while (input_.has_next()) {
        const RecordId rid = input_.next();

        if (matches(rid)) {
            next_record_ = rid;
            return;
        }
    }
}

bool FilterExecutor::matches(const RecordId& rid) const {
    const Expression& left = condition_.left();
    const Expression& right = condition_.right();

    if (left.type() != ExpressionType::IDENTIFIER) {
        throw std::invalid_argument(
            "FilterExecutor: left side must be a column"
        );
    }

    if (condition_.operator_() != "=") {
        throw std::invalid_argument(
            "FilterExecutor: unsupported operator"
        );
    }

    return record_file_.get(
        rid,
        left.value()
    ) == right.value();
}

} // namespace flashdb