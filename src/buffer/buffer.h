#ifndef FLASHDB_BUFFER_BUFFER_H
#define FLASHDB_BUFFER_BUFFER_H

#include <cstddef>

#include "file/block_id.h"
#include "file/page.h"

namespace flashdb {

/**
 * Buffer represents one database page currently stored in memory.
 *
 * A buffer tracks:
 * - which block it contains
 * - whether it has been modified
 * - how many users currently hold it
 */
class Buffer {
public:
    /**
     * Create an empty buffer.
     */
    Buffer();

    Page& page();
    const Page& page() const;

    const BlockId& block() const;

    bool is_used() const;

    bool is_dirty() const;

    /**
     * Return the number of active users of this buffer.
     */
    std::size_t pin_count() const;

    /**
     * Assign a block to this buffer.
     *
     * @param block Block that will be stored.
     */
    void set_block(const BlockId& block);

    /**
     * Mark the buffer as modified.
     */
    void mark_dirty();

    /**
     * Mark the buffer as clean.
     */
    void mark_clean();

    /**
     * Add one user of this buffer.
     */
    void pin();

    /**
     * Remove one user of this buffer.
     */
    void unpin();

private:
    Page page_;
    BlockId block_;
    bool used_;
    bool dirty_;
    std::size_t pin_count_;
};

} // namespace flashdb

#endif