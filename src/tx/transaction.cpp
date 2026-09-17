#include "transaction.h"

namespace flashdb {

std::size_t Transaction::next_transaction_id_ = 0;

Transaction::Transaction(
    FileManager& file_manager,
    LogManager& log_manager,
    BufferManager& buffer_manager)
    : file_manager_(file_manager),
      log_manager_(log_manager),
      buffer_manager_(buffer_manager),
      transaction_id_(next_transaction_id_++) {}

std::size_t Transaction::commit() {
    log_manager_.flush();
    return transaction_id_;
}

std::size_t Transaction::rollback() {
    return transaction_id_;
}

std::size_t Transaction::id() const {
    return transaction_id_;
}

} // namespace flashdb