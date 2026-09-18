#pragma once

#include <string>
#include <vector>

#include "planner/plan.h"
#include "record/record_file.h"

namespace flashdb {

class QueryExecutor {
public:
    QueryExecutor(
        const Plan& plan,
        RecordFile& record_file
    );

    std::vector<std::vector<std::string>> execute();

private:
    const Plan& plan_;
    RecordFile& record_file_;
};

}