#include "query/create_table_executor.h"

#include <stdexcept>

namespace flashdb {

// Prepare CREATE TABLE execution against the database catalog.
CreateTableExecutor::CreateTableExecutor(
    const Plan& plan,
    TableCatalog& catalog)
    : plan_(plan),
      catalog_(catalog) {
}

// Register the new table schema with the database metadata.
void CreateTableExecutor::execute() {

    if (plan_.get_name() != "CreateTable") {
        throw std::invalid_argument(
            "CreateTableExecutor: expected CreateTable plan"
        );
    }

    const Schema* schema = plan_.get_schema();

    if (schema == nullptr) {
        throw std::invalid_argument(
            "CreateTableExecutor: missing table schema"
        );
    }

    if (catalog_.has_table(plan_.get_table_name())) {
        throw std::invalid_argument(
            "CreateTableExecutor: table already exists"
        );
    }

    catalog_.create_table(
        plan_.get_table_name(),
        *schema
    );
}

}