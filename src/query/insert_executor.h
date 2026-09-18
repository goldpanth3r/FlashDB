#pragma once

#include <cstddef>
#include <string>

#include "planner/plan.h"
#include "record/record_file.h"

namespace flashdb {

class InsertExecutor {
public:
    InsertExecutor(
        const Plan& plan,
        RecordFile& record_file
    );

    RecordId execute();

private:
    const Plan& plan_;
    RecordFile& record_file_;
};

}