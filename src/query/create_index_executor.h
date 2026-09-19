#pragma once

#include "database.h"
#include "planner/plan.h"

namespace flashdb {

class CreateIndexExecutor {
public:
    CreateIndexExecutor(
        const Plan& plan,
        Database& database
    );

    void execute();

private:
    const Plan& plan_;
    Database& database_;
};

} // namespace flashdb
