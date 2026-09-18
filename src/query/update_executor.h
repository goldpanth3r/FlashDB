#pragma once

#include <cstddef>

#include "database.h"
#include "planner/plan.h"
#include "tx/transaction.h"

namespace flashdb {

class UpdateExecutor {
public:
    UpdateExecutor(
        const Plan& plan,
        Database& database
    );

    UpdateExecutor(
        const Plan& plan,
        Database& database,
        Transaction& transaction
    );

    std::size_t execute();

private:
    const Plan& plan_;
    Database& database_;
    Transaction* transaction_;
};

} // namespace flashdb