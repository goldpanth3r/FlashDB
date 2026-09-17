#ifndef FLASHDB_BUFFER_BUFFER_H
#define FLASHDB_BUFFER_BUFFER_H

#include "file/block_id.h"
#include "file/page.h"

namespace flashdb {

/**
 * Buffer represents one page currently stored in memory.
 *
 * The Buffer Manager uses Buffer objects to cache database pages
 * so the database does not need to read from disk every time.
 */
class Buffer {
public:
    Buffer();

    /**
     * Return the page stored in this buffer.
     */
    Page& page();

    /**
     * Return the page stored in this buffer.
     */
    const Page& page() const;

    /**
     * Return the block currently stored in this buffer.
     */
    const BlockId& block() const;

    /**
     * Return whether this buffer currently contains a block.
     */
    bool is_used() const;

    /**
     * Store a block identifier in this buffer.
     *
     * @param block Block represented by this buffer.
     */
    void set_block(const BlockId& block);

private:
    Page page_;
    BlockId block_;
    bool used_;
};

} // namespace flashdb

#endif