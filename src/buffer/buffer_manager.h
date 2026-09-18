#ifndef FLASHDB_BUFFER_BUFFER_MANAGER_H
#define FLASHDB_BUFFER_BUFFER_MANAGER_H

#include <cstddef>
#include <vector>

#include "buffer.h"
#include "file/file_manager.h"

namespace flashdb {

class LogManager;

class BufferManager {
public:
    BufferManager(
        FileManager& file_manager,
        std::size_t buffer_count
    );

    void set_log_manager(
        LogManager& log_manager
    );

    Buffer* get_buffer(
        const BlockId& block
    );

    void flush_buffer(
        Buffer& buffer
    );

    void flush_all();

    void unpin_buffer(
        Buffer& buffer
    );

    void set_log_sequence_number(
        Buffer& buffer,
        std::size_t lsn
    );

    std::size_t size() const;

private:
    FileManager& file_manager_;
    std::vector<Buffer> buffers_;
    LogManager* log_manager_;

    void flush_page(
        Buffer& buffer
    );
};

} // namespace flashdb

#endif