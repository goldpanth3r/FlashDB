#ifndef FLASHDB_RECOVERY_RECOVERY_MANAGER_H
#define FLASHDB_RECOVERY_RECOVERY_MANAGER_H

#include <unordered_set>

#include "database.h"
#include "log/log_manager.h"

namespace flashdb {

class RecoveryManager {
public:
    RecoveryManager(
        Database& database,
        LogManager& log_manager
    );

    // Scan the WAL and undo transactions that did not finish.
    void recover();

private:
    Database& database_;
    LogManager& log_manager_;

    // Find transactions that already finished with COMMIT or ABORT.
    std::unordered_set<std::size_t>
    completed_transactions() const;

    // Apply the inverse operation stored in one undo record.
    void undo(
        const UndoLogRecord& record,
        std::size_t lsn
    );
};

} // namespace flashdb

#endif