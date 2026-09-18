#pragma once

#include <cstddef>
#include <memory>
#include <string>

#include "buffer/buffer_manager.h"
#include "file/file_manager.h"
#include "metadata/table_catalog.h"
#include "record/record_file.h"

namespace flashdb {

class Database {
public:
    explicit Database(
        const std::string& database_directory,
        std::size_t buffer_count = 16
    );

    ~Database();

    FileManager& file_manager();
    BufferManager& buffer_manager();
    TableCatalog& catalog();

    std::unique_ptr<RecordFile> open_table(
        const std::string& table_name
    );

private:
    FileManager file_manager_;
    BufferManager buffer_manager_;
    TableCatalog catalog_;
};

} // namespace flashdb