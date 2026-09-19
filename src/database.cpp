#include "database.h"

#include <filesystem>
#include <stdexcept>

#include "record/layout.h"

namespace flashdb {

// Initialize storage, metadata, and index managers for the database.
Database::Database(
    const std::string& database_directory,
    std::size_t buffer_count)
    : file_manager_(database_directory),
      buffer_manager_(
          file_manager_,
          buffer_count
      ),
      catalog_(
          (
              std::filesystem::path(database_directory)
              / "flashdb.catalog"
          ).string()
      ),
      index_manager_(
          database_directory
      ) {

    // Restore table definitions before opening indexes.
    catalog_.load();

    // Restore index definitions from disk.
    index_manager_.load();

    // Rebuild B+ Tree contents from persistent table records.
    index_manager_.rebuild_indexes(*this);
}

// Flush modified database pages before shutdown.
Database::~Database() {
    buffer_manager_.flush_all();
}

// Provide access to the file manager.
FileManager& Database::file_manager() {
    return file_manager_;
}

// Provide access to the buffer manager.
BufferManager& Database::buffer_manager() {
    return buffer_manager_;
}

// Provide access to the table catalog.
TableCatalog& Database::catalog() {
    return catalog_;
}

// Provide access to the index manager.
IndexManager& Database::index_manager() {
    return index_manager_;
}

// Open an existing table using its catalog metadata.
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