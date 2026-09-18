#pragma once

#include <string>

#include "parser/ast/condition.h"

namespace flashdb {

class FilterPlan {
public:
    // FilterPlan creates a filter plan.
    // Arguments:
    // condition - condition used to filter rows.
    // Returns:
    // A new FilterPlan object.
    explicit FilterPlan(const Condition& condition);

    // get_condition returns the filter condition.
    // Arguments:
    // None.
    // Returns:
    // The filter condition.
    const Condition& get_condition() const;

private:
    Condition condition_;
};

} // namespace flashdb