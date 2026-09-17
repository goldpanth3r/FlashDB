#ifndef FLASHDB_BUFFER_BUFFER_MANAGER_H
#define FLASHDB_BUFFER_BUFFER_MANAGER_H

#include <cstddef>
#include <vector>

#include "buffer.h"
#include "file/file_manager.h"

namespace flashdb {

/**
 * BufferManager manages the database buffer pool.
 *
 * The buffer pool keeps database pages in memory so that
 * database operations do not need to access disk every time.
 */
class BufferManager {
public:
    /**
     * Create a buffer manager.
     *
     * @param file_manager File manager used for disk I/O.
     * @param buffer_count Number of buffers in the pool.
     */
    BufferManager(
        FileManager& file_manager,
        std::size_t buffer_count
    );

    /**
     * Get a buffer containing the requested block.
     *
     * If the block is already cached, its pin count is increased.
     * Otherwise an available buffer is selected.
     *
     * @param block Database block to load.
     * @return Pointer to the buffer, or nullptr if all buffers are pinned.
     */
    Buffer* get_buffer(const BlockId& block);

    /**
     * Flush one buffer to disk.
     *
     * Dirty buffers are written to disk.
     * A pinned dirty buffer cannot be flushed.
     *
     * @param buffer Buffer to flush.
     */
    void flush_buffer(Buffer& buffer);

    /**
     * Flush all dirty, unpinned buffers to disk.
     */
    void flush_all();

    /**
     * Release one user's reference to a buffer.
     *
     * @param buffer Buffer to unpin.
     */
    void unpin_buffer(Buffer& buffer);

    /**
     * Return the number of buffers in the pool.
     *
     * @return Number of buffers.
     */
    std::size_t size() const;

private:
    FileManager& file_manager_;
    std::vector<Buffer> buffers_;
};

} // namespace flashdb

#endif