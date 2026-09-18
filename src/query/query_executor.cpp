#include "query/query_executor.h"

#include <memory>
#include <stdexcept>

#include "query/filter_executor.h"
#include "query/project_executor.h"
#include "query/table_scan_executor.h"

namespace flashdb {

// Build the executor pipeline for the planned query.
QueryExecutor::QueryExecutor(
    const Plan& plan,
    RecordFile& record_file)
    : plan_(plan),
      record_file_(record_file) {
}

// Execute a SELECT query from the plan down to the storage layer.
std::vector<std::vector<std::string>>
QueryExecutor::execute() {

    if (plan_.get_name() != "Project") {
        throw std::invalid_argument(
            "QueryExecutor: expected Project plan"
        );
    }

    const Plan* input_plan = plan_.get_child();

    if (input_plan == nullptr) {
        throw std::invalid_argument(
            "QueryExecutor: Project plan has no child"
        );
    }

    // Use a table scan as the base source of records.
    TableScanExecutor table_scan(record_file_);

    std::unique_ptr<FilterExecutor> filter;

    RecordExecutor* input = &table_scan;

    // Add filtering when the query contains a WHERE condition.
    if (input_plan->get_name() == "Filter") {
        const Condition* condition =
            input_plan->get_condition();

        if (condition == nullptr) {
            throw std::invalid_argument(
                "QueryExecutor: Filter plan has no condition"
            );
        }

        filter = std::make_unique<FilterExecutor>(
            table_scan,
            record_file_,
            *condition
        );

        input = filter.get();
    }

    if (input_plan->get_name() != "TableScan" &&
        input_plan->get_name() != "Filter") {
        throw std::invalid_argument(
            "QueryExecutor: unsupported input plan"
        );
    }

    // Apply the SELECT list to the records produced by the input executor.
    ProjectExecutor project(
        *input,
        record_file_,
        plan_.get_columns()
    );

    project.open();

    std::vector<std::vector<std::string>> results;

    // Consume the executor pipeline and collect the final result rows.
    while (project.has_next()) {
        results.push_back(project.next());
    }

    project.close();

    return results;
}

}