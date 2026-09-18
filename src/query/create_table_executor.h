#pragma once

#include "metadata/table_catalog.h"
#include "planner/plan.h"

namespace flashdb {

class CreateTableExecutor {
public:
    CreateTableExecutor(
        const Plan& plan,
        TableCatalog& catalog
    );

    void execute();

private:
    const Plan& plan_;
    TableCatalog& catalog_;
};

}