#pragma once

#include "database.h"
#include "planner/plan.h"

namespace flashdb {

class CreateTableExecutor {
public:
    CreateTableExecutor(
        const Plan& plan,
        Database& database
    );

    void execute();

private:
    const Plan& plan_;
    Database& database_;
};

} // namespace flashdb