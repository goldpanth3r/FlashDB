#include "transaction.h"

#include <stdexcept>
#include <string>
#include <vector>

#include "database.h"
#include "record/record_file.h"

namespace flashdb {

    namespace {

        int index_key(
            const std::string& value) {

            try {
                return std::stoi(value);
            }
            catch (const std::exception&) {
                throw std::invalid_argument(
                    "Transaction: indexed column must contain an integer"
                );
            }
        }

        // Remove the current value and restore the old value in affected indexes.
        void restore_indexes(
            Database& database,
            const std::string& table_name,
            const std::string& column_name,
            const RecordId& rid,
            const std::string& old_value,
            const std::string& current_value) {

            const std::vector<std::string> index_names =
                database.index_manager().indexes_for_table(
                    table_name
                );

            for (const std::string& index_name : index_names) {

                const IndexMetadata& metadata =
                    database.index_manager().metadata(
                        index_name
                    );

                if (metadata.column_name != column_name) {
                    continue;
                }

                database.index_manager().remove(
                    index_name,
                    index_key(current_value),
                    rid
                );

                database.index_manager().insert(
                    index_name,
                    index_key(old_value),
                    rid
                );
            }
        }

        } // namespace

std::size_t Transaction::next_transaction_id_ = 0;

// Start a transaction using the existing database managers.
Transaction::Transaction(
    FileManager& file_manager,
    LogManager& log_manager,
    BufferManager& buffer_manager
)
    : file_manager_(&file_manager),
      log_manager_(log_manager),
      buffer_manager_(&buffer_manager),
      database_(nullptr),
      transaction_id_(next_transaction_id_++) {
    buffer_manager_->set_log_manager(
        log_manager_
    );

    log_manager_.append_transaction_record(
        transaction_id_,
        LogRecordType::BEGIN
    );

    log_manager_.flush();
}

// Start a transaction with access to the complete database.
Transaction::Transaction(
    Database& database,
    LogManager& log_manager
)
    : file_manager_(&database.file_manager()),
      log_manager_(log_manager),
      buffer_manager_(&database.buffer_manager()),
      database_(&database),
      transaction_id_(next_transaction_id_++) {
    buffer_manager_->set_log_manager(
        log_manager_
    );

    log_manager_.append_transaction_record(
        transaction_id_,
        LogRecordType::BEGIN
    );

    log_manager_.flush();
}

// Commit the transaction after making its WAL records durable.
std::size_t Transaction::commit() {
    log_manager_.append_transaction_record(
        transaction_id_,
        LogRecordType::COMMIT
    );

    log_manager_.flush();

    undo_records_.clear();

    return transaction_id_;
}

// Insert a record after its undo information is durable.
RecordId Transaction::insert(
    const std::string& table_name,
    const std::unordered_map<std::string, std::string>& values
) {
    if (database_ == nullptr) {
        throw std::runtime_error(
            "Database transaction API requires Database"
        );
    }

    auto table =
        database_->open_table(table_name);

    const RecordId rid =
        table->find_insert_rid();

    UndoLogRecord undo{
        UndoRecordType::INSERT,
        transaction_id_,
        table_name,
        rid.page_number(),
        rid.slot_number(),
        "",
        "",
        {}
    };

    const std::size_t lsn =
        log_manager_.append_undo_record(undo);

    log_manager_.flush();

    table->insert_at(
        rid,
        values,
        lsn
    );

    undo_records_.push_back({
        undo,
        lsn
    });

    return rid;
}

// Save the old value before changing a record.
void Transaction::update(
    const std::string& table_name,
    const RecordId& rid,
    const std::string& field_name,
    const std::string& value
) {
    if (database_ == nullptr) {
        throw std::runtime_error(
            "Database transaction API requires Database"
        );
    }

    auto table =
        database_->open_table(table_name);

    const std::string old_value =
        table->get(
            rid,
            field_name
        );

    UndoLogRecord undo{
        UndoRecordType::UPDATE,
        transaction_id_,
        table_name,
        rid.page_number(),
        rid.slot_number(),
        field_name,
        old_value,
        {}
    };

    const std::size_t lsn =
        log_manager_.append_undo_record(undo);

    log_manager_.flush();

    table->set_with_log(
        rid,
        field_name,
        value,
        lsn
    );

    undo_records_.push_back({
        undo,
        lsn
    });
}

// Save the complete record before deleting it.
void Transaction::remove(
    const std::string& table_name,
    const RecordId& rid
) {
    if (database_ == nullptr) {
        throw std::runtime_error(
            "Database transaction API requires Database"
        );
    }

    auto table =
        database_->open_table(table_name);

    std::unordered_map<std::string, std::string> old_record;

    for (const auto& field :
         table->layout().schema().fields()) {
        old_record[field.name] =
            table->get(
                rid,
                field.name
            );
    }

    UndoLogRecord undo{
        UndoRecordType::DELETE,
        transaction_id_,
        table_name,
        rid.page_number(),
        rid.slot_number(),
        "",
        "",
        old_record
    };

    const std::size_t lsn =
        log_manager_.append_undo_record(undo);

    log_manager_.flush();

    table->remove_with_log(
        rid,
        lsn
    );

    undo_records_.push_back({
        undo,
        lsn
    });
}

// Undo all changes in reverse order and record the abort.
std::size_t Transaction::rollback() {
    if (database_ != nullptr) {
        for (auto it = undo_records_.rbegin();
             it != undo_records_.rend();
             ++it) {
            undo_record(*it);
        }
    }

    log_manager_.append_transaction_record(
        transaction_id_,
        LogRecordType::ABORT
    );

    log_manager_.flush();

    undo_records_.clear();

    return transaction_id_;
}

// Apply the inverse operation for one logged change.
void Transaction::undo_record(
    const TransactionUndo& undo
) {
    if (database_ == nullptr) {
        throw std::runtime_error(
            "Database transaction API requires Database"
        );
    }

    const UndoLogRecord& record =
        undo.record;

    auto table =
        database_->open_table(
            record.table_name
        );

    const RecordId rid(
        record.page_number,
        record.slot_number
    );

    switch (record.type) {
        case UndoRecordType::INSERT:
            table->remove_with_log(
                rid,
                undo.lsn
            );
            break;

        case UndoRecordType::UPDATE: {
            const std::string current_value =
                table->get(
                    rid,
                    record.field_name
                );

            table->set_with_log(
                rid,
                record.field_name,
                record.old_value,
                undo.lsn
            );

            // Restore the index from the new value back to the old value.
            restore_indexes(
                *database_,
                record.table_name,
                record.field_name,
                rid,
                record.old_value,
                current_value
            );

            break;
        }

        case UndoRecordType::DELETE: {
            // Restore the deleted row.
            table->restore(
                rid,
                record.old_record,
                undo.lsn
            );

            // Restore index entries for the deleted row.
            const std::vector<std::string> index_names =
                database_->index_manager().indexes_for_table(
                    record.table_name
                );

            for (const std::string& index_name :
                index_names) {

                const IndexMetadata& metadata =
                    database_->index_manager().metadata(
                        index_name
                    );

                auto value_it =
                    record.old_record.find(
                        metadata.column_name
                    );

                if (value_it ==
                    record.old_record.end()) {
                    continue;
                }

                const int key =
                    index_key(value_it->second);

                database_->index_manager().insert(
                    index_name,
                    key,
                    rid
                );
            }

            break;
        }

        default:
            throw std::runtime_error(
                "Unknown undo record type"
            );
    }
}

// Return the unique identifier of this transaction.
std::size_t Transaction::id() const {
    return transaction_id_;
}

} // namespace flashdb
