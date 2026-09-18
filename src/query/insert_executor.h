#pragma once

#include <cstddef>
#include <memory>

#include "database.h"
#include "planner/plan.h"
#include "record/rid.h"
#include "tx/transaction.h"

namespace flashdb {

class InsertExecutor {
public:
    InsertExecutor(
        const Plan& plan,
        Database& database
    );

    InsertExecutor(
        const Plan& plan,
        Database& database,
        Transaction& transaction
    );

    RecordId execute();

private:
    const Plan& plan_;
    Database& database_;
    Transaction* transaction_;
};

} // namespace flashdb