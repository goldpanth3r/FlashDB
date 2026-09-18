#ifndef FLASHDB_RECORD_RECORD_FILE_H
#define FLASHDB_RECORD_RECORD_FILE_H

#include <string>
#include <unordered_map>
#include <vector>

#include "buffer/buffer_manager.h"
#include "file/block_id.h"
#include "file/file_manager.h"
#include "record/layout.h"
#include "record/rid.h"

namespace flashdb {

/**
 * RecordFile manages all records belonging to one table.
 *
 * A table is stored as a sequence of fixed-size database pages.
 *
 * RecordFile is responsible for:
 *
 * - creating table pages
 * - finding free record slots
 * - inserting records
 * - reading records
 * - updating records
 * - deleting records
 * - scanning records
 */
class RecordFile {
public:
    /**
     * Open or create a table file.
     *
     * file_manager is used to create and inspect pages.
     * buffer_manager is used to access pages in memory.
     */
    RecordFile(
        FileManager& file_manager,
        BufferManager& buffer_manager,
        const std::string& table_name,
        const Layout& layout
    );

    /**
     * Insert a record into the table.
     *
     * Existing pages are searched first.
     * If every page is full, a new page is created.
     */
    RecordId insert(
        const std::unordered_map<std::string, std::string>& values
    );

    /**
     * Read one field from a record.
     */
    std::string get(
        const RecordId& rid,
        const std::string& field_name
    );

    /**
     * Update one field in a record.
     */
    void set(
        const RecordId& rid,
        const std::string& field_name,
        const std::string& value
    );

    /**
     * Delete a record from the table.
     */
    void remove(const RecordId& rid);

    /**
     * Return the number of pages currently used by the table.
     */
    int page_count() const;

    /**
     * Find all currently used records in the table.
     */
    std::vector<RecordId> scan();

    const Layout& layout() const;

private:
    FileManager& file_manager_;
    BufferManager& buffer_manager_;

    std::string table_name_;
    Layout layout_;

    /**
     * Create the BlockId for one table page.
     */
    BlockId block_id(int page_number) const;

    /**
     * Return the filename used by this table.
     */
    std::string table_filename() const;

    /**
     * Validate a record page number.
     */
    void validate_page_number(int page_number) const;
};

} // namespace flashdb

#endif