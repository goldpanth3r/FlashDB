#include "database.h"

#include <filesystem>
#include <stdexcept>

#include "record/layout.h"

namespace flashdb {

// Initialize the shared storage and metadata managers for one database.
Database::Database(
    const std::string& database_directory,
    std::size_t buffer_count)
    : file_manager_(database_directory),
      buffer_manager_(file_manager_, buffer_count),
      catalog_(
          (
              std::filesystem::path(database_directory)
              / "flashdb.catalog"
          ).string()
      ) {

    // Restore table definitions before database operations begin.
    catalog_.load();
}

// Flush modified database pages before the database shuts down.
Database::~Database() {
    buffer_manager_.flush_all();
}

// Provide shared disk storage to database components.
FileManager& Database::file_manager() {
    return file_manager_;
}

// Provide the shared buffer pool to database components.
BufferManager& Database::buffer_manager() {
    return buffer_manager_;
}

// Provide the database metadata catalog.
TableCatalog& Database::catalog() {
    return catalog_;
}

// Open a table using the shared storage managers and catalog metadata.
std::unique_ptr<RecordFile> Database::open_table(
    const std::string& table_name) {

    if (!catalog_.has_table(table_name)) {
        throw std::invalid_argument(
            "Database::open_table: table does not exist"
        );
    }

    const Schema& schema =
        catalog_.get_schema(table_name);

    Layout layout(schema);

    return std::make_unique<RecordFile>(
        file_manager_,
        buffer_manager_,
        table_name,
        layout
    );
}

} // namespace flashdb