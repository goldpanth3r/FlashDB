#include "query/query_executor.h"

#include <memory>
#include <stdexcept>
#include <vector>

#include "query/filter_executor.h"
#include "query/index_scan_executor.h"
#include "query/project_executor.h"
#include "query/table_scan_executor.h"

namespace flashdb {

namespace {

// Check whether an index can answer this WHERE condition.
bool can_use_index(
    const Condition& condition,
    const std::string& table_name,
    const IndexManager& index_manager,
    std::string& index_name,
    int& key) {

    if (condition.operator_() != "=") {
        return false;
    }

    const Expression& left =
        condition.left();

    const Expression& right =
        condition.right();

    if (left.type() != ExpressionType::IDENTIFIER ||
        right.type() != ExpressionType::INTEGER) {
        return false;
    }

    const std::vector<std::string> index_names =
        index_manager.indexes_for_table(
            table_name
        );

    for (const std::string& name :
         index_names) {

        const IndexMetadata& metadata =
            index_manager.metadata(name);

        if (metadata.column_name !=
            left.value()) {

            continue;
        }

        try {
            key = std::stoi(
                right.value()
            );
        }
        catch (const std::exception&) {
            return false;
        }

        index_name = name;

        return true;
    }

    return false;
}

} // namespace

// Prepare SELECT execution using the shared database storage.
QueryExecutor::QueryExecutor(
    const Plan& plan,
    Database& database)
    : plan_(plan),
      database_(database) {
}

// Execute SELECT using an index when possible, otherwise use a table scan.
std::vector<std::vector<std::string>>
QueryExecutor::execute() {

    if (plan_.get_name() != "Project") {
        throw std::invalid_argument(
            "QueryExecutor: expected Project plan"
        );
    }

    const Plan* input_plan =
        plan_.get_child();

    if (input_plan == nullptr) {
        throw std::invalid_argument(
            "QueryExecutor: Project plan has no child"
        );
    }

    std::unique_ptr<RecordFile> record_file =
        database_.open_table(
            plan_.get_table_name()
        );

    std::unique_ptr<IndexScanExecutor>
        index_scan;

    std::unique_ptr<TableScanExecutor>
        table_scan;

    std::unique_ptr<FilterExecutor>
        filter;

    RecordExecutor* input = nullptr;

    /*
     * Use an index for an equality condition when
     * an index exists for the referenced column.
     */
    if (input_plan->get_name() == "Filter" &&
        input_plan->get_condition() != nullptr) {

        const Condition& condition =
            *input_plan->get_condition();

        std::string index_name;
        int key = 0;

        if (can_use_index(
                condition,
                plan_.get_table_name(),
                database_.index_manager(),
                index_name,
                key)) {

            const std::vector<RecordId> record_ids =
                database_.index_manager().search_all(
                    index_name,
                    key
                );

            index_scan =
                std::make_unique<IndexScanExecutor>(
                    record_ids
                );

            input = index_scan.get();
        }
    }

    /*
     * If no usable index exists, keep the original
     * table-scan and filter execution path.
     */
    if (input == nullptr) {

        table_scan =
            std::make_unique<TableScanExecutor>(
                *record_file
            );

        input = table_scan.get();

        if (input_plan->get_name() == "Filter") {

            const Condition* condition =
                input_plan->get_condition();

            if (condition == nullptr) {
                throw std::invalid_argument(
                    "QueryExecutor: Filter plan has no condition"
                );
            }

            filter =
                std::make_unique<FilterExecutor>(
                    *table_scan,
                    *record_file,
                    *condition
                );

            input = filter.get();
        }
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
        *record_file,
        plan_.get_columns()
    );

    project.open();

    std::vector<std::vector<std::string>> results;

    // Consume the executor pipeline and collect the final result rows.
    while (project.has_next()) {
        results.push_back(
            project.next()
        );
    }

    project.close();

    return results;
}

} // namespace flashdb