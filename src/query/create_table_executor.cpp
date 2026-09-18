#include "query/create_table_executor.h"

#include <stdexcept>

namespace flashdb {

// Prepare CREATE TABLE execution using the shared database managers.
CreateTableExecutor::CreateTableExecutor(
    const Plan& plan,
    Database& database)
    : plan_(plan),
      database_(database) {
}

// Register the schema and create the table's physical storage.
void CreateTableExecutor::execute() {

    if (plan_.get_name() != "CreateTable") {
        throw std::invalid_argument(
            "CreateTableExecutor: expected CreateTable plan"
        );
    }

    const Schema* schema =
        plan_.get_schema();

    if (schema == nullptr) {
        throw std::invalid_argument(
            "CreateTableExecutor: missing table schema"
        );
    }

    const std::string& table_name =
        plan_.get_table_name();

    TableCatalog& catalog =
        database_.catalog();

    if (catalog.has_table(table_name)) {
        throw std::invalid_argument(
            "CreateTableExecutor: table already exists"
        );
    }

    // Register the schema in the shared database catalog.
    catalog.create_table(
        table_name,
        *schema
    );

    // Create the initial physical table page.
    database_.file_manager().append(
        table_name + ".tbl"
    );

    // Persist the table definition across database restarts.
    catalog.save();
}

}