#ifndef FLASHDB_PARSER_AST_TRANSACTION_STATEMENT_H
#define FLASHDB_PARSER_AST_TRANSACTION_STATEMENT_H

namespace flashdb {

enum class TransactionCommand {
    BEGIN,
    COMMIT,
    ROLLBACK
};

class TransactionStatement {
public:
    explicit TransactionStatement(
        TransactionCommand command
    );

    TransactionCommand command() const;

private:
    TransactionCommand command_;
};

} // namespace flashdb

#endif