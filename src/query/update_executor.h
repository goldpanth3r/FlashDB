#pragma once

#include <cstddef>

#include "planner/plan.h"
#include "record/record_file.h"

namespace flashdb {

class UpdateExecutor {
public:
    UpdateExecutor(
        const Plan& plan,
        RecordFile& record_file
    );

    std::size_t execute();

private:
    const Plan& plan_;
    RecordFile& record_file_;
};

}