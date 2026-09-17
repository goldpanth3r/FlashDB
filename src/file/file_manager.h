#ifndef FLASHDB_FILE_FILE_MANAGER_H
#define FLASHDB_FILE_FILE_MANAGER_H

#include <filesystem>
#include <string>

#include "block_id.h"
#include "page.h"

namespace flashdb {

/**
 * FileManager performs low-level disk I/O.
 *
 * Every database table is stored as a .tbl file.
 * Data is read and written in fixed-size 4KB blocks.
 */
class FileManager {
public:
    explicit FileManager(const std::string& database_directory);

    void read(const BlockId& block, Page& page);

    void write(const BlockId& block, const Page& page);

    BlockId append(const std::string& filename);

    int length(const std::string& filename);

private:
    std::filesystem::path db_directory_;

    std::filesystem::path file_path(const std::string& filename) const;
};

} // namespace flashdb

#endif