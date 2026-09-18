#pragma once

#include <string>
#include <vector>

#include "database.h"
#include "planner/plan.h"

namespace flashdb {

class QueryExecutor {
public:
    QueryExecutor(
        const Plan& plan,
        Database& database
    );

    std::vector<std::vector<std::string>> execute();

private:
    const Plan& plan_;
    Database& database_;
};

} // namespace flashdb