#include "recovery_manager.h"

#include <stdexcept>
#include <unordered_set>

#include "record/record_file.h"

namespace flashdb {

namespace {

// Read the record kind from the serialized WAL record.
LogRecordKind get_record_kind(
    const std::vector<std::byte>& record
) {
    // 4 bytes are used by the WAL version.
    constexpr std::size_t KIND_OFFSET = 4;

    if (record.size() <= KIND_OFFSET) {
        throw std::runtime_error(
            "RecoveryManager: invalid WAL record"
        );
    }

    return static_cast<LogRecordKind>(
        std::to_integer<unsigned char>(
            record[KIND_OFFSET]
        )
    );
}

} // namespace

// Keep the database and WAL available while recovery runs.
RecoveryManager::RecoveryManager(
    Database& database,
    LogManager& log_manager
)
    : database_(database),
      log_manager_(log_manager) {
}

// Find transactions that already finished with COMMIT or ABORT.
std::unordered_set<std::size_t>
RecoveryManager::completed_transactions() const {
    std::unordered_set<std::size_t> completed;

    for (std::size_t lsn = 0;
         lsn < log_manager_.size();
         ++lsn) {

        const auto raw_record =
            log_manager_.read(lsn);

        if (get_record_kind(raw_record) !=
            LogRecordKind::TRANSACTION) {
            continue;
        }

        const TransactionLogRecord record =
            log_manager_.read_transaction_record(lsn);

        if (record.type == LogRecordType::COMMIT ||
            record.type == LogRecordType::ABORT) {

            completed.insert(
                record.transaction_id
            );
        }
    }

    return completed;
}

// Apply the inverse operation stored in one WAL undo record.
void RecoveryManager::undo(
    const UndoLogRecord& record,
    std::size_t lsn
) {
    auto table =
        database_.open_table(
            record.table_name
        );

    const RecordId rid(
        record.page_number,
        record.slot_number
    );

    switch (record.type) {
        case UndoRecordType::INSERT:
            // The original operation inserted this record.
            // Recovery removes it.
            table->remove_with_log(
                rid,
                lsn
            );
            break;

        case UndoRecordType::UPDATE:
            // The original operation changed one field.
            // Recovery restores its previous value.
            table->set_with_log(
                rid,
                record.field_name,
                record.old_value,
                lsn
            );
            break;

        case UndoRecordType::DELETE:
            // The original operation deleted the record.
            // Recovery restores the complete old record.
            table->restore(
                rid,
                record.old_record,
                lsn
            );
            break;

        default:
            throw std::runtime_error(
                "RecoveryManager: unknown undo record type"
            );
    }
}

// Scan the WAL backward, undo incomplete transactions,
// and mark them as aborted.
void RecoveryManager::recover() {
    const auto completed =
        completed_transactions();

    std::unordered_set<std::size_t> recovered_transactions;

    for (std::size_t lsn = log_manager_.size();
         lsn > 0;
         --lsn) {

        const std::size_t current_lsn =
            lsn - 1;

        const auto raw_record =
            log_manager_.read(current_lsn);

        const LogRecordKind kind =
            get_record_kind(raw_record);

        // Transaction lifecycle records do not contain undo data.
        if (kind == LogRecordKind::TRANSACTION) {
            continue;
        }

        if (kind != LogRecordKind::UNDO) {
            throw std::runtime_error(
                "RecoveryManager: unknown WAL record kind"
            );
        }

        const UndoLogRecord record =
            log_manager_.read_undo_record(
                current_lsn
            );

        // Completed transactions do not need recovery.
        if (completed.contains(
                record.transaction_id)) {
            continue;
        }

        // Undo operations in exact reverse WAL order.
        undo(
            record,
            current_lsn
        );

        recovered_transactions.insert(
            record.transaction_id
        );
    }

    // Mark recovered transactions as aborted.
    for (const std::size_t transaction_id :
         recovered_transactions) {

        log_manager_.append_transaction_record(
            transaction_id,
            LogRecordType::ABORT
        );
    }

    if (!recovered_transactions.empty()) {
        log_manager_.flush();
    }
}

} // namespace flashdb