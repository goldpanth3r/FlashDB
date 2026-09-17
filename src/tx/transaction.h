#ifndef FLASHDB_TX_TRANSACTION_H
#define FLASHDB_TX_TRANSACTION_H

#include <cstddef>

#include "buffer/buffer_manager.h"
#include "file/file_manager.h"
#include "log/log_manager.h"

namespace flashdb {

/**
 * Transaction represents one database transaction.
 *
 * A transaction groups database operations into one unit.
 * Later this class will also handle locking and recovery.
 */
class Transaction {
public:
    /**
     * Create a transaction.
     *
     * @param file_manager Used for database file operations.
     * @param log_manager Used for transaction logging.
     * @param buffer_manager Used for cached pages.
     */
    Transaction(
        FileManager& file_manager,
        LogManager& log_manager,
        BufferManager& buffer_manager);

    /**
     * Commit the transaction.
     *
     * @return Transaction ID.
     */
    std::size_t commit();

    /**
     * Roll back the transaction.
     *
     * @return Transaction ID.
     */
    std::size_t rollback();

    /**
     * Return this transaction's ID.
     */
    std::size_t id() const;

private:
    FileManager& file_manager_;
    LogManager& log_manager_;
    BufferManager& buffer_manager_;

    std::size_t transaction_id_;

    static std::size_t next_transaction_id_;
};

} // namespace flashdb

#endif