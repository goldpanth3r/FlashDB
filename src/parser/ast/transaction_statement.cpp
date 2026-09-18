#include "transaction_statement.h"

namespace flashdb {

// Store the transaction command parsed from SQL.
TransactionStatement::TransactionStatement(
    TransactionCommand command)
    : command_(command) {
}

// Expose the transaction operation to the planner.
TransactionCommand TransactionStatement::command() const {
    return command_;
}

} // namespace flashdb