#ifndef FLASHDB_RECORD_RECORD_FILE_H
#define FLASHDB_RECORD_RECORD_FILE_H

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

#include "buffer/buffer_manager.h"
#include "file/block_id.h"
#include "file/file_manager.h"
#include "record/layout.h"
#include "record/rid.h"

namespace flashdb {

class RecordFile {
public:
    RecordFile(
        FileManager& file_manager,
        BufferManager& buffer_manager,
        const std::string& table_name,
        const Layout& layout
    );

    RecordId insert(
        const std::unordered_map<std::string, std::string>& values
    );

    RecordId find_insert_rid() const;

    void insert_at(
        const RecordId& rid,
        const std::unordered_map<std::string, std::string>& values,
        std::size_t log_sequence_number
    );

    std::string get(
        const RecordId& rid,
        const std::string& field_name
    );

    std::unordered_map<std::string, std::string> get_record(
        const RecordId& rid
    );

    void set(
        const RecordId& rid,
        const std::string& field_name,
        const std::string& value
    );

    void set_with_log(
        const RecordId& rid,
        const std::string& field_name,
        const std::string& value,
        std::size_t log_sequence_number
    );

    void set_log_sequence_number(
        const RecordId& rid,
        std::size_t log_sequence_number
    );

    void remove(
        const RecordId& rid
    );

    void remove_with_log(
        const RecordId& rid,
        std::size_t log_sequence_number
    );

    void restore(
        const RecordId& rid,
        const std::unordered_map<std::string, std::string>& values
    );

    void restore(
        const RecordId& rid,
        const std::unordered_map<std::string, std::string>& values,
        std::size_t log_sequence_number
    );

    int page_count() const;

    std::vector<RecordId> scan();

    const Layout& layout() const;

    const std::string& table_name() const;

private:
    FileManager& file_manager_;
    BufferManager& buffer_manager_;
    std::string table_name_;
    Layout layout_;

    BlockId block_id(int page_number) const;

    std::string table_filename() const;

    void validate_page_number(int page_number) const;
};

} // namespace flashdb

#endif