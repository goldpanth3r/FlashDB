#ifndef FLASHDB_TX_TRANSACTION_H
#define FLASHDB_TX_TRANSACTION_H

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

#include "buffer/buffer_manager.h"
#include "file/file_manager.h"
#include "log/log_manager.h"
#include "record/rid.h"

namespace flashdb {

class Database;

struct TransactionUndo {
    UndoLogRecord record;
    std::size_t lsn;
};

class Transaction {
public:
    Transaction(
        FileManager& file_manager,
        LogManager& log_manager,
        BufferManager& buffer_manager
    );

    Transaction(
        Database& database,
        LogManager& log_manager
    );

    std::size_t commit();

    std::size_t rollback();

    RecordId insert(
        const std::string& table_name,
        const std::unordered_map<std::string, std::string>& values
    );

    void update(
        const std::string& table_name,
        const RecordId& rid,
        const std::string& field_name,
        const std::string& value
    );

    void remove(
        const std::string& table_name,
        const RecordId& rid
    );

    std::size_t id() const;

private:
    FileManager* file_manager_;
    LogManager& log_manager_;
    BufferManager* buffer_manager_;
    Database* database_;

    std::size_t transaction_id_;

    std::vector<TransactionUndo> undo_records_;

    static std::size_t next_transaction_id_;

    void undo_record(
        const TransactionUndo& undo
    );
};

} // namespace flashdb

#endif