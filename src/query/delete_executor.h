#pragma once

#include <cstddef>

#include "database.h"
#include "planner/plan.h"
#include "record/record_file.h"
#include "tx/transaction.h"

namespace flashdb {

class DeleteExecutor {
public:
    DeleteExecutor(
        const Plan& plan,
        RecordFile& record_file
    );

    DeleteExecutor(
        const Plan& plan,
        Database& database
    );

    DeleteExecutor(
        const Plan& plan,
        Database& database,
        Transaction& transaction
    );

    std::size_t execute();

private:
    const Plan& plan_;

    Database* database_;
    RecordFile* record_file_;
    Transaction* transaction_;
};

} // namespace flashdb