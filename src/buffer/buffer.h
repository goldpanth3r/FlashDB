#ifndef FLASHDB_BUFFER_BUFFER_H
#define FLASHDB_BUFFER_BUFFER_H

#include <cstddef>
#include <limits>

#include "file/block_id.h"
#include "file/page.h"

namespace flashdb {

class Buffer {
public:
    Buffer();

    Page& page();
    const Page& page() const;

    const BlockId& block() const;

    bool is_used() const;
    bool is_dirty() const;

    std::size_t pin_count() const;

    std::size_t log_sequence_number() const;

    void set_block(const BlockId& block);

    void set_log_sequence_number(
        std::size_t lsn
    );

    void mark_dirty();
    void mark_clean();

    void pin();
    void unpin();

private:
    Page page_;
    BlockId block_;

    bool used_;
    bool dirty_;
    std::size_t pin_count_;

    std::size_t log_sequence_number_;
};

} // namespace flashdb

#endif