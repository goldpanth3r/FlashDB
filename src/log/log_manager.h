#ifndef FLASHDB_LOG_LOG_MANAGER_H
#define FLASHDB_LOG_LOG_MANAGER_H

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace flashdb {

// Identifies the kind of record stored in the WAL.
enum class LogRecordKind : std::uint8_t {
    TRANSACTION = 1,
    UNDO = 2
};

// Describes the lifecycle state of a transaction.
enum class LogRecordType : std::uint8_t {
    BEGIN = 1,
    COMMIT = 2,
    ABORT = 3
};

// Describes the change that can be undone.
enum class UndoRecordType : std::uint8_t {
    INSERT = 1,
    UPDATE = 2,
    DELETE = 3
};

// Stores information about a transaction lifecycle record.
struct TransactionLogRecord {
    LogRecordType type;
    std::size_t transaction_id;
};

// Stores information needed to undo one database change.
struct UndoLogRecord {
    UndoRecordType type;
    std::size_t transaction_id;

    std::string table_name;

    int page_number;
    std::size_t slot_number;

    std::string field_name;
    std::string old_value;

    std::unordered_map<std::string, std::string> old_record;
};

class LogManager {
public:
    // Open or create the WAL for the database.
    explicit LogManager(
        const std::string& database_directory
    );

    // Append one raw record to the WAL.
    std::size_t append(
        const std::vector<std::byte>& record
    );

    // Append a transaction lifecycle record.
    std::size_t append_transaction_record(
        std::size_t transaction_id,
        LogRecordType type
    );

    // Append an undo record for a database change.
    std::size_t append_undo_record(
        const UndoLogRecord& record
    );

    // Read one WAL record using its log sequence number.
    std::vector<std::byte> read(
        std::size_t lsn
    ) const;

    // Read and decode a transaction lifecycle record.
    TransactionLogRecord read_transaction_record(
        std::size_t lsn
    ) const;

    // Read and decode an undo record.
    UndoLogRecord read_undo_record(
        std::size_t lsn
    ) const;

    // Flush buffered WAL data to disk.
    void flush();

    // Return the number of records currently in the WAL.
    std::size_t size() const;

private:
    std::filesystem::path log_path_;
    std::ofstream log_file_;
    std::size_t record_count_;

    // Count existing records when opening an existing WAL.
    std::size_t count_existing_records() const;

    // Convert a transaction record into bytes.
    static std::vector<std::byte> encode_transaction_record(
        std::size_t transaction_id,
        LogRecordType type
    );

    // Convert transaction record bytes back into a record.
    static TransactionLogRecord decode_transaction_record(
        const std::vector<std::byte>& record
    );

    // Convert an undo record into bytes.
    static std::vector<std::byte> encode_undo_record(
        const UndoLogRecord& record
    );

    // Convert undo record bytes back into a record.
    static UndoLogRecord decode_undo_record(
        const std::vector<std::byte>& record
    );
};

} // namespace flashdb

#endif