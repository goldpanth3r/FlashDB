#include "planner/filter_plan.h"

namespace flashdb {

FilterPlan::FilterPlan(const Condition& condition)
    : condition_(condition) {
}

const Condition& FilterPlan::get_condition() const {
    return condition_;
}

} // namespace flashdb