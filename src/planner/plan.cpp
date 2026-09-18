#include "planner/plan.h"

namespace flashdb {

// Store the operations and data required to execute a planned statement.
Plan::Plan(
    const std::string& name,
    const std::string& table_name,
    std::unique_ptr<Plan> child,
    std::optional<Condition> condition,
    std::vector<Expression> columns,
    std::vector<Expression> values)
    : name_(name),
      table_name_(table_name),
      child_(std::move(child)),
      condition_(std::move(condition)),
      columns_(std::move(columns)),
      values_(std::move(values)) {
}

// Provide the operation type used by the execution layer.
const std::string& Plan::get_name() const {
    return name_;
}

// Provide the table targeted by the planned operation.
const std::string& Plan::get_table_name() const {
    return table_name_;
}

// Connect higher-level operations to their input plan.
const Plan* Plan::get_child() const {
    return child_.get();
}

// Provide the predicate used by filtering operations.
const Condition* Plan::get_condition() const {
    if (!condition_.has_value()) {
        return nullptr;
    }

    return &condition_.value();
}

// Provide the columns required by a projection.
const std::vector<Expression>& Plan::get_columns() const {
    return columns_;
}

// Provide the values carried by an INSERT operation.
const std::vector<Expression>& Plan::get_values() const {
    return values_;
}

}