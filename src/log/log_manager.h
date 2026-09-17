#ifndef FLASHDB_LOG_LOG_MANAGER_H
#define FLASHDB_LOG_LOG_MANAGER_H

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace flashdb {

/**
 * LogManager stores database log records on disk.
 *
 * The log is append-only.
 *
 * Each record is stored as:
 *
 * [4-byte record length][record bytes]
 *
 * The log will later be used by transactions and recovery.
 */
class LogManager {
public:
    /**
     * Create or open the database log.
     *
     * @param database_directory Directory containing FlashDB data.
     */
    explicit LogManager(const std::string& database_directory);

    /**
     * Append one record to the log.
     *
     * @param record Bytes representing the log record.
     * @return Log sequence number of the new record.
     */
    std::size_t append(const std::vector<std::byte>& record);

    /**
     * Read a previously stored record.
     *
     * @param lsn Log sequence number.
     * @return Stored record.
     */
    std::vector<std::byte> read(std::size_t lsn) const;

    /**
     * Flush all pending log data to disk.
     */
    void flush();

    /**
     * Return the number of records in the log.
     */
    std::size_t size() const;

private:
    std::filesystem::path log_path_;
    std::ofstream log_file_;
    std::size_t record_count_;

    /**
     * Scan the existing log and count its records.
     */
    std::size_t count_existing_records() const;
};

} // namespace flashdb

#endif