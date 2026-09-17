#ifndef FLASHDB_BUFFER_BUFFER_MANAGER_H
#define FLASHDB_BUFFER_BUFFER_MANAGER_H

#include <cstddef>
#include <vector>

#include "buffer.h"
#include "file/file_manager.h"

namespace flashdb {

/**
 * BufferManager controls the pages currently cached in memory.
 *
 * It loads pages from the FileManager and gives higher database
 * layers access to those pages.
 */
class BufferManager {
public:
    /**
     * Create a Buffer Manager.
     *
     * @param file_manager File manager used for disk I/O.
     * @param buffer_count Number of buffers available in memory.
     */
    BufferManager(FileManager& file_manager, std::size_t buffer_count);

    /**
     * Get a buffer containing the requested block.
     *
     * If the block is already cached, that buffer is returned.
     * Otherwise an unused buffer is loaded from disk.
     *
     * @param block Database block to load.
     * @return Pointer to the buffer, or nullptr if no buffer is available.
     */
    Buffer* get_buffer(const BlockId& block);

    /**
     * Return the number of buffers managed by this object.
     */
    std::size_t size() const;

private:
    FileManager& file_manager_;
    std::vector<Buffer> buffers_;
};

} // namespace flashdb

#endif