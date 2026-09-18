#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "parser/ast/condition.h"
#include "parser/ast/expression.h"

namespace flashdb {

class Plan {
public:
    Plan(
        const std::string& name,
        const std::string& table_name,
        std::unique_ptr<Plan> child = nullptr,
        std::optional<Condition> condition = std::nullopt,
        std::vector<Expression> columns = {},
        std::vector<Expression> values = {}
    );

    const std::string& get_name() const;
    const std::string& get_table_name() const;
    const Plan* get_child() const;
    const Condition* get_condition() const;
    const std::vector<Expression>& get_columns() const;
    const std::vector<Expression>& get_values() const;

private:
    std::string name_;
    std::string table_name_;
    std::unique_ptr<Plan> child_;
    std::optional<Condition> condition_;
    std::vector<Expression> columns_;
    std::vector<Expression> values_;
};

}